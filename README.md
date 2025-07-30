# :framed_picture: FrameFi: Wireless Adapter for Digital Picture Frames :signal_strength:
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)

FrameFi transforms a [LILYGO T-Dongle-S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

## :stopwatch: TL;DR

**Secrets:** Create `include/secrets.h` and update variables.

**Computer:** Plug in the LILYGO T-Dongle-S3 to a computer USB port while holding the button to put it into boot mode.

**Upload Sketch:** Upload the sketch to the LILYGO T-Dongle-S3.

**Wi-Fi Credentials:** Connect to `AutoCoonnectAP-Frame-Fi` access point and enter WiFi credentials.

**Digital Picture Frame:** Plug in dongle to digital picture frame.

**Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

**FTP Mode:** Upload pictures to the dongle via FTP.

## :sparkles: Features

- **Dual-Mode Operation:** Seamlessly switch between a USB Mass Storage (MSC) device and an FTP server.
- **Web Interface & API:** A built-in web server provides an API to check the device's status and switch between modes.
- **Wireless File Management:** In FTP mode, you can connect to the device over your Wi-Fi network to add, remove, and manage files on the microSD card.
> [!CAUTION]
> FTP is an insecure protocol. Only use this feature on a trusted network.
- **USB Mass Storage Mode:** In MSC mode, the device mounts the microSD card as a standard USB thumb drive, allowing for high-speed file transfers directly from your computer.
- **Fast Data Transfer:** Utilizes the `SD_MMC` interface for the microSD card, offering significantly faster read/write speeds compared to the standard SPI interface.
- **Dynamic WiFi Configuration:** Uses `WiFiManager` to create a captive portal for easy Wi-Fi setup without hardcoding credentials.
- **LED Status Indicators:** A built-in LED provides at-a-glance status updates for different modes.

## :electric_plug: Hardware Requirements

- **LILYGO T-Dongle-S3:** This project is specifically designed for this board.
- **microSD Card:** A FAT32 formatted microSD card is required to store pictures and files. The sketch has been tested with a 16GB card.

## :package: Software Dependencies

- **PlatformIO:** This project is built using the PlatformIO ecosystem. You can install it as a [VSCode Extension](https://platformio.org/install/ide?install=vscode) or use the [PlatformIO Core (CLI)](https://platformio.org/install/cli).
- **Task:** A task runner / build tool. Installation instructions can be found [here](https://taskfile.dev/installation/).

## :gear: Setup & Configuration

Before building, you need to configure your credentials and format your SD card.

<details>
<summary>Formatting the microSD Card</summary>

### :floppy_disk: Formatting the microSD Card

The microSD card must be formatted as **FAT32**.

> [!WARNING]
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
</details>

<details>
<summary>Configure Credentials</summary>

### :key: Configure Credentials

1.  **Create `secrets.h`:** In the `include/` directory, create a file named `secrets.h`.
2.  **Add Credentials:** Copy the contents of `include/secrets.h.tmpl` into your new `secrets.h` file and update the values. This file contains the credentials for the Wi-Fi Manager Access Point and the FTP server.

    ```cpp
    #pragma once

    // WiFi Credentials
    #define WIFI_SSID "YourWiFiNetworkName"
    #define WIFI_PASSWORD "YourWiFiPassword"

    #define WIFI_AP_SSID "AutoConnectAP-Frame-Fi"
    #define WIFI_AP_PASSWORD "password"

    #define FTP_USER "user"
    #define FTP_PASSWORD "password"
    ```
> [!NOTE]
> This project uses `WiFiManager` to handle Wi-Fi connections via a captive portal, so you don't need to hardcode your network credentials. The `WIFI_SSID` and `WIFI_PASSWORD` fields in `secrets.h` are placeholders for a potential future feature and are not currently used.
</details>

<details>
<summary>Secrets Management</summary>

### :lock: Secrets Management

This project uses [sops](https://github.com/getsops/sops) for encrypting and decrypting secrets. The following files are encrypted:

- `include/secrets.h`
- `scripts/.env`

#### Decrypting Secrets

To decrypt the files, run the following command:

```shell
sops -d include/secrets.h.enc > include/secrets.h
sops -d scripts/.env.enc > scripts/.env
```

#### Encrypting Secrets

To encrypt the files after making changes, run the following command:

```shell
sops -e include/secrets.h > include/secrets.h.enc
sops -e scripts/.env > scripts/.env.enc
```
</details>

## :rocket: Usage

The device boots into **USB Mass Storage (MSC) mode** by default. You can switch between modes by pressing the onboard button or by using the web API.

### :detective: Modes of Operation

- **USB Mass Storage Mode (Default):**
    1.  Plug the T-Dongle-S3 into your computer's USB port.
    2. The device will connect to the configured Wi-Fi network. If no credentials are saved, it will create a Wi-Fi Access Point named "AutoConnectAP-T-Dongle".
    3. Connect to this AP. If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.
    4.  The device will be recognized as a USB Mass Storage device (thumb drive), giving you direct access to the microSD card.

- **FTP Server Mode:**
    1.  Press the onboard button to switch from MSC to FTP mode.
    2.  Use an FTP client to connect to the device's IP address (visible in the Serial Monitor) using the `FTP_USER` and `FTP_PASSWORD` you set in `include/secrets.h`.
> [!CAUTION]
> FTP is an insecure protocol. Only use this feature on a trusted network.

<details>
<summary>LED Status Indicators</summary>

### :art: LED Status Indicators

The onboard LED provides visual feedback on the device's status:

| Color  | Meaning                               |
| :----: | :------------------------------------ |
| :red_circle:    | Initializing on boot                  |
| :large_blue_circle:   | Connecting to Wi-Fi or in setup mode  |
| :green_circle:  | USB Mass Storage (MSC) mode active    |
| :orange_circle: | FTP mode active                       |
</details>

<details>
<summary>Web API</summary>

### :globe_with_meridians: Web API

The device hosts a simple web server that allows you to check status and switch modes.

- **`GET /`**: Returns the current mode.
  ```sh
  curl -X GET http://<DEVICE_IP>/
  ```
  *Example Response:*
  ```json
  {"mode":"USB MSC"}
  ```

- **`POST /msc`**: Switches the device to USB Mass Storage (MSC) mode.
  ```sh
  curl -X POST http://<DEVICE_IP>/msc
  ```
  *Example Response:*
  ```json
  WIP
  ```

- **`POST /ftp`**: Switches the device to FTP mode.
  ```sh
  curl -X POST http://<DEVICE_IP>/ftp
  ```
  *Example Response:*
  ```json
  WIP
  ```

- **`POST /restart`**: Restarts the device.
  ```sh
  curl -X POST http://<DEVICE_IP>/restart
  ```
  *Example Response:*
  ```json
  WIP
  ```
</details>

<details>
<summary>Building</summary>

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
</details>

<details>
<summary>Flashing the Firmware</summary>

## :inbox_tray: Flashing the Firmware

If you don't want to build the project from source, you can flash a pre-compiled release directly to your device.

1.  **Download the Latest Release:**
    - Go to the [Releases page](https://github.com/nicholaswilde/frame-fi/releases).
    - Download the `LILYGO-T-Dongle-S3-Firmware-binaries.zip` file from the latest release.
    - Unzip the archive. It will contain `firmware.bin`, `partitions.bin`, and `bootloader.bin`.

2.  **Install esptool:**
    If you have PlatformIO installed, you already have `esptool.py`. If not, you can install it with pip:
    ```shell
    pip install esptool
    ```

3.  **Flash the Device:**
    - Put your T-Dongle-S3 into bootloader mode. You can usually do this by holding down the `BOOT` button (the one on the side), plugging it into your computer, and then releasing the button.
    - Find the serial port of your device. It will be something like `COM3` on Windows, `/dev/ttyUSB0` on Linux, or `/dev/cu.usbserial-XXXX` on macOS.
    - Run the following command, replacing `<YOUR_SERIAL_PORT>` with your device's port:
      ```shell
      esptool.py --chip esp32s3 --port <YOUR_SERIAL_PORT> --before default_reset --after hard_reset write_flash \
      0x0000 bootloader.bin \
      0x8000 partitions.bin \
      0x10000 firmware.bin
      ```

> [!TIP]
> If you have PlatformIO installed, you can use the `pio run --target upload` command, which handles the flashing process automatically.
</details>

<details>
<summary>Synchronizing Files</summary>

## :arrow_right_hook: Synchronizing Files

The `scripts/sync.sh` script provides an easy way to synchronize a local directory with the device's microSD card over FTP. It uses `lftp` to mirror the contents, deleting any files on the device that are not present locally.

### :package: Dependencies

You must have `lftp` installed on your system.

- **Debian/Ubuntu:**
  ```shell
  sudo apt install lftp
  ```
- **macOS (Homebrew):**
  ```shell
  brew install lftp
  ```

### :gear: Configuration

There are two ways to configure the script:

1.  **`.env` File (Recommended):**
    - Copy the template: `cp scripts/.env.tmpl scripts/.env`
    - Edit `scripts/.env` with your device's IP address and other settings.
      ```dotenv
      FTP_HOST="192.168.1.100"
      FTP_USER="user"
      FTP_PASSWORD="password"
      LOCAL_DIR="data"
      REMOTE_DIR="/"
      ```

2.  **Command-Line Arguments:**
    - You can override the `.env` file settings by passing environment variables directly.

### :pencil: Usage

1.  Make sure the device is in **FTP Server Mode**.
2.  Run the script from the project root:
    ```shell
    ./scripts/sync.sh
    ```

**Example with Command-Line Arguments:**

This command syncs a specific local directory to the device, overriding any settings in `.env`.

```shell
FTP_HOST="192.168.1.100" LOCAL_DIR="path/to/your/pictures" ./scripts/sync.sh
```
</details>

<details>
<summary>To Do</summary>

## :white_check_mark: To Do

- [ ] Enable the LCD display to show:
    - Wi-Fi information in AP mode.
    - IP address in FTP mode.
    - The current mode name.
    - File count, used space percentage, and free space on the SD card in USB MSC mode. https://github.com/nicholaswilde/frame-fi/issues/7
- [ ] Use hard-coded Wi-Fi credentials in addition to the captive portal. https://github.com/nicholaswilde/frame-fi/issues/9
- [ ] Implement versioning and releasing of `bin` files via Github Actions. https://github.com/nicholaswilde/frame-fi/issues/8
</details>

<details>
<summary>Inspiration</summary>

## :bulb: Inspiration 

This project was inspired by the following projects.

- <https://github.com/espressif/arduino-esp32>
- <https://github.com/Xinyuan-LilyGO/T-Dongle-S3>
- <https://github.com/i-am-shodan/USBArmyKnife>
</details>

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

## :pencil: Author

This project was started in 2025 by [Nicholas Wilde](https://github.com/nicholaswilde/).

[1]: <https://lilygo.cc/products/t-dongle-s3>
