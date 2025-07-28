# :framed_picture: FrameFi: Wireless Adapter for Digital Picture Frames :signal_strength:

FrameFi transforms a [LILYGO T-Dongle-S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

## :sparkles: Features

- **Dual-Mode Operation:** Seamlessly switch between a USB Mass Storage (MSC) device and an FTP server.
- **Web Interface & API:** A built-in web server provides an API to check the device's status and switch between modes.
- **Wireless File Management:** In FTP mode, you can connect to the device over your Wi-Fi network to add, remove, and manage files on the microSD card.
- **USB Mass Storage Mode:** In MSC mode, the device mounts the microSD card as a standard USB thumb drive, allowing for high-speed file transfers directly from your computer.
- **Fast Data Transfer:** Utilizes the `SD_MMC` interface for the microSD card, offering significantly faster read/write speeds compared to the standard SPI interface.
- **Dynamic WiFi Configuration:** Uses `WiFiManager` to create a captive portal for easy Wi-Fi setup without hardcoding credentials.
- **LED Status Indicators:** A built-in LED provides at-a-glance status updates for different modes.

## :electric_plug: Hardware Requirements

- **LILYGO T-Dongle-S3:** This project is specifically designed for this board.
- **microSD Card:** A FAT32 formatted microSD card is required to store pictures and files. The sketch has been tested with a 16GB card.

> [!WARNING]
> This sketch is configured to use the `SD_MMC` library, which requires the microSD card slot to be wired to the
> ESP32-S3's dedicated SD_MMC pins. Most T-Dongle-S3 boards have the integrated slot wired for SPI only. This code
> will likely fail to initialize the SD card unless you have custom hardware that routes the SD card to the correct
> SD_MMC pins as defined in `pin_config.h`.

## :floppy_disk: Software Dependencies

- **PlatformIO:** This project is built using the PlatformIO ecosystem. You can install it as a [VSCode Extension](https://platformio.org/install/ide?install=vscode) or use the [PlatformIO Core (CLI)](https://platformio.org/install/cli).
- **Task:** A task runner / build tool. Installation instructions can be found [here](https://taskfile.dev/installation/).

## :gear: Setup & Configuration

Before building, you need to configure your credentials and format your SD card.

### Formatting the microSD Card

The microSD card must be formatted as **FAT32**.

> [!CAUTION]
> Formatting the card will erase all of its contents. Back up any important files before proceeding.

- **Windows:**
    1.  Insert the microSD card into your computer.
    2.  Open File Explorer, right-click on the SD card drive, and select **Format**.
    3.  Choose **FAT32** from the "File system" dropdown menu.
    4.  Click **Start**.

- **macOS:**
    1.  Insert the microSD card.
    2.  Open **Disk Utility**.
    3.  Select the SD card from the list on the left.
    4.  Click **Erase**.
    5.  Choose **MS-DOS (FAT)** from the "Format" dropdown.
    6.  Click **Erase**.

- **Linux:**
    1.  Insert the microSD card.
    2.  Open a terminal and run `lsblk` to identify the device name (e.g., `/dev/sdX`).
    3.  Unmount the card if it's auto-mounted: `sudo umount /dev/sdX*`.
    4.  Format the card: `sudo mkfs.vfat -F 32 /dev/sdX1` (assuming the partition is `/dev/sdX1`).

### Configure Credentials

1.  **Create `secrets.h`:** In the `include/` directory, create a file named `secrets.h`.
2.  **Add Credentials:** Copy the contents of `include/secrets.h.tmpl` into your new `secrets.h` file and update the values. This file contains the credentials for the Wi-Fi Manager Access Point and the FTP server.

    ```cpp
    #pragma once

    // WiFi Credentials
    #define WIFI_SSID "YourWiFiNetworkName"
    #define WIFI_PASSWORD "YourWiFiPassword"

    #define WIFI_AP_SSID "AutoConnectAP-T-Dongle"
    #define WIFI_AP_PASSWORD "password"

    #define FTP_USER "user"
    #define FTP_PASSWORD "password"
    ```
> [!NOTE]
> The `WIFI_SSID` and `WIFI_PASSWORD` fields are not currently used, as the device uses `WiFiManager` to handle Wi-Fi connections through a captive portal. They are included for potential future use.

## :rocket: Usage

The device boots into **USB Mass Storage (MSC) mode** by default. You can switch between modes by pressing the onboard button or by using the web API.

### :detective: Modes of Operation

- **USB Mass Storage Mode (Default):**
    1.  Plug the T-Dongle-S3 into your computer's USB port.
    2.  The device will be recognized as a USB Mass Storage device (thumb drive), giving you direct access to the microSD card.

- **FTP Server Mode:**
    1.  Press the onboard button to switch from MSC to FTP mode.
    2.  The device will connect to the configured Wi-Fi network. If no credentials are saved, it will create a Wi-Fi Access Point named "AutoConnectAP-T-Dongle".
    3.  Connect to this AP. If the captive portal does not open automatically, navigate to `http://192.168.4.1` in your web browser to configure Wi-Fi.
    4.  Use an FTP client to connect to the device's IP address (visible in the Serial Monitor) using the `FTP_USER` and `FTP_PASSWORD` you set in `include/secrets.h`.

### :art: LED Status Indicators

The onboard LED provides visual feedback on the device's status:

| Color  | Meaning                               |
| :----- | :------------------------------------ |
| Red    | Initializing on boot                  |
| Blue   | Connecting to Wi-Fi or in setup mode  |
| Green  | USB Mass Storage (MSC) mode active    |
| Orange | FTP mode active                       |

### :globe_with_meridians: Web API

The device hosts a simple web server with the following endpoints:

- **`GET /`**: Returns the current mode.
  ```json
  {"mode":"USB MSC"}
  ```
- **`POST /msc`**: Switches the device to MSC mode.
- **`POST /ftp`**: Switches the device to FTP mode.
- **`POST /restart`**: Restarts the device.

## :hammer_and_wrench: Building

This project uses a `Taskfile.yml` for common development tasks. After installing [Task](https://taskfile.dev/), you can run the following commands:

- **Build the project:**
  ```shell
  task build
  ```
- **Upload the firmware:**
  ```shell
  task upload
  ```
- **Monitor the serial output:**
  ```shell
  task monitor
  ```
- **Clean build files:**
  ```shell
  task clean
  ```
- **List all available tasks:**
  ```shell
  task -l
  ```

Alternatively, you can use the `platformio` CLI directly:

- **Build the project:**
  ```shell
  pio run
  ```
- **Upload the firmware:**
  ```shell
  pio run --target upload
  ```
- **Clean build files:**
  ```shell
  pio run --target clean
  ```
- **Monitor the serial output:**
  ```shell
  pio device monitor
  ```

## :white_check_mark: To Do

- [ ] Enable the LCD display to show:
    - Wi-Fi information in AP mode.
    - IP address in FTP mode.
    - The current mode name.
    - File count, used space percentage, and free space on the SD card in USB MSC mode.

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

## :pencil: Author

This project was started in 2025 by [Nicholas Wilde](https://github.com/nicholaswilde/).

[1]: <https://lilygo.cc/products/t-dongle-s3>
