/******************************************************************************
 *
 * digital-picture-frame
 * ----------------
 * transforms a LILYGO T-Dongle-S3 into a wireless adapter for a digital
 * picture frame, enabling you to upload pictures and manage files remotely.
 *
 * @author Nicholas Wilde, 0xb299a622
 * @date 25 Jul 2025
 * @version 0.1.0
 *
 *****************************************************************************/ 
 
#include "Arduino.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SimpleFTPServer.h>
#include <SPI.h>
#include <SD.h>
#include <SD_MMC.h>
#include "USB.h"
#include "USBMSC.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// --- Personal header files ---
#include "secrets.h" // Import sensitive data
#include "catppuccin_colors.h" // Include our custom color palette

// --- External libraries ---
#include <OneButton.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <FastLED.h>   // https://github.com/FastLED/FastLED

// --- LED Configuration ---
#define NUM_LEDS 1
#define LED_DI_PIN 40
#define LED_CI_PIN 39
#define BRIGHTNESS 13 // 5% of 255

// --- Button Configuration ---
#define BTN_PIN 0

// --- Mode Descriptions ---
const char* MODE_MSC_DESC = "USB MSC";
const char* MODE_FTP_DESC = "Application (FTP Server)";
 
// --- Create objects ---
WebServer server(80);
OneButton button(BTN_PIN, true); // true for active low
FtpServer ftpServer;
CRGB leds[NUM_LEDS];
USBMSC MSC;
USBCDC USBSerial;

#define HWSerial    Serial0
#define MOUNT_POINT "/sdcard"
sdmmc_card_t *card;

// --- A flag to track the current mode ---
bool isInMscMode = true;

// --- Function prototypes ---
void connectToWiFi();
void setupApiRoutes();
void setupSerial();
void enterMscMode();
bool enterFtpMode();
void handleStatus();
void handleSwitchToMSC();
void handleSwitchToFTP();
void handleRestart();
void toggleMode();
void msc_init();
void sd_init();
void ftp_transfer_callback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize);
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);
static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void drawHeader(const char* title);
void drawStorageInfo(int files, float totalSizeMB, float freeSizeMB);
void drawApModeScreen(const char* ap_ssid, const char* ap_ip);
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawUsbMscModeScreen(const char* mac, int files, int totalSizeMB, float freeSizeMB);

/*--------------------*
 * --- Main Logic --- *
 *--------------------*/

/**
 * @brief 
 */
void setup(){
  setupSerial();

  // --- Initialize the LED pin as an output
  FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Turn the LED on
  leds[0] = CRGB::Yellow;
  FastLED.show();
    
  // --- Initialize Button ---
  button.attachClick(toggleMode);
  
  // --- Connect to WiFi ---
  connectToWiFi();

  // --- Setup and start Web Server ---
  setupApiRoutes();
  server.begin();
  HWSerial.println("HTTP server started.");
  
  // Start in MSC mode
  sd_init();
  HWSerial.println("SD Card initialized for MSC.");

  if (card) {
    // Turn the LED on
    leds[0] = CRGB::Green;
    FastLED.show();
    USB.onEvent(usbEventCallback);
    msc_init();
    USBSerial.begin();
    USB.begin();
    HWSerial.println("\n✅ Started in MSC mode. Connect USB to a computer.");
  } else {
    HWSerial.println("\n❌ Failed to start in MSC mode. SD Card not found.");
  }
}

void loop(){
  button.tick();
  server.handleClient();
  if (!isInMscMode) {
    ftpServer.handleFTP(); // Continuously process FTP requests  
  }
}

/*---------------------*
 * ---  --- *
 *---------------------*/

/**
 * @brief 
 */
void setupSerial() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial) {
    if (millis() - start > 2000) { // 2-second timeout
      break;
    }
  }
  HWSerial.setDebugOutput(true);
  delay(100);
}

/**
 * @brief 
 */
