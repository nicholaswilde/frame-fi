# LILYGO T-Dongle S3 Wireless Digital Picture Frame Adapter

This project transforms a [LILYGO T-Dongle-S3][1] into a wireless adapter for a digital picture frame, enabling you to
upload pictures and manage files remotely. It offers two modes of operation: an SFTP server for wireless file
transfers and a USB Mass Storage mode for direct computer access.

## :sparkles: Features

- **Wireless File Management:** Start the device in SFTP mode to connect to it over your Wi-Fi network. Add, remove,
  and manage files on the microSD card using an SFTP client like FileZilla or Cyberduck.
- **USB Mass Storage Mode:** By holding the BOOT button on startup, the device mounts the microSD card as a standard
  USB thumb drive, allowing for high-speed file transfers directly from your computer.
- **Fast Data Transfer:** Utilizes the `SD_MMC` interface for the microSD card, offering significantly faster
  read/write speeds compared to the standard SPI interface.

## :electric_plug: Hardware Requirements

- **LILYGO T-Dongle-S3:** This project is specifically designed for this board.
- **microSD Card:** A FAT32 formatted microSD card is required to store pictures and files.

> [!WARNING]
> This sketch is configured to use the `SD_MMC` library, which requires the microSD card slot to be wired to the
> ESP32-S3's dedicated SD_MMC pins. Most T-Dongle-S3 boards have the integrated slot wired for SPI only. This code
> will likely fail to initialize the SD card unless you have custom hardware that routes the SD card to the correct
> SD_MMC pins as defined in `pin_config.h`.

## :floppy_disk: Software Dependencies

- **Arduino IDE** or **Arduino CLI**
- **ESP32 Board Package:** Ensure you have the latest version installed.
- **SimpleFTPServer Library:** Install this from the Arduino Library Manager.

## :gear: Setup & Configuration

1.  **Install Dependencies:** Make sure all the software dependencies listed above are installed.
2.  **Board Configuration:** In the Arduino IDE or your `sketch.yaml` file, select the following settings:
    - **Board:** `ESP32S3 Dev Module`
    - **USB Mode:** `Hardware CDC and JTAG`
3.  **Configure Credentials:** Open the `digital-picture-frame.ino` file and update the following variables with your
    information:
    ```cpp
    // --- User Configuration ---
    const char* ssid = "YOUR_WIFI_SSID";       // Your Wi-Fi network SSID
    const char* password = "YOUR_WIFI_PASSWORD"; // Your Wi-Fi network password
    const char* ftp_user = "esp32";             // SFTP username
    const char* ftp_pass = "esp32";             // SFTP password
    // --- End of User Configuration ---
    ```
4.  **Insert microSD Card:** Insert a FAT32 formatted microSD card into the T-Dongle-S3.

## :rocket: Usage

The device's mode is determined by the state of the BOOT button (GPIO0) on startup.

- **SFTP Server Mode (Wireless):**
    1.  Power on the T-Dongle-S3 **without** pressing the BOOT button.
    2.  The device will connect to the configured Wi-Fi network.
    3.  Use an SFTP client to connect to the device's IP address (visible in the Serial Monitor) with the username and
        password you configured.

- **USB Mass Storage Mode (Wired):**
    1.  Press and hold the BOOT button.
    2.  While holding the button, plug the T-Dongle-S3 into your computer's USB port.
    3.  The device will be recognized as a USB Mass Storage device (thumb drive), giving you direct access to the
        microSD card.

## :hammer_and_wrench: Building

This project uses `Taskfile.yml` for common development tasks. You can use the [Task](https://taskfile.dev/) build
tool to execute them.

- **Compile the sketch:**
  ```shell
  task compile
  ```
- **List connected boards:**
  ```shell
  task list
  ```
- **Update the core index:**
  ```shell
  task update
  ```

Alternatively, you can use the `arduino-cli` directly:

```shell
# Compile the sketch
arduino-cli compile

# Upload the sketch (replace /dev/ttyACM0 with your board's port)
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32s3
```

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

## :pencil: Author

This project was started in 2025 by [Nicholas Wilde](https://github.com/nicholaswilde/).

[1]: <https://lilygo.cc/products/t-dongle-s3>
