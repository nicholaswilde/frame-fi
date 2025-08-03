/******************************************************************************
 * FrameFi
 * ----------------
 * transforms a LILYGO T-Dongle-S3 into a wireless adapter for a digital
 * picture frame, enabling you to upload pictures and manage files remotely.
 *
 * @author Nicholas Wilde, 0xb299a622
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
#include <dirent.h>

// --- Personal header files ---
#include "secrets.h" // Import sensitive data
#include "catppuccin_colors.h" // Include our custom color palette

// --- External libraries ---
#include <OneButton.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <FastLED.h>   // https://github.com/FastLED/FastLED
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI

// --- LED Configuration ---

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
TFT_eSPI tft = TFT_eSPI();

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
void handleSwitchToMsc();
void handleSwitchToFtp();
void handleRestart();
void toggleMode();
void resetWifiSettings();
void mscInit();
void sdInit();
void ftpTransferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize);
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);
static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void drawHeader(const char* title, uint16_t bannerColor);
void drawStorageInfo(int files, float totalSizeMB, float freeSizeMB);
void drawApModeScreen(const char* ap_ssid, const char* ap_ip);
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawUsbMscModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawBootScreen();
void drawResetWiFiSettingsScreen();
int countFiles(File dir);
int countFilesInPath(const char *path);

// --- Main Logic ---

/**
 * @brief Initializes the device and all its components.
 */
void setup(){
  setupSerial();

  // --- Initialize the LED pin as an output ---
  FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(leds, NUM_LEDS);
#if defined(LED_BRIGHTNESS)
  FastLED.setBrightness(LED_BRIGHTNESS);
#else
  FastLED.setBrightness(13); // 5% of 255
#endif

  // --- Turn the LED on ---
  leds[0] = CRGB::Yellow;
  FastLED.show();
    
  // --- Initialize Button ---
  button.attachClick(toggleMode);
  button.setPressMs(3000); // 3 seconds
  button.attachLongPressStop(resetWifiSettings);

  // --- Initialize TFT Display ---
  pinMode(TFT_LEDA, OUTPUT);
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  tft.init();
  tft.setRotation(DISPLAY_ORIENTATION); // Adjust rotation as needed
  tft.fillScreen(CATPPUCCIN_BASE);
  digitalWrite(TFT_LEDA, LOW);
#else
  digitalWrite(TFT_LEDA, HIGH);
#endif
  
  // --- Show boot screen ---
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawBootScreen();
#endif
  delay(2000); // Keep boot screen visible for 2 seconds
  
  // --- Connect to WiFi ---
  connectToWiFi();

  // --- Setup and start Web Server ---
  setupApiRoutes();
  server.begin();
  HWSerial.println("HTTP server started.");
  
  // --- Start in MSC mode ---
sdInit();
  HWSerial.println("SD Card initialized for MSC.");

  if (card) {
    // --- Turn the LED on ---
    leds[0] = CRGB::Green;
    FastLED.show();
    USB.onEvent(usbEventCallback);
    mscInit();
    USBSerial.begin();
    USB.begin();
    HWSerial.println("\n✅ Started in MSC mode. Connect USB to a computer.");
    // --- Display initial mode screen ---
    enterMscMode();
    
    // --- Display MSC mode screen ---
    int numFiles = countFilesInPath(MOUNT_POINT);
    FATFS *fs;
    DWORD fre_clust;
    f_getfree(MOUNT_POINT, &fre_clust, &fs);
    uint64_t totalBytes = (uint64_t)(fs->n_fatent - 2) * fs->csize * fs->ssize;
    uint64_t freeBytes = (uint64_t)fre_clust * fs->csize * fs->ssize;
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
    drawUsbMscModeScreen(WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), numFiles, totalBytes / (1024 * 1024), freeBytes / (1024.0 * 1024.0));
#endif
  } else {
    HWSerial.println("\n❌ Failed to start in MSC mode. SD Card not found.");
  }
}

/**
 * @brief Main loop that runs repeatedly.
 */
void loop(){
  button.tick();
  server.handleClient();
  if (!isInMscMode) {
    ftpServer.handleFTP(); // Continuously process FTP requests  
  }
}

// --- Serial Communications ---

/**
 * @brief Initializes the serial communication.
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
  HWSerial.println(APP_VERSION);
  delay(100);
}

// --- USB Mass Storage Control ---

/**
 * @brief Initializes the SD card.
 */
void sdInit(void) {
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
 * @brief Writes data to the SD card.
 */
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  // HWSerial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_write_sectors(card, buffer + offset, lba, count);
  return bufsize;
}

/**
 * @brief Reads data from the SD card.
 */
static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  // HWSerial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_read_sectors(card, (uint8_t*)buffer + offset, lba, count);
  return bufsize;
}

/**
 * @brief Handles the start and stop of the MSC device.
 */
static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  HWSerial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

/**
 * @brief Handles USB events.
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
 * @brief Initializes the USB MSC device.
 */
