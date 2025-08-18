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
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#undef FF_MAX_LFN
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
#include <OneButton.h> // https://github.com/ck Conrad/esp32-onebutton
#include <FastLED.h>   // https://github.com/FastLED/FastLED
#include <PubSubClient.h>
#include <ArduinoJson.h>     // https://github.com/bblanchon/ArduinoJson
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include <Preferences.h> // https://github.com/vshymanskyy/Preferences

// --- Data Structure for Device Information ---
struct DeviceInfo {
  // Mode and Display
  const char* modeString;
  bool isInMscMode;
  const char* displayStatus;
  bool isDisplayOn;
  int displayOrientation;

  // Network
  char ipAddress[16];
  char macAddress[18];

  // SD Card
  int fileCount;
  uint64_t totalSize;
  uint64_t usedSize;
  uint64_t freeSize;

  // MQTT
  int mqttState;
  bool mqttConnected;
  bool isMqttEnabled;

  // LED
  const char* ledColor;
  int ledBrightness;
};

// --- FTP Configuration ---
struct FtpConfig {
  char user[32];
  char pass[32];
};

// --- WebServer Configuration ---
struct WebServerConfig {
  char user[32];
  char pass[32];
};

// --- MQTT Configuration ---
struct MqttConfig {
  char host[64];
  char port[6];
  char user[32];
  char pass[32];
  char client_id[32];
};

// --- Function to populate device info ---
void getDeviceInfo(DeviceInfo& info);

// --- Mode Descriptions ---
namespace Mode {
  const char* MSC = "USB MSC";
  const char* FTP = "Application (FTP Server)";
}
 
// --- Create objects ---
WebServer server(80);
OneButton button(BTN_PIN, true); // true for active low
FtpServer ftpServer;
CRGB leds[NUM_LEDS];
USBMSC MSC;
USBCDC USBSerial;
File uploadFile;
TFT_eSPI tft = TFT_eSPI();
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define HWSerial    Serial0
#define MOUNT_POINT "/sdcard"
 sdmmc_card_t *card;

bool shouldSaveConfig = false;

int ledBrightness;

// --- FTP & MQTT Credentials ---
FtpConfig ftpConfig;
WebServerConfig webServerConfig;
MqttConfig mqttConfig;

// --- Mode Switching Flags ---
volatile bool pendingModeSwitch = false;
volatile bool targetMscMode = false;

// --- A flag to track the current mode ---
bool isInMscMode = true;
bool isDisplayOn = true; // A flag to track the display status
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
bool isMqttEnabled = true; // A flag to track the MQTT status
#else
bool isMqttEnabled = false;
#endif

// --- MSC screen refresh tracking ---
volatile bool msc_disk_dirty = false;
volatile unsigned long last_msc_write_time = 0;
const unsigned long MSC_REFRESH_DEBOUNCE_MS = 2000; // 2 seconds

// --- MQTT Topics ---
namespace MqttTopics {
  const char* STATE = "frame-fi/state";
  const char* DISPLAY_STATUS = "frame-fi/display/status";
  const char* DISPLAY_SET = "frame-fi/display/set";
}

// --- Timers ---
unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 300000; // 5 minutes
// Timer variables for non-blocking reconnection
unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 5000; // Interval to wait between retries (5 seconds)

// --- Function prototypes ---
void initializeConfigs();
void setupLed();
void setupButton();
void setupDisplay();
void displayBootScreen();
void setupFilesystems();
void setupWebServer();
void startInitialMode();
void handleButton();
void handleMqtt();
void handleFtp();
void handleMsc();
void connectToWiFi();
void setupApiRoutes();
void setupSerial();
void enterMscMode();
bool enterFtpMode();
void handleStatus();
void handleRestart();
void handleDisplayAction(const char* action);
void setDisplayState(bool on);
void handleWifiReset();
void handleMqttAction(const char* action);
void manageMqttState(const char* state);
void sendJsonResponse(const char* status, const char* message);
void handleLedStatus();
void handleLedAction(const char* action);
void handleLedBrightness();
void setLedState(const char* state);
void handleUpload();
void handleUploadData();
void handleGetMode();
void handleDisplayStatus();
void handleMqttStatus();
void handleLedBrightnessGet();
void toggleMode();
void resetWifiSettings();
void mscInit();
void sdInit();
void handleSwitchToMsc();
void handleSwitchToFtp();
void ftpTransferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize);
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);
static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void drawHeader(const char* title, uint16_t bannerColor);
void drawStorageInfo(int files, int totalSizeMB, float freeSizeMB);
void drawInfoScreen(const char* title, const char* message, const char* version, uint16_t headerColor);
void drawApModeScreen(const char* ap_ssid, const char* ap_ip);
void drawModeScreen(const char* mode, uint16_t headerColor, const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB, bool mqttConnected);
void drawUsbMscModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB, bool mqttConnected);
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB, bool mqttConnected);
void drawMqttStatusIcon(bool mqttConnected, int x, int y);
int countFiles(File dir);
int countFilesInPath(const char *path);
void updateAndDrawMscScreen();
void updateDisplayAndMqtt();
void setupMqtt();
void publishMqttStatus();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void saveConfigCallback();
void saveConfig();
void loadConfig();
const char* getLedColorString(CRGB color);

