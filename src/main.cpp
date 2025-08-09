/******************************************************************************
 * 
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
#include <PubSubClient.h>
#include <ArduinoJson.h>     // https://github.com/bblanchon/ArduinoJson
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include <LittleFS.h>

// --- Data Structure for Device Information ---
struct DeviceInfo {
  // Mode and Display
  const char* modeString;
  bool isInMscMode;
  const char* displayStatus;
  bool isDisplayOn;
  int displayOrientation;

  // Network
  String ipAddress;
  String macAddress;

  // SD Card
  int fileCount;
  uint64_t totalSize;
  uint64_t usedSize;
  uint64_t freeSize;

  // MQTT
  int mqttState;
  bool mqttConnected;
};

// --- Function to populate device info ---
void getDeviceInfo(DeviceInfo& info);

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
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define HWSerial    Serial0
#define MOUNT_POINT "/sdcard"
sdmmc_card_t *card;

// --- MQTT Configuration ---
#define MQTT_CONFIG_FILE "/mqtt_config.json"
bool shouldSaveMqttConfig = false;
char mqtt_host[64] = "192.168.1.100";
char mqtt_port[6] = "1883";
char mqtt_user[32] = "";
char mqtt_pass[32] = "";
char mqtt_client_id[32] = "FrameFi";


// --- A flag to track the current mode ---
bool isInMscMode = true;
bool isDisplayOn = true; // A flag to track the display status

// --- MSC screen refresh tracking ---
volatile bool msc_disk_dirty = false;
volatile unsigned long last_msc_write_time = 0;
const unsigned long MSC_REFRESH_DEBOUNCE_MS = 2000; // 2 seconds

// --- MQTT Topics ---
#define MQTT_STATE_TOPIC "frame-fi/state"
#define MQTT_DISPLAY_STATUS_TOPIC "frame-fi/display/status"
#define MQTT_DISPLAY_SET_TOPIC "frame-fi/display/set"

// --- Timers ---
unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 300000; // 5 minutes
// Timer variables for non-blocking reconnection
unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 5000; // Interval to wait between retries (5 seconds)

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
void handleDisplayToggle();
void handleDisplayOn();
void handleDisplayOff();
void handleWifiReset();
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
void drawStorageInfoPortrait(int files, int totalSizeMB, float freeSizeMB);
void drawStorageInfoLandscape(int files, int totalSizeMB, float freeSizeMB);
void drawApModeScreen(const char* ap_ssid, const char* ap_ip);
void drawApModeScreenPortrait(const char* ap_ssid, const char* ap_ip);
void drawApModeScreenLandscape(const char* ap_ssid, const char* ap_ip);
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawFtpModeScreenPortrait(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawFtpModeScreenLandscape(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawUsbMscModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawUsbMscModeScreenPortrait(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawUsbMscModeScreenLandscape(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawBootScreen();
void drawResetWiFiSettingsScreen();
int countFiles(File dir);
int countFilesInPath(const char *path);
void updateAndDrawMscScreen();
void setupMqtt();
void publishMqttStatus();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void saveMqttConfigCallback();
void saveMqttConfig();
void loadConfig();

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
  button.attachLongPressStart(resetWifiSettings);

  // --- Initialize TFT Display ---
  pinMode(TFT_LEDA, OUTPUT);
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  tft.init();
  tft.setRotation(DISPLAY_ORIENTATION); // Adjust rotation as needed
  tft.fillScreen(CATPPUCCIN_BASE);
  if (isDisplayOn) {
    digitalWrite(TFT_LEDA, LOW); // Turn display on
  } else {
    digitalWrite(TFT_LEDA, HIGH); // Turn display off
  }
#else
  digitalWrite(TFT_LEDA, HIGH);
#endif
  
  // --- Show boot screen ---
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawBootScreen();
  delay(2000); // Keep boot screen visible for 2 seconds
#endif

  // Load existing configuration
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    HWSerial.println("LittleFS mount failed");
    return;
  }

  loadConfig();
#endif

  // --- Connect to WiFi ---
  connectToWiFi();

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  setupMqtt();
#endif

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
    updateAndDrawMscScreen();
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

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  if (!mqttClient.connected()) {
    long now = millis();
    // 2. Check if the reconnection interval has passed
    if (now - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = now;
      reconnect();
    }
  } else {
    mqttClient.loop();  
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastMqttPublish >= mqttPublishInterval) {
    lastMqttPublish = currentMillis;
    publishMqttStatus();
  }
#endif

  if (!isInMscMode) {
    ftpServer.handleFTP(); // Continuously process FTP requests  
  }

  // --- Check if the MSC screen needs to be refreshed ---
  if (isInMscMode && msc_disk_dirty && (millis() - last_msc_write_time > MSC_REFRESH_DEBOUNCE_MS)) {
    msc_disk_dirty = false; // Reset flag
    updateAndDrawMscScreen();
  }
}

/**
 * Loads the configuration from a file.
 */