void mscInit(void) {
  MSC.vendorID("LILYGO");       // max 8 chars
  MSC.productID("T-Dongle-S3"); // max 16 chars
  MSC.productRevision("1.0");   // max 4 chars
  MSC.onStartStop(onStartStop);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.mediaPresent(true);
  MSC.begin(card->csd.capacity, card->csd.sector_size);
}

// --- WiFi ---

/**
 * @brief Connects to the WiFi network and provides visual feedback.
 */
void connectToWiFi() {
  HWSerial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  // wm.resetSettings(); // wipe settings
  
  // --- Set up a callback for when the captive portal is entered ---
  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    HWSerial.println("Entered config mode");
    HWSerial.println(WiFi.softAPIP());
    HWSerial.println(myWiFiManager->getConfigPortalSSID());
    leds[0] = CRGB::Blue; // Solid blue for captive portal
    FastLED.show();
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
    drawApModeScreen(myWiFiManager->getConfigPortalSSID().c_str(), WiFi.softAPIP().toString().c_str());
#endif
  });

  // --- Blink blue while connecting ---
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

// --- Button Actions ---

/**
 * @brief Resets WiFi settings if the button is held for 3 seconds.
 */
void resetWifiSettings() {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawResetWiFiSettingsScreen();
#endif
  HWSerial.println("Button held for 3 seconds. Resetting WiFi settings...");
  WiFiManager wm;
  wm.resetSettings();
  HWSerial.println("WiFi settings reset. Restarting...");
  ESP.restart();
}

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

// --- Web Server ---

/**
 * @brief Defines the web server API endpoints.
 */
void setupApiRoutes() {
  server.on("/", HTTP_GET, handleStatus);
  server.on("/msc", HTTP_POST, handleSwitchToMsc);
  server.on("/ftp", HTTP_POST, handleSwitchToFtp);
  server.on("/restart", HTTP_POST, handleRestart);
}

/**
 * @brief Stops FTP, unmounts SD, and enables USB MSC mode.
 */
void enterMscMode() {
  if (isInMscMode) return; // Already in this mode
  
  HWSerial.println("\n--- Entering MSC Mode ---");

  // --- Turn the LED on ---
  leds[0] = CRGB::Green;
  FastLED.show();
    
  // --- Stop FTP Server ---
  ftpServer.end();
  HWSerial.println("FTP Server stopped.");

  // --- Unmount SD_MMC ---
  SD_MMC.end();
  HWSerial.println("SD Card unmounted from SD_MMC.");

  // --- Initialize SD for MSC ---
  sdInit();
  HWSerial.println("SD Card initialized for MSC.");

  // --- Initialize USB MSC ---
  if (card) {
    USB.onEvent(usbEventCallback);
    mscInit();
    USBSerial.begin();
    USB.begin();
    HWSerial.println("\n✅ Switched to MSC mode. Connect USB to a computer.");
    isInMscMode = true;

    // --- Display MSC mode screen ---
    int numFiles = countFilesInPath(MOUNT_POINT);
    FATFS *fs;
    DWORD fre_clust;
    f_getfree(MOUNT_POINT, &fre_clust, &fs);
    uint64_t totalBytes = (uint64_t)(fs->n_fatent - 2) * fs->csize * fs->ssize;
    uint64_t freeBytes = (uint64_t)fre_clust * fs->csize * fs->ssize;
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
    drawUsbMscModeScreen(WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), numFiles, totalBytes / (1024 * 1024), freeBytes / (1024.0 * 1024.0));
#endif
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

  // --- Turn the LED on ---
  leds[0] = CRGB::Orange;
  FastLED.show();
  
  // --- Stop USB MSC ---
  MSC.mediaPresent(false);
  delay(2000);
  MSC.end();
  USBSerial.end();
  HWSerial.println("USB MSC stopped.");

  // --- Unmount SD from VFS ---
  if (card) {
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    HWSerial.println("SD Card unmounted from VFS.");
  }

  // --- Initialize SD_MMC ---
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
  if (!SD_MMC.begin(MOUNT_POINT, true)) {
    HWSerial.println("Card Mount Failed");
    return false;
  }
  HWSerial.println("SD Card mounted with SD_MMC.");

  // --- Start FTP Server ---
  ftpServer.begin(FTP_USER, FTP_PASSWORD);
  ftpServer.setTransferCallback(ftpTransferCallback);
  HWSerial.println("FTP Server started.");

  HWSerial.println("\n✅ Application mode active.");
  isInMscMode = false;

  // --- Display FTP mode screen ---
  File root = SD_MMC.open("/");
  int numFiles = countFiles(root);
  root.close();
  uint64_t totalBytes = SD_MMC.cardSize();
  uint64_t usedBytes = SD_MMC.usedBytes();
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawFtpModeScreen(WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), numFiles, totalBytes / (1024 * 1024), (totalBytes - usedBytes) / (1024.0 * 1024.0));
#endif
  
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
void handleSwitchToMsc() {
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
void handleSwitchToFtp() {
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

// --- FTP Server ---

/**
 * @brief Callback function for FTP transfers.
 */
void ftpTransferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize) {
  if (ftpOperation == FTP_UPLOAD || ftpOperation == FTP_DOWNLOAD) {
    // --- Blink LED by turning it OFF briefly, then back ON ---
    // --- This leaves the LED in the ON state after the pulse ---
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(50);
    leds[0] = CRGB::Orange;
    FastLED.show();
  } else if (ftpOperation == FTP_UPLOAD_STOP || ftpOperation == FTP_DOWNLOAD_STOP || ftpOperation == FTP_TRANSFER_ERROR) {
    // --- Ensure LED is solid orange after any transfer completion or error ---
    leds[0] = CRGB::Orange;
    FastLED.show();
  }
}

// --- File System ---

/**
 * @brief Counts the number of files in a directory recursively using VFS.
 */
int countFilesInPath(const char *path) {
  DIR *dir = opendir(path);
  if (!dir) {
    HWSerial.printf("Error opening directory: %s\n", path);
    return 0;
  }
  int count = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      count++;
    } else if (entry->d_type == DT_DIR) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        char subpath[512];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);
        count += countFilesInPath(subpath);
      }
    }
  }
  closedir(dir);
  return count;
}