// --- Main Logic ---

/**
 * @brief Initializes the device and all its components.
 */
void setup() {
  initializeConfigs();
  setupSerial();
  setupLed();
  setupButton();
  setupDisplay();
  displayBootScreen();
  loadConfig();

  // --- Connect to WiFi and configure services ---
  connectToWiFi();

  if (shouldSaveConfig) {
    saveConfig();
  }

  setupWebServer();
  
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  setupMqtt();
  reconnect(); // Attempt initial MQTT connection
#endif

  // --- Start in initial mode ---
  startInitialMode();
}

/**
 * @brief Initializes FTP and MQTT configurations with default values.
 */
void initializeConfigs() {
  // --- Initialize FTP and MQTT configurations with default values ---
#if defined(FTP_USER)
  strcpy(ftpConfig.user, FTP_USER);
#else
  strcpy(ftpConfig.user, "user");
#endif
#if defined(FTP_PASSWORD)
  strcpy(ftpConfig.pass, FTP_PASSWORD);
#else
  strcpy(ftpConfig.pass, "password");
#endif

#if defined(WEB_SERVER_USER)
  strcpy(webServerConfig.user, WEB_SERVER_USER);
#else
  strcpy(webServerConfig.user, "");
#endif
#if defined(WEB_SERVER_PASSWORD)
  strcpy(webServerConfig.pass, WEB_SERVER_PASSWORD);
#else
  strcpy(webServerConfig.pass, "");
#endif

#if defined(MQTT_HOST)
  strcpy(mqttConfig.host, MQTT_HOST);
#else
  strcpy(mqttConfig.host, "192.168.1.100");
#endif
#if defined(MQTT_PORT)
  char buffer[6];
  sprintf(buffer, "%d", MQTT_PORT);
  strcpy(mqttConfig.port, buffer);
#else
  strcpy(mqttConfig.port, "1883");
#endif
#if defined(MQTT_USER)
  strcpy(mqttConfig.user, MQTT_USER);
#else
  strcpy(mqttConfig.user, "");
#endif
#if defined(MQTT_PASSWORD)
  strcpy(mqttConfig.pass, MQTT_PASSWORD);
#else
  strcpy(mqttConfig.pass, "");
#endif
#if defined(MQTT_CLIENT_ID)
  strcpy(mqttConfig.client_id, MQTT_CLIENT_ID);
#else
  strcpy(mqttConfig.client_id, "FrameFi");
#endif
}

/**
 * @brief Initializes the LED.
 */
void setupLed() {
  // --- Initialize the LED pin as an output ---
  FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(leds, NUM_LEDS);
#if defined(LED_BRIGHTNESS)
  ledBrightness = LED_BRIGHTNESS;
#else
  ledBrightness = 13;  // 5% of 255
#endif
  FastLED.setBrightness(ledBrightness);
  // --- Turn the LED on ---
  leds[0] = CRGB::Yellow;
  FastLED.show();
}

/**
 * @brief Initializes the button.
 */
void setupButton() {
  // --- Initialize Button ---
  button.attachClick(toggleMode);
  button.setPressMs(3000); // 3 seconds
  button.attachLongPressStart(resetWifiSettings);
}

/**
 * @brief Initializes the TFT display.
 */
void setupDisplay() {
  // --- Initialize TFT Display ---
  pinMode(TFT_LEDA, OUTPUT);
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  tft.init();
  tft.setRotation(DISPLAY_ORIENTATION); // Adjust rotation as needed
  tft.fillScreen(CATPPUCCIN_BASE);
  setDisplayState(isDisplayOn);
#else
  digitalWrite(TFT_LEDA, HIGH);
#endif
}

/**
 * @brief Displays the boot screen.
 */
void displayBootScreen() {
  // --- Show boot screen ---
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawInfoScreen("FrameFi", "Booting...", APP_VERSION, CATPPUCCIN_BLUE);
  delay(2000); // Keep boot screen visible for 2 seconds
#endif
}

/**
 * @brief Sets up and starts the web server.
 */
void setupWebServer() {
  // --- Setup and start Web Server ---
  setupApiRoutes();
  server.begin();
  HWSerial.println("HTTP server started.");
}

/**
 * @brief Starts the device in the initial mode (MSC).
 */
