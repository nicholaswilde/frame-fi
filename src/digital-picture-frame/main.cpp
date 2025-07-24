// Include necessary libraries
#include <WiFi.h>
#include <WebServer.h>
#include "FS.h"
#include "SD_MMC.h"
#include "USB.h"
#include "USBMSC.h"
#include <SimpleFTPServer.h>
#include <FastLED.h>
#include <OneButton.h>
#include "secrets.h" // Import sensitive data

// --- Hardware Pin Definitions ---
#define BTN_PIN            0
#define LED_DI_PIN         40
#define LED_CI_PIN         39

// --- FastLED Configuration for APA102 ---
#define NUM_LEDS           1
#define BRIGHTNESS         50
CRGB leds[NUM_LEDS];

// --- Custom SD_MMC Pin Configuration ---
#define SD_MMC_CLK_PIN     12
#define SD_MMC_CMD_PIN     16
#define SD_MMC_D0_PIN      14
#define SD_MMC_D1_PIN      17
#define SD_MMC_D2_PIN      21
#define SD_MMC_D3_PIN      18

// --- Mode Descriptions ---
const char* MODE_MSC_DESC = "USB MSC";
const char* MODE_FTP_DESC = "Application (FTP Server)";

// Create objects
WebServer server(80);
FtpServer ftpSrv;
USBMSC MSC;
OneButton button(BTN_PIN, true); // true for active low

// A flag to track the current mode
bool isInMscMode = false;

// Function prototypes
void setLedColor(CRGB color);
void connectToWiFi();
void setupApiRoutes();
void startFtpServer();
void enterMscMode();
bool enterFtpMode();
void handleStatus();
void handleSwitchToMSC();
void handleSwitchToFTP();
void toggleMode();

void setup() {
  Serial.begin(115200);
  delay(1000);

  // --- Initialize LED ---
  FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  Serial.println("ESP32-S3 Dual-Control FTP/MSC Switching with LED Status");
  Serial.println("------------------------------------------------------");

  // --- Initialize Button ---
  button.attachClick(toggleMode);

  // --- Connect to WiFi ---
  connectToWiFi();

  // --- Initialize USB ---
  USB.begin();

  // --- Setup and start Web Server ---
  setupApiRoutes();
  server.begin();
  Serial.println("HTTP server started.");

  // Configure SD_MMC pins once
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, 
                 SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);

  // --- Start in USB MSC Mode ---
  isInMscMode = true;
  Serial.println("Starting in MSC mode by default.");
  // We don't need to stop FTP or unmount SD because they haven't been started yet.
  MSC.begin();
  Serial.println("\n✅ MSC mode active. Connect USB to a computer.");
  setLedColor(CRGB::Magenta);
}

void loop() {
  // Keep polling the button state
  button.tick();
  
  // Handle incoming web client requests
  server.handleClient();

  // If in FTP mode, handle FTP client connections
  if (!isInMscMode) {
    ftpSrv.handleFTP();
  }
}

/**
 * @brief Toggles between FTP and MSC modes when the button is pressed.
 */
void toggleMode() {
  Serial.println("Button clicked! Toggling mode...");
  if (isInMscMode) {
    isInMscMode = false;
    enterFtpMode();
  } else {
    isInMscMode = true;
    enterMscMode();
  }
}

/**
 * @brief Sets the color of the status LED.
 */
void setLedColor(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

/**
 * @brief Connects to the WiFi network and provides visual feedback.
 */
void connectToWiFi() {
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    setLedColor(CRGB::Blue);
    delay(250);
    setLedColor(CRGB::Black);
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
    setLedColor(CRGB::Yellow);
  }
}

/**
 * @brief Defines the web server API endpoints.
 */
void setupApiRoutes() {
  server.on("/", HTTP_GET, handleStatus);
  server.on("/msc", HTTP_POST, handleSwitchToMSC);
  server.on("/ftp", HTTP_POST, handleSwitchToFTP);
}

/**
 * @brief Starts the FTP Server and sets the LED color.
 */
void startFtpServer() {
  Serial.println("Starting FTP Server...");
  ftpSrv.begin(FTP_USER, FTP_PASSWORD, "/"); 
  Serial.println("FTP Server started.");
  setLedColor(CRGB::Green);
}

/**
 * @brief Stops FTP, unmounts SD, and enables USB MSC mode.
 */
void enterMscMode() {
  Serial.println("\n--- Entering MSC Mode ---");
  ftpSrv.stop();
  Serial.println("FTP Server stopped.");
  SD_MMC.end();
  Serial.println("SD Card released.");
  MSC.begin();
  Serial.println("\n✅ Switched to MSC mode. Connect USB to a computer.");
  setLedColor(CRGB::Magenta);
}

/**
 * @brief Mounts SD card and starts the FTP server.
 * @return true if successful, false otherwise.
 */
bool enterFtpMode() {
  Serial.println("\n--- Entering Application (FTP) Mode ---");
  if (!SD_MMC.begin()) {
    Serial.println("Error: Card Mount Failed.");
    setLedColor(CRGB::Red);
    return false;
  } else {
    Serial.println("\n✅ Application mode active.");
    startFtpServer();
    return true;
  }
}

/**
 * @brief Handles requests to the root URL ("/"). Sends a JSON status object.
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
  if (!isInMscMode) {
    isInMscMode = true;
    enterMscMode();
    String jsonResponse = "{\"status\":\"success\", \"message\":\"Switched to MSC mode.\"}";
    server.send(200, "application/json", jsonResponse);
  } else {
    String jsonResponse = "{\"status\":\"no_change\", \"message\":\"Already in MSC mode.\"}";
    server.send(200, "application/json", jsonResponse);
  }
}

/**
 * @brief Handles the POST request to switch back to Application (FTP) mode.
 */
void handleSwitchToFTP() {
  if (isInMscMode) {
    isInMscMode = false;
    if (enterFtpMode()) {
      String jsonResponse = "{\"status\":\"success\", \"message\":\"Switched to Application (FTP) mode.\"}";
      server.send(200, "application/json", jsonResponse);
    } else {
      String jsonResponse = "{\"status\":\"error\", \"message\":\"Failed to re-initialize SD card.\"}";
      server.send(500, "application/json", jsonResponse);
    }
  } else {
    String jsonResponse = "{\"status\":\"no_change\", \"message\":\"Already in Application (FTP) mode.\"}";
    server.send(200, "application/json", jsonResponse);
  }
}
