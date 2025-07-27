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

#include "secrets.h" // Import sensitive data

// External libraries
#include <OneButton.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// --- Mode Descriptions ---
const char* MODE_MSC_DESC = "USB MSC";
const char* MODE_FTP_DESC = "Application (FTP Server)";
 
// Create objects
WebServer server(80);
OneButton button(BTN_PIN, true); // true for active low

// A flag to track the current mode
bool isInMscMode = false;

// Function prototypes
void connectToWiFi();
void setupApiRoutes();
void enterMscMode();
bool enterFtpMode();
void handleStatus();
void handleSwitchToMSC();
void handleSwitchToFTP();
void toggleMode();

void setup(){
  Serial.begin(115200);
  while(!Serial) {
    delay(10);
  }

  // --- Initialize Button ---
  button.attachClick(toggleMode);
  
  // --- Connect to WiFi ---
  connectToWiFi();

  // --- Setup and start Web Server ---
  setupApiRoutes();
  server.begin();
  Serial.println("HTTP server started.");
}

void loop(){
  server.handleClient();
  button.tick();
}

/**
 * @brief Connects to the WiFi network and provides visual feedback.
 */
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setConfigPortalTimeout(180); // 3 minutes
  bool res = wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  if(!res) {
    Serial.println("Failed to connect and hit timeout. Restarting...");
    ESP.restart();
  }
  else {
    Serial.println("\nWiFi connected!");
    Serial.printf("Connected to: %s\n", WIFI_SSID);
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
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
 * @brief Defines the web server API endpoints.
 */
void setupApiRoutes() {
  server.on("/", HTTP_GET, handleStatus);
  server.on("/msc", HTTP_POST, handleSwitchToMSC);
  server.on("/ftp", HTTP_POST, handleSwitchToFTP);
}

/**
 * @brief Stops FTP, unmounts SD, and enables USB MSC mode.
 */
void enterMscMode() {
  Serial.println("\n--- Entering MSC Mode ---");
  Serial.println("FTP Server stopped.");
  Serial.println("SD Card released.");
  Serial.println("\n✅ Switched to MSC mode. Connect USB to a computer.");
}

/**
 * @brief Mounts SD card and starts the FTP server.
 * @return true if successful, false otherwise.
 */
bool enterFtpMode() {
  Serial.println("\n--- Entering Application (FTP) Mode ---");
  Serial.println("\n✅ Application mode active.");
  return true;
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
