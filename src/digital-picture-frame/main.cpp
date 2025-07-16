#include "secrets.h"

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial to initialize

  // Configure the BOOT button pin
  pinMode(BTN_PIN, INPUT_PULLUP);

  Serial.println("LILYGO T-Dongle S3 - SFTP Server & USB Drive (SD_MMC Version)");
}

void loop() {
  Serial.println("IP address");
  delay(1000);
} 
