/******************************************
 * 
 * wifimanager
 * ----------------
 * Use an AP to serve a web configuration
 * portal with MQTT settings.
 * 
 * @author Nicholas Wilde, 0xb299a622
 * @date 25 Jul 2025
 * @version 0.3.0
 * 
 ******************************************/

// Include the required libraries
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// --- AP Mode ---
#define WIFI_AP_SSID "AutoConnectAP-FrameFi"
#define WIFI_AP_PASSWORD "password"

// --- MQTT Configuration ---
char mqtt_host[64] = "192.168.1.100";
char mqtt_port[6] = "1883";
char mqtt_user[32] = "";
char mqtt_pass[32] = "";
char mqtt_client_id[32] = "FrameFi";

// --- WiFi and MQTT Clients ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- Configuration file path ---
#define CONFIG_FILE "/config.json"

// Flag to check if we should save the config
bool shouldSaveConfig = false;

/**
 * Callback notifying us of the need to save config.
 */
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

/**
 * Saves the configuration to a file.
 */
void saveConfig() {
  Serial.println("Saving config");
  DynamicJsonDocument doc(1024);
  doc["mqtt_host"] = mqtt_host;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_user"] = mqtt_user;
  doc["mqtt_pass"] = mqtt_pass;
  doc["mqtt_client_id"] = mqtt_client_id;

  File configFile = LittleFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  serializeJson(doc, configFile);
  configFile.close();
}

/**
 * Loads the configuration from a file.
 */
void loadConfig() {
  if (LittleFS.exists(CONFIG_FILE)) {
    Serial.println("Loading config");
    File configFile = LittleFS.open(CONFIG_FILE, "r");
    if (configFile) {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, configFile);
      if (!error) {
        strcpy(mqtt_host, doc["mqtt_host"]);
        strcpy(mqtt_port, doc["mqtt_port"]);
        strcpy(mqtt_user, doc["mqtt_user"]);
        strcpy(mqtt_pass, doc["mqtt_pass"]);
        strcpy(mqtt_client_id, doc["mqtt_client_id"]);
      } else {
        Serial.println("Failed to load json config");
      }
      configFile.close();
    }
  } else {
    Serial.println("Config file not found, using defaults.");
  }
}

/**
 * Handles incoming MQTT messages.
 */
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  Serial.println("-----------------------");
}

/**
 * Reconnects to the MQTT broker.
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe to a topic
      client.subscribe("frame-fi/display/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * Resets all settings and restarts the device.
 */
void resetSettings() {
  // Reset WiFiManager settings
  WiFiManager wm;
  wm.resetSettings();
  Serial.println("WiFi settings have been reset.");

  // Erase the configuration file
  if (LittleFS.exists(CONFIG_FILE)) {
    LittleFS.remove(CONFIG_FILE);
    Serial.println("Configuration file has been deleted.");
  }

  // Restart the device
  Serial.println("Restarting device...");
  ESP.restart();
}

void setup() {
  // Start Serial for debugging purposes
  Serial.begin(115200);
  // Wait for the serial port to connect. Needed for native USB only.
  while (!Serial) {
    delay(10);
  }

  Serial.println("\nStarting");

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return;
  }

  // Uncomment the following line to reset all settings
  // resetSettings();

  // Load existing configuration
  loadConfig();

  // Explicitly set the ESP32 to be a WiFi-client (station)
  WiFi.mode(WIFI_STA);


  // Create a WiFiManager object
  WiFiManager wm;

  // Set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // Add custom parameters for MQTT
  WiFiManagerParameter custom_mqtt_host("mqtt_host", "MQTT Host", mqtt_host, 
                                        64);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", mqtt_user, 
                                        32);
  WiFiManagerParameter custom_mqtt_pass("mqtt_pass", "MQTT Password", 
                                        mqtt_pass, 32);
  WiFiManagerParameter custom_mqtt_client_id("mqtt_client_id", "MQTT Client ID", 
                                             mqtt_client_id, 32);

  wm.addParameter(&custom_mqtt_host);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  wm.addParameter(&custom_mqtt_client_id);

  //reset settings - for testing
  // wm.resetSettings();

  // Set a timeout for the configuration portal
  wm.setConfigPortalTimeout(180);

  if (!wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
    Serial.println("Failed to connect and hit timeout. Restarting...");
    ESP.restart();
  } else {
    Serial.println("");
    Serial.println("Wi-Fi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // Read updated parameters
  strcpy(mqtt_host, custom_mqtt_host.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  strcpy(mqtt_client_id, custom_mqtt_client_id.getValue());

  // Save the custom parameters to FS
  if (shouldSaveConfig) {
    saveConfig();
  }
  Serial.print("mqtt_host: ");
  Serial.println(mqtt_host);

  Serial.print("mqtt_port: ");
  Serial.println(String(mqtt_port).toInt());

  Serial.print("mqtt_client_id: ");
  Serial.println(mqtt_client_id);

  // Setup MQTT client
  client.setServer(mqtt_host, String(mqtt_port).toInt());
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Add a small delay to prevent watchdog timeouts
  delay(10);
}