void sd_init(void) {
  esp_err_t ret;
  const char mount_point[] = MOUNT_POINT;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

  sdmmc_host_t host = {
      .flags = SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_DDR,
      .slot = SDMMC_HOST_SLOT_1,
      .max_freq_khz = SDMMC_FREQ_DEFAULT,
      .io_voltage = 3.3f,
      .init = &sdmmc_host_init,
      .set_bus_width = &sdmmc_host_set_bus_width,
      .get_bus_width = &sdmmc_host_get_slot_width,
      .set_bus_ddr_mode = &sdmmc_host_set_bus_ddr_mode,
      .set_card_clk = &sdmmc_host_set_card_clk,
      .do_transaction = &sdmmc_host_do_transaction,
      .deinit = &sdmmc_host_deinit,
      .io_int_enable = sdmmc_host_io_int_enable,
      .io_int_wait = sdmmc_host_io_int_wait,
      .command_timeout_ms = 0,
  };
  sdmmc_slot_config_t slot_config = {
      .clk = (gpio_num_t)SD_MMC_CLK_PIN,
      .cmd = (gpio_num_t)SD_MMC_CMD_PIN,
      .d0 = (gpio_num_t)SD_MMC_D0_PIN,
      .d1 = (gpio_num_t)SD_MMC_D1_PIN,
      .d2 = (gpio_num_t)SD_MMC_D2_PIN,
      .d3 = (gpio_num_t)SD_MMC_D3_PIN,
      .cd = SDMMC_SLOT_NO_CD,
      .wp = SDMMC_SLOT_NO_WP,
      .width = 4, // SDMMC_SLOT_WIDTH_DEFAULT,
      .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
  };

  gpio_set_pull_mode((gpio_num_t)SD_MMC_CMD_PIN, GPIO_PULLUP_ONLY); // CMD, needed in 4- and 1- line modes
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D0_PIN, GPIO_PULLUP_ONLY);  // D0, needed in 4- and 1-line modes
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D1_PIN, GPIO_PULLUP_ONLY);  // D1, needed in 4-line mode only
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D2_PIN, GPIO_PULLUP_ONLY);  // D2, needed in 4-line mode only
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D3_PIN, GPIO_PULLUP_ONLY);  // D3, needed in 4- and 1-line modes

  ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      USBSerial.println("Failed to mount filesystem. "
                        "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    } else {
      USBSerial.printf("Failed to initialize the card (%s). "
                       "Make sure SD card lines have pull-up resistors in place.",
                       esp_err_to_name(ret));
    }
    return;
  }
}

/**
 * @brief 
 */
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  // HWSerial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_write_sectors(card, buffer + offset, lba, count);
  return bufsize;
}

/**
 * @brief 
 */
static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  // HWSerial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_read_sectors(card, (uint8_t*)buffer + offset, lba, count);
  return bufsize;
}

/**
 * @brief 
 */
static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  HWSerial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

/**
 * @brief 
 */
static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id) {
    case ARDUINO_USB_STARTED_EVENT: HWSerial.println("USB PLUGGED"); break;
    case ARDUINO_USB_STOPPED_EVENT: HWSerial.println("USB UNPLUGGED"); break;
    case ARDUINO_USB_SUSPEND_EVENT: HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en); break;
    case ARDUINO_USB_RESUME_EVENT: HWSerial.println("USB RESUMED"); break;
    default: break;
    }
  }
}

/**
 * @brief 
 */
void msc_init(void) {
  MSC.vendorID("LILYGO");       // max 8 chars
  MSC.productID("T-Dongle-S3"); // max 16 chars
  MSC.productRevision("1.0");   // max 4 chars
  MSC.onStartStop(onStartStop);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.mediaPresent(true);
  MSC.begin(card->csd.capacity, card->csd.sector_size);
}

/*---------------------*
 * ---  --- *
 *---------------------*/

/**
 * @brief Connects to the WiFi network and provides visual feedback.
 */
