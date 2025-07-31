#include "Arduino.h"
#include "secrets.h"

// External libraries
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include "catppuccin_colors.h" // Include our custom color palette

TFT_eSPI tft = TFT_eSPI();

// Define an enum for device modes for clarity
enum DeviceMode {
  AP_MODE,
  FTP_MODE,
  USB_MODE
};

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
void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB);
void drawUsbMscModeScreen(const char* mac, int files, int totalSizeMB, float freeSizeMB);

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

   // --- EXAMPLE USAGE ---
  // In your real code, this variable would be set based on device state
  DeviceMode currentMode = AP_MODE;
  
  // Static data for the example
  const char* ip_addr = "192.168.1.101";
  const char* mac_addr = "DE:AD:BE:EF:FE:ED";
  const char* ap_ssid = "AutoConnectAP-FrameFi";
  const char* ap_ip = "192.168.4.1";
  int file_count = 1482;
  int total_mb = 512;
  float free_mb = 230.1;
  
    // Call the correct drawing function based on the current mode
  if (currentMode == AP_MODE) {
    drawApModeScreen(ap_ssid, ap_ip);
  }
  else if (currentMode == FTP_MODE) {
    drawFtpModeScreen(ip_addr, mac_addr, file_count, total_mb, free_mb);
  }
  else if (currentMode == USB_MODE) {
    drawUsbMscModeScreen(mac_addr, file_count, total_mb, free_mb);
  }
}

void loop(){ 
  Serial.println("test");
  delay(1000);
}

// Draws the top header bar
void drawHeader(const char* title) {
  tft.fillRect(0, 0, TFT_HEIGHT, 12, CATPPUCCIN_BLUE);
  tft.setTextColor(CATPPUCCIN_CRUST);
  tft.setTextSize(1);
  tft.drawCentreString(title, TFT_WIDTH, 2, 1); // x-center, y, font
  tft.setTextSize(1);
}

// Draws the right column with storage statistics
void drawStorageInfo(int files, int totalSizeMB, float freeSizeMB) {
  int y_pos = 53;
  int x_pos = 5;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Size:  ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(totalSizeMB);
  tft.print("MB");

  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print(" Files: ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(files);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Free:  ");
  tft.setTextColor(CATPPUCCIN_PEACH);
  tft.print(freeSizeMB, 0);
  tft.print("MB");
}

void drawApModeScreen(const char* ap_ssid, const char* ap_ip) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi Setup");

  // Left Column: Network Info
  int y_pos = 17;
  int x_pos = 5;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:  ");
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("AP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("AP IP: ");
  tft.setTextColor(CATPPUCCIN_YELLOW); // Use green for active mode
  tft.print(ap_ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("SSID:  ");
  y_pos += 12;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW); // Use green for active mode
  tft.print(ap_ssid);
}

void drawFtpModeScreen(const char* ip, const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi");

  // Left Column: Network Info
  int y_pos = 17;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:  ");
  tft.setTextColor(CATPPUCCIN_GREEN); // Use green for active mode
  tft.print("FTP");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP:    ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(ip);
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:   ");
  // y_pos += 10;
  // tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(mac);

  // Right Column: Storage Info
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}

void drawUsbMscModeScreen(const char* mac, int files, int totalSizeMB, float freeSizeMB) {
  tft.fillScreen(CATPPUCCIN_BASE);
  drawHeader("FrameFi");

  // Left Column: Network Info
  int y_pos = 17;
  int x_pos = 5;
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("Mode:  ");
  tft.setTextColor(CATPPUCCIN_GREEN);
  tft.print("USB MSC");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("IP:    ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print("N/A");
  y_pos += 12;

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(CATPPUCCIN_MAUVE);
  tft.print("MAC:   ");
  tft.setTextColor(CATPPUCCIN_YELLOW);
  tft.print(mac);

  // Right Column: Storage Info
  drawStorageInfo(files, totalSizeMB, freeSizeMB);
}
