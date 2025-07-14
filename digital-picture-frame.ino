/*
  LILYGO T-Dongle S3 SFTP Server and USB Mass Storage Device (using SD_MMC)

  *** HARDWARE NOTE ***
  This version of the sketch is modified to use the SD_MMC library instead of SD (SPI).
  The SD_MMC interface provides significantly faster data transfer speeds.
  HOWEVER, this requires the SD card slot to be wired to the ESP32-S3's dedicated SD_MMC pins.
  Most official examples and documentation for the LILYGO T-Dongle S3 show the use of
  SPI for the SD card. It is highly likely that the integrated SD card slot on the
  T-Dongle S3 is ONLY wired for SPI mode.
  
  Therefore, this sketch will likely FAIL to initialize the SD card on a standard
  LILYGO T-Dongle S3. It is provided as an example of how the code would look
  if the hardware supported SD_MMC mode.

  The standard SD_MMC library for Arduino ESP32 does not allow remapping pins via the .begin() function.
  It relies on the default hardware pins configured in the ESP32's IO MUX. The defines below
  are for documentation and will NOT be used by the SD_MMC.begin() call.
  To use custom pins, you would need to modify the underlying ESP-IDF configuration.
  *********************

  This sketch turns the LILYGO T-Dongle S3 into a dual-mode device:
  1. SFTP Server: Allows wireless file transfer to/from an SD card.
  2. USB Mass Storage (Thumb Drive): Allows the computer to read the SD card as a USB drive.

  How it works:
  - On startup, the sketch checks the state of the BOOT button (GPIO0).
  - If the BOOT button is NOT pressed, the device enters SFTP Server mode.
    - It connects to your Wi-Fi network.
    - It starts an SFTP server, allowing you to connect with a client like FileZilla.
  - If the BOOT button IS pressed during startup, the device enters USB Mass Storage mode.
    - It exposes the SD card as a USB drive to your computer.
    - The SFTP server is not started in this mode.

  Setup:
  1. Install the "SimpleFTPServer" library from the Arduino Library Manager.
  2. Make sure you have the latest ESP32 board package installed.
  3. In the Arduino IDE, select the "ESP32S3 Dev Module" board.
  4. Under "Tools", set "USB Mode" to "Hardware CDC and JTAG".
  5. Update the `ssid` and `password` variables with your Wi-Fi credentials.
  6. Update the `ftp_user` and `ftp_pass` variables with your desired SFTP login.
  7. Insert a FAT32 formatted microSD card into the T-Dongle S3.
*/

#include <WiFi.h>
#include <SD_MMC.h> // Using SD_MMC library instead of SD
#include <FS.h>
#include <SimpleFTPServer.h>
#include "USB.h"
#include "USBMSC.h"

// --- User Configuration ---
const char* ssid = "YOUR_WIFI_SSID";       // Your Wi-Fi network SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Your Wi-Fi network password
const char* ftp_user = "esp32";             // SFTP username
const char* ftp_pass = "esp32";             // SFTP password
// --- End of User Configuration ---

// --- Custom SD_MMC Pin Definitions ---
// NOTE: These are for documentation. The SD_MMC.h library does not use these defines
// and will use the default hardware pins for the SDMMC peripheral.
#define SD_MMC_D0_PIN 14
#define SD_MMC_D1_PIN 17
#define SD_MMC_D2_PIN 21
#define SD_MMC_D3_PIN 18
#define SD_MMC_CLK_PIN 12
#define SD_MMC_CMD_PIN 16
// --- End of Custom SD_MMC Pin Definitions ---

// The BOOT button is on GPIO0
#define BOOT_BUTTON_PIN 0

FtpServer ftpSrv;
USBMSC msc;

// Callback function for when the USB Mass Storage device is read
int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  // Using SD_MMC to read sectors from the card
  return SD_MMC.read((uint8_t*)buffer, lba, bufsize / 512);
}

// Callback function for when the USB Mass Storage device is written to
int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  // Using SD_MMC to write sectors to the card
  return SD_MMC.write((uint8_t*)buffer, lba, bufsize / 512);
}

// Callback function for when the USB Mass Storage device is started/stopped
void onStartStop(bool started) {
  if (started) {
    Serial.println("MSC Started");
  } else {
    Serial.println("MSC Stopped");
  }
}

// Function to set up and start USB Mass Storage mode
void startMassStorageMode() {
  Serial.println("BOOT button pressed. Entering USB Mass Storage mode.");
  
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  msc.begin(SD_MMC.cardSize() / 512, 512);
  USB.begin();

  Serial.println("USB Mass Storage device started. Connect to a computer to access the SD card.");
}

// Function to initialize the SD card
void sd_init() {
  Serial.println("Initializing SD card...");

  // The following line is added as requested, but is commented out because
  // the SD_MMC library for Arduino does not have a public `setPins` function.
  // Calling it would result in a compilation error. Pin mapping must be done
  // at a lower level (ESP-IDF).
  // Also, corrected a typo in the original request (D2 was listed twice).
  // SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);

  // Initialize the SD card using SD_MMC
  // This will use the dedicated SDMMC peripheral and pins.
  // It will likely fail on a standard T-Dongle S3.
  if (!SD_MMC.begin()) {
    Serial.println("SD_MMC Card Mount Failed. This is expected if the T-Dongle S3 hardware uses SPI for the SD card.");
    // You might want to halt here or blink an LED to indicate an error
    while (1);
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    while (1);
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial to initialize

  // Configure the BOOT button pin
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("LILYGO T-Dongle S3 - SFTP Server & USB Drive (SD_MMC Version)");

  // Initialize the SD card
  sd_init();

  // Check if the BOOT button is pressed to decide the mode
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    // --- USB Mass Storage Mode ---
    startMassStorageMode();
  } else {
    // --- SFTP Server Mode ---
    Serial.println("Entering SFTP Server mode.");

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start FTP server
    // Note: The SimpleFTPServer library uses the default filesystem, which we've now
    // initialized as SD_MMC.
    ftpSrv.begin(ftp_user, ftp_pass);
    Serial.println("FTP Server started.");
    Serial.println("You can now connect with an FTP client.");
  }
}

void loop() {
  // In SFTP mode, we need to handle FTP client requests
  // In USB MSC mode, the loop can be empty as it's handled by interrupts
  if (digitalRead(BOOT_BUTTON_PIN) != LOW) {
    ftpSrv.handleFTP();
  }
}
