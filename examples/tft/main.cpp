#include "Arduino.h"
#include "secrets.h"

// External libraries
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include "catppuccin_colors.h" // Include our custom color palette

TFT_eSPI tft = TFT_eSPI();

#define PRINT_STR(str, x, y)                                                                                                                         \
  do {                                                                                                                                               \
    Serial.println(str);                                                                                                                             \
    tft.drawString(str, x, y);                                                                                                                       \
    y += 8;                                                                                                                                          \
  } while (0);

void setup(){
  int32_t x, y;
  Serial.begin(115200);
  Serial.println("Hello T-Dongle-S3");
  pinMode(38, OUTPUT);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(CATPPUCCIN_BASE);
  digitalWrite(38, 0);
  tft.setTextFont(1);
  tft.setTextColor(CATPPUCCIN_TEXT, CATPPUCCIN_BASE);
  PRINT_STR("Hello World!", x, y)
  PRINT_STR("Hello World2!", x, y)
}

void loop(){ 
  // tft.setCursor(0, 0, 2);
  // tft.println("Hello World!");
  Serial.println("test");
  delay(1000);
}
