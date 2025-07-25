/*************************************
 * blink
 * ----------------
 * Blink an LED
 *
 * @author Nicholas Wilde, 0xb299a622
 * @date 25 Jul 2025
 * @version 0.1.0
 *
 *************************************/

#include <Arduino.h>
#include <FastLED.h>

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  // Initialize the LED pin as an output
  FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(leds, NUM_LEDS);
}

void loop() {
  // Turn the LED on
  leds[0] = CRGB::Green;
  FastLED.show();
  Serial.println("Green");
// Wait for a second
  delay(1000);
  
  // Turn the LED off
  leds[0] = CRGB::Black;
  FastLED.show();
  Serial.println("Off");
  // Wait for a second
  delay(1000);  
} 
