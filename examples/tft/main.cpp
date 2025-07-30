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

// Prototypes
void drawHeader(const char* title);
void drawStorageInfo(int files, float totalSizeMB, float freeSizeMB);
void drawApModeScreen(const char* ap_ssid, const char* ap_ip);
void drawFtpModeScreen(const char* ip, const char* mac, int files, float totalSizeMB, float freeSizeMB);

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

// Draws the top header bar
void drawHeader(const char* title) {
  tft.fillRect(0, 0, TFT_HEIGHT, 20, CATPPUCCIN_BLUE);
  tft.setTextColor(CATPPUCCIN_CRUST);
  tft.setTextSize(2);
  tft.drawCentreString(title, TFT_WIDTH, 2, 1); // x-center, y, font
  tft.setTextSize(1);
}

// Draws the right column with storage statistics
void drawStorageInfo(int files, float totalSizeMB, float freeSizeMB) {
  int y_pos = 25;
  int x_pos = 85;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print("Files: ");
  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.print(files);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print("Size: ");
  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.print(totalSizeMB, 1);
  tft.print("MB");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print("Free: ");
  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.print(freeSizeMB, 1);
  tft.print("MB");
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

void drawFtpModeScreen(const char* ip, const char* mac, int files, float totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi");
  tft.drawFastVLine(80, 22, 56, CATPPUCCIN_LAVENDER);

  // Left Column: Network Info
  int y_pos = 25;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode: ");
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("FTP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP: ");
  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.print(ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:");
  y_pos += 10;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_TEXT);
  tft.print(mac);

  // Right Column: Storage Info
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