void startInitialMode() {
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
void loop() {
  server.handleClient();
  handleButton();

  // --- Handle pending mode switch from API calls ---
  if (pendingModeSwitch) {
    pendingModeSwitch = false; // Reset the flag immediately
    if (targetMscMode) {
      enterMscMode();
    } else {
      enterFtpMode();
    }
  }

  handleMqtt();
  handleFtp();
  handleMsc();
}

/**
 * @brief Handles button events.
 */
void handleButton() {
  button.tick();
}



/**
 * @brief Handles MQTT connection and message publishing.
 */
void handleMqtt() {
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  if (!isMqttEnabled) {
    if (mqttClient.connected()) {
      mqttClient.disconnect();
    }
    return;
  }
  if (!mqttClient.connected()) {
    long now = millis();
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
}

/**
 * @brief Handles FTP server requests.
 */
void handleFtp() {
  if (!isInMscMode) {
    ftpServer.handleFTP();
  }
}

/**
 * @brief Handles MSC screen refresh logic.
 */
void handleMsc() {
  if (isInMscMode && msc_disk_dirty && (millis() - last_msc_write_time > MSC_REFRESH_DEBOUNCE_MS)) {
    msc_disk_dirty = false; // Reset flag
    updateAndDrawMscScreen();
  }
}

/**
 * Loads the configuration from a file.
 */
void loadConfig() {
  Preferences prefs;
  prefs.begin("frame-fi", true); // Read-only

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  isMqttEnabled = prefs.getBool("mqtt_enabled", isMqttEnabled);
  String mqttHost = prefs.getString("mqtt_host", mqttConfig.host);
  strcpy(mqttConfig.host, mqttHost.c_str());
  String mqttPort = prefs.getString("mqtt_port", mqttConfig.port);
  strcpy(mqttConfig.port, mqttPort.c_str());
  String mqttUser = prefs.getString("mqtt_user", mqttConfig.user);
  strcpy(mqttConfig.user, mqttUser.c_str());
  String mqttPass = prefs.getString("mqtt_pass", mqttConfig.pass);
  strcpy(mqttConfig.pass, mqttPass.c_str());
  String mqttClientId = prefs.getString("mqtt_client_id", mqttConfig.client_id);
  strcpy(mqttConfig.client_id, mqttClientId.c_str());
#endif

  String ftpUser = prefs.getString("ftp_user", ftpConfig.user);
  strcpy(ftpConfig.user, ftpUser.c_str());
  String ftpPass = prefs.getString("ftp_pass", ftpConfig.pass);
  strcpy(ftpConfig.pass, ftpPass.c_str());

  String webUser = prefs.getString("web_user", webServerConfig.user);
  strcpy(webServerConfig.user, webUser.c_str());
  String webPass = prefs.getString("web_pass", webServerConfig.pass);
  strcpy(webServerConfig.pass, webPass.c_str());

  ledBrightness = prefs.getInt("led_brightness", ledBrightness);

  prefs.end();
  HWSerial.println("Configuration loaded from prefs.");
}

/**
 * Saves the configuration to a file.
 */
void saveConfig() {
  Preferences prefs;
  prefs.begin("frame-fi", false); // Read-write

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  prefs.putBool("mqtt_enabled", isMqttEnabled);
  prefs.putString("mqtt_host", mqttConfig.host);
  prefs.putString("mqtt_port", mqttConfig.port);
  prefs.putString("mqtt_user", mqttConfig.user);
  prefs.putString("mqtt_pass", mqttConfig.pass);
  prefs.putString("mqtt_client_id", mqttConfig.client_id);
#endif

  prefs.putString("ftp_user", ftpConfig.user);
  prefs.putString("ftp_pass", ftpConfig.pass);
  prefs.putString("web_user", webServerConfig.user);
  prefs.putString("web_pass", webServerConfig.pass);

  prefs.putInt("led_brightness", ledBrightness);

  prefs.end();
  HWSerial.println("Configuration saved to prefs.");
}

void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

/**
 * @brief Returns a string representation of the LED color.
 */
const char* getLedColorString(CRGB color) {
  if (color == CRGB::Black) return "off";
  if (color == CRGB::Yellow) return "yellow";
  if (color == CRGB::Blue) return "blue";
  if (color == CRGB::Green) return "green";
  if (color == CRGB::Orange) return "orange";
  return "unknown";
}

// --- Get Device Info ---
void getDeviceInfo(DeviceInfo& info) {
  info.isInMscMode = ::isInMscMode; // Use global isInMscMode
  info.isDisplayOn = ::isDisplayOn; // Use global isDisplayOn
  info.modeString = info.isInMscMode ? Mode::MSC : Mode::FTP;
  info.displayStatus = info.isDisplayOn ? "on" : "off";
  info.displayOrientation = tft.getRotation();
  strncpy(info.ipAddress, WiFi.localIP().toString().c_str(), sizeof(info.ipAddress));
  info.ipAddress[sizeof(info.ipAddress) - 1] = '\0';
  strncpy(info.macAddress, WiFi.macAddress().c_str(), sizeof(info.macAddress));
  info.macAddress[sizeof(info.macAddress) - 1] = '\0';
  info.mqttState = mqttClient.state();
  info.mqttConnected = mqttClient.connected();
  info.isMqttEnabled = ::isMqttEnabled;
  info.ledColor = getLedColorString(leds[0]);
  info.ledBrightness = ::ledBrightness;

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

  // Set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
    
  wm.setParamsPage(true);

  const char* headhtml = "<script>function f2(id){var x = document.getElementById(id);x.type ==='password'?x.type='text':x.type='password';}</script>";
  wm.setCustomHeadElement(headhtml);
    
  WiFiManagerParameter custom_ftp_user("ftp_user", "FTP User", ftpConfig.user, 32);
  WiFiManagerParameter custom_ftp_pass("ftp_pass", "FTP Password", ftpConfig.pass, 32, "type=\'password\'");
  const char _customHtml_checkbox[] = "type=\'checkbox\' onclick=\"f2(\'ftp_pass\')\"";
  WiFiManagerParameter custom_checkbox("showpass", "Show Password", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  
  wm.addParameter(&custom_ftp_user);
  wm.addParameter(&custom_ftp_pass);
  wm.addParameter(&custom_checkbox);

  WiFiManagerParameter custom_web_user("web_user", "Web User", webServerConfig.user, 32);
  WiFiManagerParameter custom_web_pass("web_pass", "Web Password", webServerConfig.pass, 32, "type='password'");
  const char _customHtml_checkbox3[] = "type='checkbox' onclick=\"f2('web_pass')\"";
  WiFiManagerParameter custom_checkbox3("showpass3", "Show Password", "T", 2, _customHtml_checkbox3, WFM_LABEL_AFTER);

  const char *bufferStr = "<br/><hr></br>";
  WiFiManagerParameter custom_web_sep(bufferStr);

  wm.addParameter(&custom_web_sep);
  wm.addParameter(&custom_web_user);
  wm.addParameter(&custom_web_pass);
  wm.addParameter(&custom_checkbox3);

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1

  const char *bufferStr2 = "<br/><hr></br>";
    
  // Add custom parameters for MQTT
  WiFiManagerParameter custom_mqtt_client_id("mqtt_client_id", "MQTT Client ID", mqttConfig.client_id, 32);
  WiFiManagerParameter custom_mqtt_host("mqtt_host", "MQTT Host", mqttConfig.host, 64);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqttConfig.port, 6);
  WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", mqttConfig.user, 32);
  WiFiManagerParameter custom_mqtt_pass("mqtt_pass", "MQTT Password", mqttConfig.pass, 32, "type='password'");
  const char _customHtml_checkbox2[] = "type='checkbox' onclick=\"f2('mqtt_pass')\""; 
  WiFiManagerParameter custom_checkbox2("showpass2", "Show Password", "T", 2, _customHtml_checkbox2, WFM_LABEL_AFTER);

  WiFiManagerParameter custom_mqtt_sep(bufferStr);

  wm.addParameter(&custom_mqtt_sep);
  wm.addParameter(&custom_mqtt_client_id);  
  wm.addParameter(&custom_mqtt_host);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  wm.addParameter(&custom_checkbox2);
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
  strcpy(ftpConfig.user, custom_ftp_user.getValue());
  strcpy(ftpConfig.pass, custom_ftp_pass.getValue());
  strcpy(webServerConfig.user, custom_web_user.getValue());
  strcpy(webServerConfig.pass, custom_web_pass.getValue());

#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  strcpy(mqttConfig.host, custom_mqtt_host.getValue());
  strcpy(mqttConfig.port, custom_mqtt_port.getValue());
  strcpy(mqttConfig.user, custom_mqtt_user.getValue());
  strcpy(mqttConfig.pass, custom_mqtt_pass.getValue());
  strcpy(mqttConfig.client_id, custom_mqtt_client_id.getValue());
#endif
}

// --- Button Actions ---

/**
 * @brief Resets WiFi settings if the button is held for 3 seconds.
 */
void resetWifiSettings() {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawInfoScreen("FrameFi", "Resetting Wi-Fi...", "Restarting...", CATPPUCCIN_RED);
#endif
  HWSerial.println("Button held for 3 seconds. Resetting WiFi settings...");
  WiFiManager wm;
  wm.resetSettings();
  
  // --- Preferences ---
  Preferences prefs;
  prefs.begin("frame-fi", false); // Read-write
  prefs.clear();
  
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
  server.on("/mode/msc", HTTP_GET, handleGetMode);
  server.on("/mode/ftp", HTTP_POST, handleSwitchToFtp);
  server.on("/mode/ftp", HTTP_GET, handleGetMode);
  server.on("/device/restart", HTTP_POST, handleRestart);
  server.on("/display/toggle", HTTP_POST, [](){ handleDisplayAction("toggle"); });
  server.on("/display/on", HTTP_POST, [](){ handleDisplayAction("on"); });
  server.on("/display/off", HTTP_POST, [](){ handleDisplayAction("off"); });
  server.on("/display/status", HTTP_GET, handleDisplayStatus);
  server.on("/wifi/reset", HTTP_POST, handleWifiReset);
  server.on("/mqtt/enable", HTTP_POST, [](){ handleMqttAction("enable"); });
  server.on("/mqtt/disable", HTTP_POST, [](){ handleMqttAction("disable"); });
  server.on("/mqtt/toggle", HTTP_POST, [](){ handleMqttAction("toggle"); });
  server.on("/mqtt/status", HTTP_GET, handleMqttStatus);
  server.on("/led/status", HTTP_GET, handleLedStatus);
  server.on("/led/toggle", HTTP_POST, [](){ handleLedAction("toggle"); });
  server.on("/led/on", HTTP_POST, [](){ handleLedAction("on"); });
  server.on("/led/off", HTTP_POST, [](){ handleLedAction("off"); });
  server.on("/led/brightness", HTTP_GET, handleLedBrightnessGet);
  server.on("/led/brightness", HTTP_POST, handleLedBrightness);
  server.on("/upload", HTTP_POST, handleUpload, handleUploadData);
}

void updateDisplayAndMqtt() {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  DeviceInfo info;
  getDeviceInfo(info);
  if (isInMscMode) {
    drawUsbMscModeScreen(info.ipAddress, info.macAddress, info.fileCount, info.totalSize / (1024 * 1024), info.freeSize / (1024.0 * 1024.0), info.mqttConnected);
  } else {
    drawFtpModeScreen(info.ipAddress, info.macAddress, info.fileCount, info.totalSize / (1024 * 1024), info.freeSize / (1024.0 * 1024.0), info.mqttConnected);
  }
#endif
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
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

    // --- Update display and MQTT ---
    updateDisplayAndMqtt();
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

#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  drawInfoScreen("FrameFi", "Entering FTP Mode...", "", CATPPUCCIN_PEACH);
#endif

  // --- Turn the LED on ---
  leds[0] = CRGB::Purple;
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
  ftpServer.begin(ftpConfig.user, ftpConfig.pass);
  ftpServer.setTransferCallback(ftpTransferCallback);
  HWSerial.println("FTP Server started.");

  HWSerial.println("\n✅ Application mode active.");
  isInMscMode = false;

  // --- Update display and MQTT ---
  updateDisplayAndMqtt();
  return true;
}



/**
 * @brief Handles requests to the root URL ("/"). Sends a JSON status object.
 */
void handleStatus() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  DeviceInfo info;
  getDeviceInfo(info);

  const int JSON_STATUS_SIZE = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(3);
  DynamicJsonDocument jsonResponse(JSON_STATUS_SIZE);
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
  mqtt["enabled"] = info.isMqttEnabled;
  mqtt["state"] = info.mqttState;
  mqtt["connected"] = info.mqttConnected;
  JsonObject led = jsonResponse.createNestedObject("led");
  led["color"] = info.ledColor;
  led["state"] = (leds[0] == CRGB::Black) ? "off" : "on";
  led["brightness"] = info.ledBrightness;

  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Handles the POST request to switch to MSC mode.
 */
void handleSwitchToMsc() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  if (isInMscMode) {
    DynamicJsonDocument jsonResponse(256);
    jsonResponse["status"] = "no_change";
    jsonResponse["message"] = "Already in MSC mode.";
    String output;
    serializeJson(jsonResponse, output);
    server.send(200, "application/json", output);
  } else {
    DynamicJsonDocument jsonResponse(256);
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Attempting to switch to MSC mode.";
    String output;
    serializeJson(jsonResponse, output);
    server.send(200, "application/json", output);
    pendingModeSwitch = true;
    targetMscMode = true;
  }  
}
 
/**
 * @brief Handles the POST request to switch back to Application (FTP) mode.
 */
void handleSwitchToFtp() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  if (isInMscMode) {
    DynamicJsonDocument jsonResponse(256);
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Attempting to switch to Application (FTP) mode.";
    String output;
    serializeJson(jsonResponse, output);
    server.send(200, "application/json", output);
    pendingModeSwitch = true;
    targetMscMode = false;
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
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
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
 * @brief Sets the display state (on/off).
 */
void setDisplayState(bool on) {
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  digitalWrite(TFT_LEDA, on ? LOW : HIGH);
  isDisplayOn = on;
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  publishMqttStatus();
#endif
#endif
}

/**
 * @brief Handles the POST request for display actions (on/off/toggle).
 */
void handleDisplayAction(const char* action) {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
  String message;
  if (strcmp(action, "on") == 0) {
    setDisplayState(true);
    message = "Display turned on.";
  } else if (strcmp(action, "off") == 0) {
    setDisplayState(false);
    message = "Display turned off.";
  } else if (strcmp(action, "toggle") == 0) {
    setDisplayState(!isDisplayOn);
    message = isDisplayOn ? "Display toggled on." : "Display toggled off.";
  }
  sendJsonResponse("success", message.c_str());
#else
  sendJsonResponse("no_change", "Display is disabled in firmware.");
#endif
}

/**
 * @brief Handles the POST request to reset WiFi settings.
 */
void handleWifiReset() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  sendJsonResponse("success", "Resetting WiFi and restarting...");
  delay(200);
  resetWifiSettings();
}

/**
 * @brief Sends a standardized JSON response.
 */
void sendJsonResponse(const char* status, const char* message) {
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = status;
  jsonResponse["message"] = message;
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Manages the MQTT state (enable/disable/toggle).
 */
void manageMqttState(const char* state) {
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  if (strcmp(state, "enable") == 0) {
    isMqttEnabled = true;
    reconnect();
  } else if (strcmp(state, "disable") == 0) {
    isMqttEnabled = false;
    mqttClient.disconnect();
  } else if (strcmp(state, "toggle") == 0) {
    isMqttEnabled = !isMqttEnabled;
    if (isMqttEnabled) {
      reconnect();
    } else {
      mqttClient.disconnect();
    }
  }
  saveConfig();
  updateDisplayAndMqtt();
#endif
}

/**
 * @brief Handles the POST request for MQTT actions (enable/disable/toggle).
 */
void handleMqttAction(const char* action) {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  manageMqttState(action);
  String message = "MQTT " + String(action) + "d.";
  sendJsonResponse("success", message.c_str());
}

/**
 * @brief Sets the LED state (on/off/toggle).
 */
void setLedState(const char* state) {
  if (strcmp(state, "on") == 0) {
    if (isInMscMode) {
      leds[0] = CRGB::Green;
    } else {
      leds[0] = CRGB::Purple;
    }
  } else if (strcmp(state, "off") == 0) {
    leds[0] = CRGB::Black;
  } else if (strcmp(state, "toggle") == 0) {
    if (leds[0] == CRGB::Black) {
      if (isInMscMode) {
        leds[0] = CRGB::Green;
      } else {
        leds[0] = CRGB::Purple;
      }
    } else {
      leds[0] = CRGB::Black;
    }
  }
  FastLED.show();
}

/**
 * @brief Handles the POST request for LED actions (on/off/toggle).
 */
void handleLedAction(const char* action) {
    if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
        return server.requestAuthentication();
    }
    setLedState(action);
    String message = "LED turned " + String(action) + ".";
    if (strcmp(action, "toggle") == 0) {
        message = "LED toggled.";
    }
    sendJsonResponse("success", message.c_str());
}

/**
 * @brief Handles the GET request to return the LED color and state.
 */
void handleLedStatus() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["color"] = getLedColorString(leds[0]);
  jsonResponse["state"] = (leds[0] == CRGB::Black) ? "off" : "on";
  jsonResponse["brightness"] = ledBrightness;
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Handles the POST request to set the LED brightness.
 */
void handleLedBrightness() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }

  String brightnessStr = server.arg("plain");
  int newBrightness = brightnessStr.toInt();

  // Check for conversion errors and valid range
  if ((newBrightness == 0 && brightnessStr != "0") || newBrightness < 0 || newBrightness > 255) {
    sendJsonResponse("error", "Invalid brightness value. Body must be a plain text integer between 0 and 255.");
  } else {
    ledBrightness = newBrightness;
    FastLED.setBrightness(ledBrightness);
    FastLED.show();
    saveConfig();
    String message = "LED brightness set to " + String(ledBrightness) + ".";
    sendJsonResponse("success", message.c_str());
  }
}