/**
 * @brief Counts the number of files in a directory recursively.
 */
int countFiles(File dir) {
  int count = 0;
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // --- no more files ---
      break;
    }
    if (entry.isDirectory()) {
      count += countFiles(entry);
    } else {
      count++;
    }
    entry.close();
  }
  return count;
}

// --- Display ---

/**
 * @brief Draws the top header bar.
 */
void drawHeader(const char* title, uint16_t bannerColor) {
  tft.fillRect(0, 0, TFT_HEIGHT, 12, bannerColor);
  tft.setTextColor(CATPPUCCIN_CRUST);
  tft.setTextSize(1);
  tft.drawCentreString(title, TFT_WIDTH, 2, 1); // x-center, y, font
  tft.setTextSize(1);
}

/**
 * @brief Draws the right column with storage statistics.
 */
void drawStorageInfo(int files, int totalSizeMB, float freeSizeMB) {
  int y_pos = 53;
  int x_pos = 5;

  // --- Calculate usage ---
  float usedSizeMB = totalSizeMB - freeSizeMB;
  float usedPercentage = (totalSizeMB > 0) ? (usedSizeMB / totalSizeMB) * 100 : 0;

  // --- Convert to GB for display ---
  float totalSizeGB = totalSizeMB / 1024.0;
  float usedSizeGB = usedSizeMB / 1024.0;

  // --- Draw storage text info ---
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Size:  ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(totalSizeGB, 2);
  tft.print("GB");

  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print(" Files: ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(files);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Used:  ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(usedSizeGB, 2);
  tft.print("GB (");
  tft.print((int)usedPercentage);
  tft.print("%) ");
  y_pos += 12;

  // --- Draw capacity bar ---
  int bar_x = x_pos;
  int bar_y = y_pos;
  int bar_width = TFT_HEIGHT - (2 * x_pos); // Bar width spans the screen with padding
  int bar_height = 8;
  int filled_width = (bar_width * usedPercentage) / 100;

  // --- Draw the bar background (empty part) ---
  tft.drawRect(bar_x, bar_y, bar_width, bar_height, CATPPUCCIN_BASE);
  // --- Draw the filled part of the bar ---
  tft.fillRect(bar_x, bar_y, filled_width, bar_height, CATPPUCCIN_GREEN);
}

/**
 * @brief Displays the boot-up screen.
 */
void drawBootScreen() {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", CATPPUCCIN_BLUE);

  int y_pos = 30;
  int x_pos = tft.width() / 2; // Center horizontally

  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.setTextSize(1);
  tft.drawCentreString("Booting...", x_pos, y_pos, 2);
  y_pos += 25;

  tft.setTextSize(1);
  tft.setTextColor(CATPPUCCIN_LAVENDER);
  String versionString = String(APP_VERSION);
  tft.drawCentreString(versionString.c_str(), x_pos, y_pos, 1);
}

/**
 * @brief Displays the resetting wifi settings screen.
 */
void drawResetWiFiSettingsScreen() {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", CATPPUCCIN_RED);

  int y_pos = 30;
  int x_pos = tft.width() / 2; // Center horizontally

  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.setTextSize(1);
  tft.drawCentreString("Resetting WiFi...", x_pos, y_pos, 2);
  y_pos += 25;

  tft.setTextSize(1);
  tft.setTextColor(CATPPUCCIN_LAVENDER);
  tft.drawCentreString("Restarting...", x_pos, y_pos, 1);
}

/**
 * @brief Displays the AP mode screen.
 */
void drawApModeScreen(const char* ap_ssid, const char* ap_ip) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi Setup", CATPPUCCIN_YELLOW);

  // --- Left Column: Network Info ---
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
 * @brief Displays the FTP mode screen.
 */
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", CATPPUCCIN_GREEN);

  // --- Left Column: Network Info ---
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

  // --- Right Column: Storage Info ---
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

/**
 * @brief Displays the USB MSC mode screen.
 */
void drawUsbMscModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", CATPPUCCIN_MAUVE);

  // --- Left Column: Network Info ---
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
  tft.print(ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:   ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(mac);

  // --- Right Column: Storage Info ---
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}