void connectToWiFi() {
  HWSerial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  // wm.resetSettings(); // wipe settings
  
  // Set up a callback for when the captive portal is entered
  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    HWSerial.println("Entered config mode");
    HWSerial.println(WiFi.softAPIP());
    HWSerial.println(myWiFiManager->getConfigPortalSSID());
    leds[0] = CRGB::Blue; // Solid blue for captive portal
    FastLED.show();
  });

  // Blink blue while connecting
  unsigned long startConnecting = millis();
  bool connecting = true;
  while (connecting && (millis() - startConnecting < 180000)) { // 3-minute timeout
    leds[0] = CRGB::Blue;
    FastLED.show();
    delay(100);
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(400);
    if (wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
      connecting = false;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    HWSerial.println("Failed to connect and hit timeout. Restarting...");
    ESP.restart();
  } else {
    HWSerial.println("\nWiFi connected!");
    HWSerial.printf("Connected to: %s\n", WIFI_SSID);
    HWSerial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  }
}

/*---------------------*
 * ---  --- *
 *---------------------*/

/**
 * @brief Toggles between FTP and MSC modes when the button is pressed.
 */
void toggleMode() {
  HWSerial.println("Button clicked! Toggling mode...");
  if (isInMscMode) {
    enterFtpMode();
  } else {
    enterMscMode();
  }
}

/**
 * @brief Defines the web server API endpoints.
 */
void setupApiRoutes() {
  server.on("/", HTTP_GET, handleStatus);
  server.on("/msc", HTTP_POST, handleSwitchToMSC);
  server.on("/ftp", HTTP_POST, handleSwitchToFTP);
  server.on("/restart", HTTP_POST, handleRestart);
}

/**
 * @brief Stops FTP, unmounts SD, and enables USB MSC mode.
 */
void enterMscMode() {
  if (isInMscMode) return; // Already in this mode
  
  HWSerial.println("\n--- Entering MSC Mode ---");

  // Turn the LED on
  leds[0] = CRGB::Green;
  FastLED.show();
    
  // Stop FTP Server
  ftpServer.end();
  HWSerial.println("FTP Server stopped.");

  // Unmount SD_MMC
  SD_MMC.end();
  HWSerial.println("SD Card unmounted from SD_MMC.");

  // Initialize SD for MSC
  sd_init();
  HWSerial.println("SD Card initialized for MSC.");

  // Initialize USB MSC
  if (card) {
    USB.onEvent(usbEventCallback);
    msc_init();
    USBSerial.begin();
    USB.begin();
    HWSerial.println("\n✅ Switched to MSC mode. Connect USB to a computer.");
    isInMscMode = true;
  } else {
    HWSerial.println("\n❌ Failed to switch to MSC mode. SD Card not found.");
  }
}

/**
 * @brief Mounts SD card and starts the FTP server.
 * @return true if successful, false otherwise.
 */
bool enterFtpMode() {
  if (!isInMscMode) return true; // Already in this mode

  HWSerial.println("\n--- Entering Application (FTP) Mode ---");

  // Turn the LED on
  leds[0] = CRGB::Orange;
  FastLED.show();
  
  // Stop USB MSC
  MSC.end();
  USBSerial.end();
  HWSerial.println("USB MSC stopped.");


  // Unmount SD from VFS
  if (card) {
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    HWSerial.println("SD Card unmounted from VFS.");
  }

  // Initialize SD_MMC
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
  if (!SD_MMC.begin("/sdcard", true)) {
    HWSerial.println("Card Mount Failed");
    return false;
  }
  HWSerial.println("SD Card mounted with SD_MMC.");

  // Start FTP Server
  ftpServer.begin(FTP_USER, FTP_PASSWORD);
  ftpServer.setTransferCallback(ftp_transfer_callback);
  HWSerial.println("FTP Server started.");

  HWSerial.println("\n✅ Application mode active.");
  isInMscMode = false;
  return true;
}

/**
 * @brief Handles requests to the root URL (")/"). Sends a JSON status object.
 */
void handleStatus() {
  const char* modeString = isInMscMode ? MODE_MSC_DESC : MODE_FTP_DESC;
  String jsonResponse = "{\"mode\":\"" + String(modeString) + "\"}";
  server.send(200, "application/json", jsonResponse);
}

/**
 * @brief Handles the POST request to switch to MSC mode.
 */
