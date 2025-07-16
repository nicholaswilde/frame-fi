#include "secrets.h"
#include <Arduino.h>
#include <FastLED.h>

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(leds, NUM_LEDS);
}

void loop() {
  Serial.println("Hello world!");
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(1000);

  leds[0] = CRGB::Black;
  FastLED.show();
  delay(1000);
}
