#include <Arduino.h>

void setup() {
  // Initialize the LED pin as an output
  pinMode(LED_DI_PIN, OUTPUT);
}

void loop() {
  // Turn the LED on
  digitalWrite(LED_DI_PIN, HIGH);
  // Wait for a second
  delay(1000);
  // Turn the LED off
  digitalWrite(LED_DI_PIN, LOW);
  // Wait for a second
  delay(1000);
} 
