#include <Arduino.h>

// Define the LED pin from the platformio.ini
#define LED_PIN 40

void setup() {
  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Turn the LED on
  digitalWrite(LED_PIN, HIGH);
  // Wait for a second
  delay(1000);
  // Turn the LED off
  digitalWrite(LED_PIN, LOW);
  // Wait for a second
  delay(1000);
} 