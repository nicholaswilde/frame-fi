# :signal_strength: FrameFi: Wireless Freedom for Digital Picture Frames :framed_picture:
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/test.yaml)

FrameFi transforms a [LILYGO T-Dongle S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

## :stopwatch: TL;DR

**Secrets:** Create `include/secrets.h` and update variables.

```shell
cp includes/secrets.h.tmpl includes/secrets.h
```

**Computer:** Plug in the LILYGO T-Dongle S3 to a computer USB port while holding the button to put it into boot mode.

**Upload Sketch:** Upload the sketch to the dongle.

```shell
task upload
```

or

```shell
pio run --target upload
```

**Wi-Fi Credentials:** Connect to `AutoConnectAP-Frame-Fi` access point and enter WiFi credentials. If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.

**Digital Picture Frame:** Plug in dongle to digital picture frame.

**Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

```sh
curl -X POST http://<DEVICE_IP>/ftp
```

```sh
curl -X POST http://<DEVICE_IP>/msc
```

**FTP Mode:** Upload pictures to the dongle via FTP using `lftp` or `scripts/sync.sh`.

```shell
lftp -c "
set ftp:ssl-allow no;
open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
mirror -R --delete --verbose --parallel=1 '<REMOTE_DIR>' '<LOCAL_DIR>';
"
```

## :sparkles: Features

- **Dual-Mode Operation:** Seamlessly switch between a USB Mass Storage (MSC) device and an FTP server.
- **Web Interface & API:** A built-in web server provides an API to check the device's status and switch between modes.
- **Wireless File Management:** In FTP mode, you can connect to the device over your Wi-Fi network to add, remove, and manage files on the microSD card.
> [!CAUTION]
> FTP is an insecure protocol. Only use this feature on a trusted network.
- **USB Mass Storage Mode:** In MSC mode, the device mounts the microSD card as a standard USB thumb drive, allowing for high-speed file transfers directly from your computer.
- **Fast Data Transfer:** Utilizes the `SD_MMC` interface for the microSD card, offering significantly faster read/write speeds compared to the standard SPI interface.
- **Dynamic WiFi Configuration:** Uses `WiFiManager` to create a captive portal for easy Wi-Fi setup without hardcoding credentials.
- **Easy WiFi Reset:** Hold the button for 3 seconds to clear saved WiFi credentials and re-enter setup mode.
- **Boot-up Screen:** Displays a welcome screen with the current firmware version on startup.
- **LED Status Indicators:** A built-in LED provides at-a-glance status updates for different modes.
- **LCD Display:** Displays relevant information on the LCD display depending on the mode, including a bar graph showing SD card usage. It also utilizes catppuccin color schemes.

## :electric_plug: Hardware Requirements

- **LILYGO T-Dongle-S3:** This project is specifically designed for this board.
- **microSD Card:** A FAT32 formatted microSD card is required to store pictures and files. The sketch has been tested with a 16GB card.
- **Digital Picture Frame:** A digital picture frame that can use a USB storage device to serve pictures.

## :package: Software Dependencies

- **PlatformIO:** This project is built using the PlatformIO ecosystem. You can install it as a [VSCode Extension](https://platformio.org/install/ide?install=vscode) or use the [PlatformIO Core (CLI)](https://platformio.org/install/cli).
- **Task (optional):** A task runner / build tool. Installation instructions can be found [here](https://taskfile.dev/installation/).

