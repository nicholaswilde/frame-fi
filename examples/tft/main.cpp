#include "Arduino.h"
#include "secrets.h"

// External libraries
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include "catppuccin_colors.h" // Include our custom color palette

TFT_eSPI tft = TFT_eSPI();

// Prototypes
void drawApModeScreen(const char* ap_ssid, const char* ap_ip);

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
  Serial.println("test");
  delay(1000);
}

void drawApModeScreen(const char* ap_ssid, const char* ap_ip) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi Setup");

  // Display instructions centered on the screen
  int y_pos = 30;
  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.drawCentreString("Connect to WiFi:", 80, y_pos, 1);
  y_pos += 12;
  
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.drawCentreString(ap_ssid, 80, y_pos, 2); // Larger font for SSID
  y_pos += 18;

  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.drawCentreString("Go to http://", 80, y_pos, 1);
  y_pos += 12;
  
  tft.setTextColor(CATPPUCCIN_GREEN);
  tft.drawCentreString(ap_ip, 80, y_pos, 2);
}

