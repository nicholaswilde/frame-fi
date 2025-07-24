/*
 * Simple FTP Server Example with SD Card on ESP32
 *
 * AUTHOR: Renzo Mischianti
 * URL: https://www.mischianti.org
 *
 * DESCRIPTION:
 * This example demonstrates how to use the SimpleFTPServer library
 * with an ESP32 and an SD card module. The ESP32 connects to a WiFi network
 * and initializes an FTP server for file transfers.
 *
 * FEATURES:
 * - WiFi connection to local network
 * - SD card initialization for file storage
 * - FTP server setup for file uploads and downloads
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include <WiFi.h>
#include <SimpleFTPServer.h>
#include <SPI.h>
#include <SD.h>
#include <SD_MMC.h>
#include "secrets.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"
sdmmc_card_t *card;

// FTP server instance
FtpServer ftpServer;

void sd_init(void) {
  esp_err_t ret;
  const char mount_point[] = MOUNT_POINT;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024
  };

  sdmmc_host_t host = {
      .flags = SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_DDR,
      .slot = SDMMC_HOST_SLOT_1,
      .max_freq_khz = SDMMC_FREQ_DEFAULT,
      .io_voltage = 3.3f,
      .init = &sdmmc_host_init,
      .set_bus_width = &sdmmc_host_set_bus_width,
      .get_bus_width = &sdmmc_host_get_slot_width,
      .set_bus_ddr_mode = &sdmmc_host_set_bus_ddr_mode,
      .set_card_clk = &sdmmc_host_set_card_clk,
      .do_transaction = &sdmmc_host_do_transaction,
      .deinit = &sdmmc_host_deinit,
      .io_int_enable = sdmmc_host_io_int_enable,
      .io_int_wait = sdmmc_host_io_int_wait,
      .command_timeout_ms = 0,
  };
  sdmmc_slot_config_t slot_config = {
      .clk = (gpio_num_t)SD_MMC_CLK_PIN,
      .cmd = (gpio_num_t)SD_MMC_CMD_PIN,
      .d0 = (gpio_num_t)SD_MMC_D0_PIN,
      .d1 = (gpio_num_t)SD_MMC_D1_PIN,
      .d2 = (gpio_num_t)SD_MMC_D2_PIN,
      .d3 = (gpio_num_t)SD_MMC_D3_PIN,
      .cd = SDMMC_SLOT_NO_CD,
      .wp = SDMMC_SLOT_NO_WP,
      .width = 4, // SDMMC_SLOT_WIDTH_DEFAULT,
      .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
  };

  gpio_set_pull_mode((gpio_num_t)SD_MMC_CMD_PIN, GPIO_PULLUP_ONLY); // CMD, needed in 4- and 1- line modes
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D0_PIN, GPIO_PULLUP_ONLY);  // D0, needed in 4- and 1-line modes
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D1_PIN, GPIO_PULLUP_ONLY);  // D1, needed in 4-line mode only
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D2_PIN, GPIO_PULLUP_ONLY);  // D2, needed in 4-line mode only
  gpio_set_pull_mode((gpio_num_t)SD_MMC_D3_PIN, GPIO_PULLUP_ONLY);  // D3, needed in 4- and 1-line modes

  ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      Serial.println("Failed to mount filesystem. "
                        "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    } else {
      Serial.printf("Failed to initialize the card (%s). "
                       "Make sure SD card lines have pull-up resistors in place.",
                       esp_err_to_name(ret));
    }
    return;
  }
}

void setup(){
   // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial) {
    // Wait for serial port to connect (required for native USB ports)
  }

  // Connect to WiFi network
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.printf("Connected to: %s\n", WIFI_SSID);
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());

  // Wait for a short delay before initializing SD card
  delay(1000);

  // Initialize SD card
  Serial.print("Initializing SD card...");
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
  // if (!SD.begin()) {
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("Card Mount Failed");
    return;
  }
  // sd_init();
  Serial.println("\nSD card initialized successfully!");

  // Start FTP server with username and password
  ftpServer.begin(FTP_USER, FTP_PASSWORD); // Replace with your desired FTP credentials
  Serial.println("FTP server started!");
}

void loop(){
  // Handle FTP server operations
  ftpServer.handleFTP(); // Continuously process FTP requests  
};