/**
 * @brief Handles file uploads.
 */
void handleUpload() {
  if (uploadFile) {
    uploadFile.close();
  }
  sendJsonResponse("success", "File uploaded successfully.");
}

void handleUploadData() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
      return server.requestAuthentication();
    }
    if (isInMscMode) {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cannot upload in MSC mode.\"}");
      return;
    }
    String path = "/";
    if (upload.filename.startsWith("/")) {
      path += upload.filename;
    } else {
      path += upload.filename;
    }
    uploadFile = SD_MMC.open(path, FILE_WRITE);
    if (!uploadFile) {
      server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to open file for writing.\"}");
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
      yield();
    }
  }
}

/**
 * @brief Handles the GET request to return the current mode (MSC or FTP).
 */
void handleGetMode() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["mode"] = isInMscMode ? "USB MSC" : "Application (FTP Server)";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Handles the GET request to return the display status.
 */
void handleDisplayStatus() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["display_status"] = isDisplayOn ? "on" : "off";
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Handles the GET request to return the MQTT status.
 */
void handleMqttStatus() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["mqtt_enabled"] = isMqttEnabled;
  jsonResponse["mqtt_connected"] = mqttClient.connected();
  jsonResponse["mqtt_state"] = mqttClient.state();
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
}

/**
 * @brief Handles the GET request to return the LED brightness.
 */
