/******************************************
 * button
 * ----------------
 * Use a button to switch between mock FTP
 * and MSC modes.
 *
 * @author Nicholas Wilde, 0xb299a622
 * @date 25 Jul 2025
 * @version 0.1.0
 *
 ******************************************/ 
 
#include "Arduino.h"
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h" // Import sensitive data

// External libraries
#include <OneButton.h>

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
void enterMscMode();
bool enterFtpMode();
void toggleMode();

void setup(){
  Serial.begin(115200);
  delay(1000);

  // --- Initialize Button ---
  button.attachClick(toggleMode);
  
  // --- Connect to WiFi ---
  connectToWiFi();
}

void loop(){
  // Keep polling the button state
  button.tick();
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
 * @brief Connects to the WiFi network and provides visual feedback.
 */
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
