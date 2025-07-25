#include "Arduino.h"
#include <WiFi.h>
#include <WebServer.h>

#include "secrets.h" // Import sensitive data

// --- Mode Descriptions ---
const char* MODE_MSC_DESC = "USB MSC";
const char* MODE_FTP_DESC = "Application (FTP Server)";
 
// Create objects
WebServer server(80);

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
// void toggleMode();

void setup(){
  Serial.begin(115200);
  delay(1000);

  // --- Connect to WiFi ---
  connectToWiFi();

  // --- Setup and start Web Server ---
  setupApiRoutes();
  server.begin();
  Serial.println("HTTP server started.");
}

void loop(){
  delay(-1);
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.printf("Connected to: %s\n", WIFI_SSID);
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
}

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
  // ftpSrv.stop();
  Serial.println("FTP Server stopped.");
  // SD_MMC.end();
  Serial.println("SD Card released.");
  // MSC.begin();
  Serial.println("\n✅ Switched to MSC mode. Connect USB to a computer.");
  // setLedColor(CRGB::Magenta);
}

/**
 * @brief Mounts SD card and starts the FTP server.
 * @return true if successful, false otherwise.
 */
bool enterFtpMode() {
  Serial.println("\n--- Entering Application (FTP) Mode ---");
  // if (!SD_MMC.begin()) {
    // Serial.println("Error: Card Mount Failed.");
    // // setLedColor(CRGB::Red);
    // return false;
  // } else {
    Serial.println("\n✅ Application mode active.");
    // startFtpServer();
    return true;
  // }
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