void handleLedBrightnessGet() {
  if (strlen(webServerConfig.user) > 0 && !server.authenticate(webServerConfig.user, webServerConfig.pass)) {
    return server.requestAuthentication();
  }
  DynamicJsonDocument jsonResponse(256);
  jsonResponse["status"] = "success";
  jsonResponse["brightness"] = ledBrightness;
  String output;
  serializeJson(jsonResponse, output);
  server.send(200, "application/json", output);
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
    leds[0] = CRGB::Purple;
    FastLED.show();
  } else if (ftpOperation == FTP_UPLOAD_STOP || ftpOperation == FTP_DOWNLOAD_STOP || ftpOperation == FTP_TRANSFER_ERROR) {
    // --- Ensure LED is solid orange after any transfer completion or error ---
    leds[0] = CRGB::Purple;
    FastLED.show();
    
    // --- Update storage info on the screen ---
    DeviceInfo info;
    getDeviceInfo(info);
#if defined(LCD_ENABLED) && LCD_ENABLED == 1
    drawFtpModeScreen(info.ipAddress, info.macAddress, info.fileCount, info.totalSize / (1024 * 1024), info.freeSize / (1024.0 * 1024.0), info.mqttConnected);
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
  mqttClient.setServer(mqttConfig.host, String(mqttConfig.port).toInt());
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

  const int JSON_STATUS_SIZE = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(2);
  DynamicJsonDocument jsonResponse(JSON_STATUS_SIZE);
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

  mqttClient.publish(MqttTopics::STATE, output.c_str(), true); // Retain message
  mqttClient.publish(MqttTopics::DISPLAY_STATUS, displayStatusMqtt, true); // Retain message
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

  if (strcmp(topic, MqttTopics::DISPLAY_SET) == 0) {
    if (strcmp(message, "ON") == 0) {
      setDisplayState(true);
    } else if (strcmp(message, "OFF") == 0) {
      setDisplayState(false);
    }
  }
#endif
}

