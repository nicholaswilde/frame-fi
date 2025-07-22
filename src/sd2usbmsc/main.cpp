#include <Arduino.h>
#include <USB.h>
#include <USBMSC.h>
#include "SD_MMC.h"
#include "sdmmc_cmd.h"

#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"

// USB Mass Storage Class (MSC) object
USBMSC msc;
#define HWSerial    Serial0

bool onebit = false;
sdmmc_card_t *card;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  // HWSerial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_write_sectors(card, buffer + offset, lba, count);
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  // HWSerial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  uint32_t count = (bufsize / card->csd.sector_size);
  sdmmc_read_sectors(card, buffer + offset, lba, count);
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  // HWSerial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT: Serial.println("USB PLUGGED"); break;
      case ARDUINO_USB_STOPPED_EVENT: Serial.println("USB UNPLUGGED"); break;
      case ARDUINO_USB_SUSPEND_EVENT: Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en); break;
      case ARDUINO_USB_RESUME_EVENT:  Serial.println("USB RESUMED"); break;

      default: break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Serial");
  Serial.println("Mounting SDcard");
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
  if (!SD_MMC.begin("/sdcard", onebit)) {
    Serial.println("Mount Failed");
    return;
  }
  Serial.println("Initializing MSC");
  // Initialize USB metadata and callbacks for MSC (Mass Storage Class)
  msc.vendorID("ESP32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  // msc.begin();
  msc.begin(card->csd.capacity, card->csd.sector_size);
  USB.onEvent(usbEventCallback);
  USB.begin();

  // Serial.printf("Card Size: %lluMB\n", SD_MMC.totalBytes() / 1024 / 1024);
  // Serial.printf("Sector: %d\tCount: %d\n", SD_MMC.sectorSize(), SD_MMC.numSectors());
}

void loop() {
  Serial.println("Hello World!");
  delay(1000);
}
