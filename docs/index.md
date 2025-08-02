---
tags:
  - frame-fi
---
# :signal_strength: FrameFi: Wireless Freedom for Digital Picture Frames :frame_photo:

[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/test.yaml)
[![docs](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/docs.yaml?label=docs&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/docs.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/ci.yaml?label=ci&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/ci.yaml)


FrameFi transforms a [LILYGO T-Dongle S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.


## :rocket: TL;DR

**Secrets:** Create `include/secrets.h` and update variables.

!!! code ""

    ```shell
    cp includes/secrets.h.tmpl includes/secrets.h
    ```

**Computer:** Plug in the LILYGO T-Dongle S3 to a computer USB port while holding the button to put it into boot mode.

**Upload Sketch:** Upload the sketch to the dongle.

!!! code ""

    === "Task"

        ```shell
        task upload
        ```

    === "PlatformIO"

        ```shell
        pio run --target upload
        ```

**Wi-Fi Credentials:** Connect to `AutoConnectAP-Frame-Fi` access point and enter WiFi credentials. If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.

**Digital Picture Frame:** Plug in dongle to digital picture frame.

**Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

!!! code ""

    === "FTP Mode"

        ```sh
        curl -X POST http://<DEVICE_IP>/ftp
        ```

    === "USB MSC Mode"

        ```sh
        curl -X POST http://<DEVICE_IP>/msc
        ```

**FTP Mode:** Upload pictures to the dongle via FTP using `lftp` or `scripts/sync.sh`.

!!! code ""

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
!!! warning
    FTP is an insecure protocol. Only use this feature on a trusted network.
- **USB Mass Storage Mode:** In MSC mode, the device mounts the microSD card as a standard USB thumb drive, allowing for high-speed file transfers directly from your computer.
- **Fast Data Transfer:** Utilizes the `SD_MMC` interface for the microSD card, offering significantly faster read/write speeds compared to the standard SPI interface.
- **Dynamic WiFi Configuration:** Uses `WiFiManager` to create a captive portal for easy Wi-Fi setup without hardcoding credentials.
- **Easy WiFi Reset:** Hold the button for 3 seconds to clear saved WiFi credentials and re-enter setup mode.
- **Boot-up Screen:** Displays a welcome screen with the current firmware version on startup.
- **LED Status Indicators:** A built-in LED provides at-a-glance status updates for different modes.
- **LCD Display:** Displays relevant information on the LCD display depending on the mode, including a bar graph showing SD card usage. It also utilizes catppuccin color schemes.

## :white_check_mark: To Do

- [x] Enable the LCD display to show:
    - Wi-Fi information in AP mode.
    - IP address in FTP mode.
    - The current mode name.
    - File count, used space percentage, and free space on the SD card in USB MSC mode. https://github.com/nicholaswilde/frame-fi/issues/7
- [x] Use hard-coded Wi-Fi credentials in addition to the captive portal. https://github.com/nicholaswilde/frame-fi/issues/9
- [x] Implement versioning and releasing of `bin` files via Github Actions. https://github.com/nicholaswilde/frame-fi/issues/8

## :scales: License

[Apache License 2.0](https://raw.githubusercontent.com/nicholaswilde/homelab/refs/heads/main/docs/LICENSE)

## :pencil:Author

This project was started in 2025 by [Nicholas Wilde][2].

## :link: References

- <https://docs.frame-fi.sh>
- <https://github.com/espressif/arduino-esp32>
- <https://github.com/Xinyuan-LilyGO/T-Dongle-S3>
- <https://github.com/i-am-shodan/USBArmyKnife>

[1]: <https://lilygo.cc/products/t-dongle-s3>
[2]: <https://github.com/nicholaswilde>