/**
 * @brief Reconnects to the MQTT broker.
 */
void reconnect() {
#if defined(MQTT_ENABLED) && MQTT_ENABLED == 1
  if (!isMqttEnabled) return;
  HWSerial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (mqttClient.connect(mqttConfig.client_id, mqttConfig.user, mqttConfig.pass)) {
    HWSerial.println("connected");
    // Subscribe
    mqttClient.subscribe(MqttTopics::DISPLAY_SET);
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
    info.ipAddress,
    info.macAddress,
    info.fileCount,
    info.totalSize / (1024 * 1024),
    info.freeSize / (1024.0 * 1024.0),
    info.mqttConnected
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
 * @brief Draws the storage statistics, adapting to the current orientation.
 */
void drawStorageInfo(int files, int totalSizeMB, float freeSizeMB) {
  uint8_t rotation = tft.getRotation();
  bool isLandscape = (rotation == 1 || rotation == 3);

  // --- Calculate usage ---
  float usedSizeMB = totalSizeMB - freeSizeMB;
  float usedPercentage = (totalSizeMB > 0) ? (usedSizeMB / totalSizeMB) * 100 : 0;
  float totalSizeGB = totalSizeMB / 1024.0;
  float usedSizeGB = usedSizeMB / 1024.0;

  if (isLandscape) {
    int y_pos = 53;
    int x_pos = 5;

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
    tft.print(" %)");
    y_pos += 12;

    // --- Draw capacity bar ---
    int bar_x = x_pos;
    int bar_height = 3;
    int bar_y = tft.height() - bar_height;
    int bar_width = TFT_HEIGHT - (2 * x_pos); // Bar width spans the screen with padding
    int filled_width = (bar_width * usedPercentage) / 100;

    tft.drawRect(bar_x, bar_y, bar_width, bar_height, CATPPUCCIN_BASE);
    tft.fillRect(bar_x, bar_y, filled_width, bar_height, CATPPUCCIN_GREEN);
  } else { // Portrait
    int y_pos = 90;
    int x_pos = 5;

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
    tft.print(" %)");
    y_pos += 12;

    // --- Draw capacity bar ---
    int bar_x = 0;
    int bar_y = 12;
    int bar_width = x_pos - 2;
    int bar_height = tft.height() - 12;
    int filled_height = (bar_height * usedPercentage) / 100;

    tft.drawRect(bar_x, bar_y, bar_width, bar_height, CATPPUCCIN_BASE);
    tft.fillRect(bar_x, y_pos - filled_height, bar_width, filled_height, CATPPUCCIN_GREEN);
  }
}

/**
 * @brief Displays a generic information screen.
 */
void drawInfoScreen(const char* title, const char* message, const char* version, uint16_t headerColor) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader(title, headerColor);

  int y_pos = 30;
  int x_pos = tft.width() / 2; // Center horizontally

  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.setTextSize(1);
  tft.drawCentreString(message, x_pos, y_pos, 2);
  y_pos += 25;

  tft.setTextSize(1);
  tft.setTextColor(CATPPUCCIN_LAVENDER);
  tft.drawCentreString(version, x_pos, y_pos, 1);
}

/**
 * @brief Displays the AP mode screen, adapting to the current orientation.
 */
void drawApModeScreen(const char* ap_ssid, const char* ap_ip) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi Setup", CATPPUCCIN_YELLOW);

  uint8_t rotation = tft.getRotation();
  bool isLandscape = (rotation == 1 || rotation == 3);

  int y_pos = 17;
  int x_pos = 5;

  // --- Draw Network Info ---
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  if (isLandscape) {
    tft.print("Mode:  ");
    tft.setTextColor(CATPPUCCIN_GREEN);
    tft.print("AP");
    y_pos += 12;

    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_MAUVE);
    tft.print("AP IP: ");
    tft.setTextColor(CATPPUCCIN_YELLOW);
    tft.print(ap_ip);
    y_pos += 12;

    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_MAUVE);
    tft.print("SSID:  ");
    if (tft.textWidth(ap_ssid) > tft.width() - tft.getCursorX()) {
      y_pos += 12;
      tft.setCursor(x_pos, y_pos);
    }
    tft.setTextColor(CATPPUCCIN_YELLOW);
    tft.print(ap_ssid);
  } else { // Portrait
    tft.print("Mode:");
    y_pos += 12;
    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_GREEN);
    tft.print("AP");
    y_pos += 12;

    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_MAUVE);
    tft.print("AP IP:");
    y_pos += 12;
    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_YELLOW);
    tft.print(ap_ip);
    y_pos += 12;

    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_MAUVE);
    tft.print("SSID:");
    y_pos += 12;
    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_YELLOW);
    tft.print(ap_ssid);
  }
}

