#include <Arduino.h>
#include <SD_MMC.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Serial");
}

void loop() {
  Serial.println("Hello World!");
  delay(1000);
}