void loadConfig() {
  if (LittleFS.exists(MQTT_CONFIG_FILE)) {
    HWSerial.println("Loading config");
    File configFile = LittleFS.open(MQTT_CONFIG_FILE, "r");
    if (configFile) {
      HWSerial.println("configFile is not error");
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, configFile);
      if (!error) {
        Serial.println("No error loading json config");   
        strcpy(mqtt_host, doc["mqtt_host"]);
        int myInt = doc["mqtt_port"];
        char buffer[6];
        sprintf(buffer, "%d", myInt);
        strcpy(mqtt_port, buffer);
        strcpy(mqtt_user, doc["mqtt_user"]);
        strcpy(mqtt_pass, doc["mqtt_pass"]);
        strcpy(mqtt_client_id, doc["mqtt_client_id"]);
      } else {
        HWSerial.println("Failed to load json config");
      }
      configFile.close();
    }
  } else {
    HWSerial.println("Config file not found, using defaults.");
  }
}

/**
 * Saves the configuration to a file.
 */
void saveMqttConfig() {
  HWSerial.println("Saving config");
  DynamicJsonDocument doc(1024);
  doc["mqtt_host"] = mqtt_host;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_user"] = mqtt_user;
  doc["mqtt_pass"] = mqtt_pass;
  doc["mqtt_client_id"] = mqtt_client_id;

  File configFile = LittleFS.open(MQTT_CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  serializeJson(doc, configFile);
  configFile.close();
}

void saveMqttConfigCallback() {
  Serial.println("Should save config");
  shouldSaveMqttConfig = true;
}

// --- Get Device Info ---
void getDeviceInfo(DeviceInfo& info) {
  info.isInMscMode = ::isInMscMode; // Use global isInMscMode
  info.isDisplayOn = ::isDisplayOn; // Use global isDisplayOn
  info.modeString = info.isInMscMode ? MODE_MSC_DESC : MODE_FTP_DESC;
  info.displayStatus = info.isDisplayOn ? "on" : "off";
  info.displayOrientation = tft.getRotation();
  info.ipAddress = WiFi.localIP().toString();
  info.macAddress = WiFi.macAddress();
  info.mqttState = mqttClient.state();
  info.mqttConnected = mqttClient.connected();

  if (info.isInMscMode) {
    if(card) {
      info.fileCount = countFilesInPath(MOUNT_POINT);
      FATFS *fs;
      DWORD fre_clust;
      if (f_getfree(MOUNT_POINT, &fre_clust, &fs) == FR_OK) {
        info.totalSize = (uint64_t)(fs->n_fatent - 2) * fs->csize * fs->ssize;
        info.freeSize = (uint64_t)fre_clust * fs->csize * fs->ssize;
        info.usedSize = info.totalSize - info.freeSize;
      } else {
        info.fileCount = 0;
        info.totalSize = 0;
        info.usedSize = 0;
        info.freeSize = 0;
      }
    } else {
      info.fileCount = 0;
      info.totalSize = 0;
      info.usedSize = 0;
      info.freeSize = 0;
    }
  } else {
    File root = SD_MMC.open("/");
    if (root) {
      info.fileCount = countFiles(root);
      root.close();
    } else {
      info.fileCount = 0;
    }
    info.totalSize = SD_MMC.cardSize();
    info.usedSize = SD_MMC.usedBytes();
    info.freeSize = info.totalSize - info.usedSize;
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

  // --- Track that a write has occurred ---
  msc_disk_dirty = true;
  last_msc_write_time = millis();

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
  if (load_eject) {
    // --- The host has ejected the device, a good time to refresh the screen ---
    updateAndDrawMscScreen();
  }
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
  
  // --- Configure WiFiManager ---
  String ap_ssid_str;
  const char* ap_ssid;
  const char* ap_password;

#if defined(WIFI_AP_SSID)
  ap_ssid = WIFI_AP_SSID;
#else
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  ap_ssid_str = "FrameFi-" + mac.substring(mac.length() - 6);
  ap_ssid = ap_ssid_str.c_str();
#endif

#if defined(WIFI_AP_PASSWORD)
  ap_password = WIFI_AP_PASSWORD;
#else
  ap_password = NULL;
#endif

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  // Set config save notify callback
  wm.setSaveConfigCallback(saveMqttConfigCallback);

  // Add custom parameters for MQTT
  WiFiManagerParameter custom_mqtt_client_id("mqtt_client_id", "MQTT Client ID", mqtt_client_id, 32);
  WiFiManagerParameter custom_mqtt_host("mqtt_host", "MQTT Host", mqtt_host, 64);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", mqtt_user, 32);
  WiFiManagerParameter custom_mqtt_pass("mqtt_pass", "MQTT Password", mqtt_pass, 32);

  wm.addParameter(&custom_mqtt_client_id);  
  wm.addParameter(&custom_mqtt_host);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
#endif

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
    if (wm.autoConnect(ap_ssid, ap_password)) {
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
  
  // Read updated parameters
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  strcpy(mqtt_host, custom_mqtt_host.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  strcpy(mqtt_client_id, custom_mqtt_client_id.getValue());
#endif
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

  // Erase the configuration file
  if (LittleFS.exists(MQTT_CONFIG_FILE)) {
    LittleFS.remove(MQTT_CONFIG_FILE);
    HWSerial.println("Configuration file has been deleted.");
  }
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
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
}

// --- Web Server ---

/**
 * @brief Defines the web server API endpoints.
 */
void setupApiRoutes() {
  server.on("/", HTTP_GET, handleStatus);
  server.on("/mode/msc", HTTP_POST, handleSwitchToMsc);
  server.on("/mode/ftp", HTTP_POST, handleSwitchToFtp);
  server.on("/device/restart", HTTP_POST, handleRestart);
  server.on("/display/toggle", HTTP_POST, handleDisplayToggle);
  server.on("/display/on", HTTP_POST, handleDisplayOn);
  server.on("/display/off", HTTP_POST, handleDisplayOff);
  server.on("/wifi/reset", HTTP_POST, handleWifiReset);
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
    updateAndDrawMscScreen();
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
    publishMqttStatus();
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
  DeviceInfo info;
  getDeviceInfo(info);
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawFtpModeScreen(info.ipAddress.c_str(), info.macAddress.c_str(), info.fileCount, info.totalSize / (1024 * 1024), info.freeSize / (1024.0 * 1024.0));
#endif
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
  return true;
}

/**
 * @brief Handles requests to the root URL ("/"). Sends a JSON status object.
 */
void handleStatus() {
  DeviceInfo info;
  getDeviceInfo(info);

  DynamicJsonDocument jsonResponse(1024);
  jsonResponse["mode"] = info.modeString;
  JsonObject display = jsonResponse.createNestedObject("display");
  display["status"] = info.displayStatus;
  display["orientation"] = info.displayOrientation;
  JsonObject sd_card = jsonResponse.createNestedObject("sd_card");
  sd_card["total_size"] = info.totalSize;
  sd_card["used_size"] = info.usedSize;
  sd_card["free_size"] = info.freeSize;
  sd_card["file_count"] = info.fileCount;
  JsonObject mqtt = jsonResponse.createNestedObject("mqtt");
  mqtt["state"] = info.mqttState;
  mqtt["connected"] = info.mqttConnected;

  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Handles the POST request to switch to MSC mode.
 */
void handleSwitchToMsc() {
  if (isInMscMode) {
    DynamicJsonDocument jsonResponse(256);
    jsonResponse["status"] = "no_change";
    jsonResponse["message"] = "Already in MSC mode.";
    String output;
    serializeJson(jsonResponse, output);
    server.send(200, "application/json", output);
  } else {
    enterMscMode();
    if (isInMscMode) {
      DynamicJsonDocument jsonResponse(256);
      jsonResponse["status"] = "success";
      jsonResponse["message"] = "Switched to MSC mode.";
      String output;
      serializeJson(jsonResponse, output);
      server.send(200, "application/json", output);
    } else {
      DynamicJsonDocument jsonResponse(256);
      jsonResponse["status"] = "error";
      jsonResponse["message"] = "Failed to switch to MSC mode.";
      String output;
      serializeJson(jsonResponse, output);
      server.send(500, "application/json", output);
    }
  }  
}
 
/**
 * @brief Handles the POST request to switch back to Application (FTP) mode.
 */
void handleSwitchToFtp() {
  if (isInMscMode) {
    if (enterFtpMode()) {
      DynamicJsonDocument jsonResponse(256);
      jsonResponse["status"] = "success";
      jsonResponse["message"] = "Switched to Application (FTP) mode.";
      String output;
      serializeJson(jsonResponse, output);
      server.send(200, "application/json", output);
    } else {
      DynamicJsonDocument jsonResponse(256);
      jsonResponse["status"] = "error";
      jsonResponse["message"] = "Failed to re-initialize SD card.";
      String output;
      serializeJson(jsonResponse, output);
      server.send(500, "application/json", output);
    }
  }
  else {
    DynamicJsonDocument jsonResponse(256);
    jsonResponse["status"] = "no_change";
    jsonResponse["message"] = "Already in Application (FTP) mode.";
    String output;
    serializeJson(jsonResponse, output);
    server.send(200, "application/json", output);
  }
}

/**
 * @brief Handles the POST request to restart the device.
 */
void handleRestart() {
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["message"] = "Restarting device...";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
  delay(100);
  ESP.restart();
}

/**
 * @brief Handles the POST request to turn the display on.
 */
void handleDisplayOn() {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  digitalWrite(TFT_LEDA, LOW);
  isDisplayOn = true;
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["message"] = "Display turned on.";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
#else
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "no_change";
  jsonResponse["message"] = "Display is disabled in firmware.";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
#endif
}

/**
 * @brief Handles the POST request to turn the display off.
 */
void handleDisplayOff() {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  digitalWrite(TFT_LEDA, HIGH);
  isDisplayOn = false;
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["message"] = "Display turned off.";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
#else
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "no_change";
  jsonResponse["message"] = "Display is disabled in firmware.";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
#endif
}

/**
 * @brief Handles the POST request to toggle the display.
 */
void handleDisplayToggle() {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  isDisplayOn = !isDisplayOn;
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  if (isDisplayOn) {
    digitalWrite(TFT_LEDA, LOW);
    jsonResponse["message"] = "Display toggled on.";
  } else {
    digitalWrite(TFT_LEDA, HIGH);
    jsonResponse["message"] = "Display toggled off.";
  }
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
#else
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "no_change";
  jsonResponse["message"] = "Display is disabled in firmware.";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
#endif
}

/**
 * @brief Handles the POST request to reset WiFi settings.
 */
void handleWifiReset() {
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["message"] = "Resetting WiFi and restarting...";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
  delay(200);
  resetWifiSettings();
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

    // --- Update storage info on the screen ---
    DeviceInfo info;
    getDeviceInfo(info);
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
    drawFtpModeScreen(info.ipAddress.c_str(), info.macAddress.c_str(), info.fileCount, info.totalSize / (1024 * 1024), info.freeSize / (1024.0 * 1024.0));
#endif
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
    publishMqttStatus();
#endif
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

// --- MQTT ---

/**
 * @brief Sets up the MQTT client and connection.
 */
void setupMqtt() {
  // Save the custom parameters to FS
  if (shouldSaveMqttConfig) {
    saveMqttConfig();
  }
  mqttClient.setServer(mqtt_host, String(mqtt_port).toInt());
  mqttClient.setCallback(callback);
  HWSerial.println("MQTT client setup complete.");
}

/**
 * @brief Publishes the current status to the MQTT state topic.
 */
void publishMqttStatus() {
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  if (!mqttClient.connected()) {
    return; // Don't publish if not connected
  }

  DeviceInfo info;
  getDeviceInfo(info);

  DynamicJsonDocument jsonResponse(1024);
  jsonResponse["mode"] = info.modeString;
  JsonObject display = jsonResponse.createNestedObject("display");
  display["status"] = info.displayStatus;
  display["orientation"] = info.displayOrientation;
  JsonObject sd_card = jsonResponse.createNestedObject("sd_card");
  sd_card["total_size"] = info.totalSize;
  sd_card["used_size"] = info.usedSize;
  sd_card["free_size"] = info.freeSize;
  sd_card["file_count"] = info.fileCount;

  String output;
  serializeJson(jsonResponse, output);

  const char* displayStatusMqtt = info.isDisplayOn ? "ON" : "OFF";

  mqttClient.publish(MQTT_STATE_TOPIC, output.c_str(), true); // Retain message
  mqttClient.publish(MQTT_DISPLAY_STATUS_TOPIC, displayStatusMqtt, true); // Retain message
  HWSerial.println("Published MQTT status.");
#endif
}

/**
 * @brief Handles incoming MQTT messages.
 */
void callback(char *topic, byte *payload, unsigned int length) {
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  HWSerial.print("Message arrived [");
  HWSerial.print(topic);
  HWSerial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  HWSerial.println(message);

  if (strcmp(topic, MQTT_DISPLAY_SET_TOPIC) == 0) {
    if (strcmp(message, "ON") == 0) {
      handleDisplayOn();
    } else if (strcmp(message, "OFF") == 0) {
      handleDisplayOff();
    }
  }
#endif
}

/**
 * @brief Reconnects to the MQTT broker.
 */
void reconnect() {
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  HWSerial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
    HWSerial.println("connected");
    // Subscribe
    mqttClient.subscribe(MQTT_DISPLAY_SET_TOPIC);
    // Publish initial status
    publishMqttStatus();
  } else {
    HWSerial.print("failed, rc=");
    HWSerial.print(mqttClient.state());
    HWSerial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
  }
#endif
}

// --- Display ---

/**
 * @brief Fetches the latest storage stats and redraws the MSC mode screen.
 */
void updateAndDrawMscScreen() {
  if (!card) return;

  DeviceInfo info;
  getDeviceInfo(info);

#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawUsbMscModeScreen(
    info.ipAddress.c_str(),
    info.macAddress.c_str(),
    info.fileCount,
    info.totalSize / (1024 * 1024),
    info.freeSize / (1024.0 * 1024.0)
  );
#endif
  HWSerial.println("MSC screen refreshed.");
}

/**
 * @brief Draws the top header bar.
 */
void drawHeader(const char* title, uint16_t bannerColor) {
  tft.fillRect(0, 0, tft.width(), 12, bannerColor);
  tft.setTextColor(CATPPUCCIN_CRUST);
  tft.setTextSize(1);
  tft.drawCentreString(title, tft.width() / 2, 2, 1); // x-center, y, font
  tft.setTextSize(1);
}

/**
 * @brief Draws the right column with storage statistics.
 */
void drawStorageInfo(int files, int totalSizeMB, float freeSizeMB) {
  uint8_t rotation = tft.getRotation();
  if (rotation == 1 || rotation == 3) { // Landscape
    drawStorageInfoLandscape(files, totalSizeMB, freeSizeMB);
  } else { // Portrait
    drawStorageInfoPortrait(files, totalSizeMB, freeSizeMB);
  }
}

/**
 * @brief Draws the storage statistics in landscape mode.
 */
void drawStorageInfoLandscape(int files, int totalSizeMB, float freeSizeMB) {
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
  tft.print("%)");
  y_pos += 12;

  // --- Draw capacity bar ---
  int bar_x = x_pos;
  int bar_height = 3;
  int bar_y = tft.height() - bar_height;
  int bar_width = TFT_HEIGHT - (2 * x_pos); // Bar width spans the screen with padding
  int filled_width = (bar_width * usedPercentage) / 100;

  // --- Draw the bar background (empty part) ---
  tft.drawRect(bar_x, bar_y, bar_width, bar_height, CATPPUCCIN_BASE);
  // --- Draw the filled part of the bar ---
  tft.fillRect(bar_x, bar_y, filled_width, bar_height, CATPPUCCIN_GREEN);
}

/**
 * @brief Draws the storage statistics in portrait mode.
 */
void drawStorageInfoPortrait(int files, int totalSizeMB, float freeSizeMB) {
  int y_pos = 90;
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
  tft.print("Files:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(files);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Size:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(totalSizeGB, 2);
  tft.print("GB");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Used:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(usedSizeGB, 2);
  tft.print("GB (");
  tft.print((int)usedPercentage);
  tft.print("%)");
  y_pos += 12;

  // --- Draw capacity bar ---
  // int bar_x = x_pos;
  int bar_x = 0;
  int bar_y = 12;
  int bar_width = x_pos-2;
  int bar_height = tft.height() - 12; // Bar width spans the screen with padding
  int filled_height = (bar_height * usedPercentage) / 100;

  // --- Draw the bar background (empty part) ---
  tft.drawRect(bar_x, bar_y, bar_width, bar_height, CATPPUCCIN_BASE);
  // --- Draw the filled part of the bar ---
  tft.fillRect(bar_x, y_pos - filled_height, bar_width, filled_height, CATPPUCCIN_GREEN);
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
  uint8_t rotation = tft.getRotation();

  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.setTextSize(1);
  if (rotation == 1 || rotation == 3) { // Landscape
    tft.drawCentreString("Resetting Wi-Fi...", x_pos, y_pos, 2);
  } else { // Portrait
    tft.drawCentreString("Resetting", x_pos, y_pos, 2);
    y_pos += 15;
    tft.drawCentreString("Wi-Fi...", x_pos, y_pos, 2);
  }
  y_pos += 25;

  tft.setTextSize(1);
  tft.setTextColor(CATPPUCCIN_LAVENDER);
  tft.drawCentreString("Restarting...", x_pos, y_pos, 1);
}

/**
 * @brief Displays the AP mode screen.
 */
void drawApModeScreen(const char* ap_ssid, const char* ap_ip) {
  uint8_t rotation = tft.getRotation();
  if (rotation == 1 || rotation == 3) { // Landscape
    drawApModeScreenLandscape(ap_ssid, ap_ip);
  } else { // Portrait
    drawApModeScreenPortrait(ap_ssid, ap_ip);
  }
}

/**
 * @brief Displays the AP mode screen in landscape mode.
 */
void drawApModeScreenLandscape(const char* ap_ssid, const char* ap_ip) {
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
 * @brief Displays the AP mode screen in portrait mode.
 */
void drawApModeScreenPortrait(const char* ap_ssid, const char* ap_ip) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi Setup", CATPPUCCIN_YELLOW);

  // --- Network Info ---
  int y_pos = 17;
  int x_pos = 5;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("AP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("AP IP:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW); // Use green for active mode
  tft.print(ap_ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("SSID:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW); // Use green for active mode
  tft.print(ap_ssid);
}

/**
 * @brief Displays the FTP mode screen.
 */
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  uint8_t rotation = tft.getRotation();
  if (rotation == 1 || rotation == 3) { // Landscape
    drawFtpModeScreenLandscape(ip, mac, files, totalSizeMB, freeSizeMB);
  } else { // Portrait
    drawFtpModeScreenPortrait(ip, mac, files, totalSizeMB, freeSizeMB);
  }
}

/**
 * @brief Displays the FTP mode screen in landscape mode.
 */
void drawFtpModeScreenLandscape(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
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
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(mac);

  // --- Right Column: Storage Info ---
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

/**
 * @brief Displays the FTP mode screen in portrait mode.
 */
void drawFtpModeScreenPortrait(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", CATPPUCCIN_GREEN);

  // --- Network Info ---
  int y_pos = 17;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("FTP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW);
  String ipStr = String(ip);
  ipStr.replace(".", "");
  tft.print(ipStr);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW);
  String macStr = String(mac);
  macStr.replace(":", "");
  tft.print(macStr);

  // --- Storage Info ---
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

/**
 * @brief Displays the USB MSC mode screen.
 */
void drawUsbMscModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  uint8_t rotation = tft.getRotation();
  if (rotation == 1 || rotation == 3) { // Landscape
    drawUsbMscModeScreenLandscape(ip, mac, files, totalSizeMB, freeSizeMB);
  } else { // Portrait
    drawUsbMscModeScreenPortrait(ip, mac, files, totalSizeMB, freeSizeMB);
  }
}

/**
 * @brief Displays the USB MSC mode screen in landscape mode.
 */
void drawUsbMscModeScreenLandscape(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
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

/**
 * @brief Displays the USB MSC mode screen in portrait mode.
 */
void drawUsbMscModeScreenPortrait(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", CATPPUCCIN_MAUVE);

  // --- Network Info ---
  int y_pos = 17;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_GREEN);
  tft.print("USB MSC");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW);
  String ipStr = String(ip);
  ipStr.replace(".", "");
  tft.print(ipStr);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW);
  String macStr = String(mac);
  macStr.replace(":", "");
  tft.print(macStr);

  // --- Storage Info ---
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}