/**
 * @brief Draws the MQTT connection status icon.
 */
void drawMqttStatusIcon(bool mqttConnected, int x, int y) {
  uint16_t color = mqttConnected ? CATPPUCCIN_GREEN : CATPPUCCIN_RED;
  tft.fillCircle(x, y, 3, color); // Draw a small circle
}

/**
 * @brief Displays the main screen for a given mode (FTP or MSC).
 */
void drawModeScreen(const char* mode, uint16_t headerColor, const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB, bool mqttConnected) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi", headerColor);

  uint8_t rotation = tft.getRotation();
  bool isLandscape = (rotation == 1 || rotation == 3);

  int y_pos = 17;
  int x_pos = 5;

  // --- Draw Network Info ---
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  if (isLandscape) {
    tft.print("Mode:  ");
    tft.setTextColor(CATPPUCCIN_GREEN);
    tft.print(mode);
    // --- Draw MQTT status icon ---
    if (isMqttEnabled) {
      drawMqttStatusIcon(mqttConnected, tft.getCursorX() + 8, y_pos + 3);
    }
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
  } else { // Portrait
    tft.print("Mode:");
    y_pos += 12;
    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(CATPPUCCIN_GREEN);
    tft.print(mode);
    // --- Draw MQTT status icon ---
    if (isMqttEnabled) {
      drawMqttStatusIcon(mqttConnected, tft.getCursorX() + 8, y_pos + 3);
    }
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
  }

  // --- Draw Storage Info ---
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

/**
 * @brief Displays the FTP mode screen.
 */
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB, bool mqttConnected) {
  drawModeScreen("FTP", CATPPUCCIN_GREEN, ip, mac, files, totalSizeMB, freeSizeMB, mqttConnected);
}

/**
 * @brief Displays the USB MSC mode screen.
 */
void drawUsbMscModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB, bool mqttConnected) {
  drawModeScreen("USB MSC", CATPPUCCIN_MAUVE, ip, mac, files, totalSizeMB, freeSizeMB, mqttConnected);
}
