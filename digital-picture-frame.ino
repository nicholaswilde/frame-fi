#include "pin_config.h"
#include "secrets.h"

// The BOOT button is on GPIO0
#define BOOT_BUTTON_PIN 0

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial to initialize

  // Configure the BOOT button pin
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("LILYGO T-Dongle S3 - SFTP Server & USB Drive (SD_MMC Version)");
}

void loop() {
  Serial.print("IP address");
  delay(5);
}