void handleSwitchToMSC() {
  if (isInMscMode) {
    String jsonResponse = "{\"status\":\"no_change\", \"message\":\"Already in MSC mode.\"}";
    server.send(200, "application/json", jsonResponse);
  } else {
    enterMscMode();
    if (isInMscMode) {
      String jsonResponse = "{\"status\":\"success\", \"message\":\"Switched to MSC mode.\"}";
      server.send(200, "application/json", jsonResponse);
    } else {
      String jsonResponse = "{\"status\":\"error\", \"message\":\"Failed to switch to MSC mode.\"}";
      server.send(500, "application/json", jsonResponse);
    }
  }
  
}
 
/**
 * @brief Handles the POST request to switch back to Application (FTP) mode.
 */
void handleSwitchToFTP() {
  if (isInMscMode) {
    if (enterFtpMode()) {
      String jsonResponse = "{\"status\":\"success\", \"message\":\"Switched to Application (FTP) mode.\"}";
      server.send(200, "application/json", jsonResponse);
    } else {
      String jsonResponse = "{\"status\":\"error\", \"message\":\"Failed to re-initialize SD card.\"}";
      server.send(500, "application/json", jsonResponse);
    }
  }
  else {
    String jsonResponse = "{\"status\":\"no_change\", \"message\":\"Already in Application (FTP) mode.\"}";
    server.send(200, "application/json", jsonResponse);
  }
}

/**
 * @brief Handles the POST request to restart the device.
 */
void handleRestart() {
  String jsonResponse = "{\"status\":\"success\", \"message\":\"Restarting device...\"}";
  server.send(200, "application/json", jsonResponse);
  delay(1000); // Give the server time to send the response
  ESP.restart();
}

/**
 * @brief Callback function for FTP transfers. Blinks the LED during transfer
 *        and leaves it solid on when complete.
 */
void ftp_transfer_callback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize) {
  if (ftpOperation == FTP_UPLOAD || ftpOperation == FTP_DOWNLOAD) {
    // Blink LED by turning it OFF briefly, then back ON.
    // This leaves the LED in the ON state after the pulse.
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(50);
    leds[0] = CRGB::Orange;
    FastLED.show();
  } else if (ftpOperation == FTP_UPLOAD_STOP || ftpOperation == FTP_DOWNLOAD_STOP || ftpOperation == FTP_TRANSFER_ERROR) {
    // Ensure LED is solid orange after any transfer completion or error.
    leds[0] = CRGB::Orange;
    FastLED.show();
  }
}

/*---------------------*
 * --- LCD Display --- *
 *---------------------*/

/**
 * @brief Draws the top header bar
 */
void drawHeader(const char* title) {
  tft.fillRect(0, 0, TFT_HEIGHT, 12, CATPPUCCIN_BLUE);
  tft.setTextColor(CATPPUCCIN_CRUST);
  tft.setTextSize(1);
  tft.drawCentreString(title, TFT_WIDTH, 2, 1); // x-center, y, font
  tft.setTextSize(1);
}

/**
 * @brief Draws the right column with storage statistics
 */
void drawStorageInfo(int files, int totalSizeMB, float freeSizeMB) {
  int y_pos = 53;
  int x_pos = 5;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Size:  ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(totalSizeMB);
  tft.print("MB");

  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print(" Files: ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(files);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Free:  ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(freeSizeMB, 0);
  tft.print("MB");
}

/**
 * @brief 
 */
void drawApModeScreen(const char* ap_ssid, const char* ap_ip) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi Setup");

  // Left Column: Network Info
  int y_pos = 17;
  int x_pos = 5;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:  ");
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("AP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("AP IP: ");
  tft.setTextColor(CATPPUCCIN_YELLOW); // Use green for active mode
  tft.print(ap_ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("SSID:  ");
  y_pos += 12;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW); // Use green for active mode
  tft.print(ap_ssid);
}

/**
 * @brief
 */
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi");

  // Left Column: Network Info
  int y_pos = 17;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:  ");
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("FTP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP:    ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:   ");
  // y_pos += 10;
  // tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(mac);

  // Right Column: Storage Info
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

/**
 * @brief 
 */
void drawUsbMscModeScreen(const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi");

  // Left Column: Network Info
  int y_pos = 17;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:  ");
  tft.setTextColor(CATPPUCCIN_GREEN);
  tft.print("USB MSC");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP:    ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print("N/A");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:   ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(mac);

  // Right Column: Storage Info
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}