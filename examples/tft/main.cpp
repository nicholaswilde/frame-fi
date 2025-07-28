#include "Arduino.h"
#include "secrets.h"
// #include "pin_config.h"

// External libraries
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();

void setup(){
  int32_t x, y;
  Serial.begin(115200);
  Serial.println("Hello T-Dongle-S3");
  tft.init();
}

void loop(){ 
  // tft.setCursor(0, 0, 2);
  // tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  // tft.println("Hello World!");
  Serial.println("test");
  delay(1000);
}
