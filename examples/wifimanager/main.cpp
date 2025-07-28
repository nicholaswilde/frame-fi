/******************************************
 *
 * wifimanager
 * ----------------
 * Use an AP to serve a web configuration
 * portal
 *
 * @author Nicholas Wilde, 0xb299a622
 * @date 25 Jul 2025
 * @version 0.1.0
 *
 ******************************************/ 
 
// Include the required libraries
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "secrets.h"

void setup() {
  // Start Serial for debugging purposes
  Serial.begin(115200);
  // Wait for the serial port to connect. Needed for native USB only.
  while(!Serial) {
    delay(10);
  }

  Serial.println("\nStarting");

  // Explicitly set the ESP32 to be a WiFi-client (station)
  WiFi.mode(WIFI_STA);

  // Create a WiFiManager object
  // WiFiManager is a blocking process. It will wait here until a connection is made.
  WiFiManager wm;

  // You can uncomment the following line to reset saved credentials.
  // Useful for testing the captive portal.
  // wm.resetSettings();

  // Set a timeout for the configuration portal. If no one connects to the
  // portal within this time, the ESP32 will continue without a connection.
  // The timeout is in seconds.
  wm.setConfigPortalTimeout(180); // 3 minutes

  // autoConnect() starts a WiFi access point with a specific name.
  // If it fails to connect to a saved network, it will create an AP
  // with the name "AutoConnectAP-T-Dongle".
  // The "password" parameter is the password for this AP.
  // If you leave the password parameter empty, it will be an open network.
  bool res = wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD);

  if(!res) {
    Serial.println("Failed to connect and hit timeout. Restarting...");
    // You could implement a deep sleep here or simply restart.
    ESP.restart();
  }
  else {
    // If you get here, you have connected to the WiFi!
    Serial.println("");
    Serial.println("Wi-Fi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // Your main code goes here.
  // This will only run after a successful WiFi connection.
  Serial.println("Looping... Still connected to Wi-Fi.");
  delay(10000); // Do something every 10 seconds
}
