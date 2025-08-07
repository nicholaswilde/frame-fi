---
tags:
  - frame-fi
---
# :signal_strength: FrameFi: Wireless Freedom for Digital Picture Frames :frame_photo:

[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/test.yaml)
[![docs](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/docs.yaml?label=docs&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/docs.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/ci.yaml?label=ci&style=for-the-badge&branch=v0.1.0)](https://github.com/nicholaswilde/frame-fi/actions/workflows/ci.yaml)


FrameFi transforms a [LILYGO T-Dongle S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

!!! warning "Development Version"
    This project is currently in a `v0.X.X` development stage. Features and configurations are subject to change, and breaking changes may be introduced at any time.

## :rocket: TL;DR

- **Secrets:** Create `include/secrets.h` and update variables.

!!! code ""

    ```shell
    cp includes/secrets.h.tmpl includes/secrets.h
    ```

- **Computer:** Plug in the LILYGO T-Dongle S3 to a computer USB port while holding the button to put it into boot mode.

- **Upload Sketch:** Upload the sketch to the dongle.

!!! code ""

    === "Task"

        ```shell
        task upload
        ```

    === "PlatformIO"

        ```shell
        pio run --target upload
        ```

- **Reboot the Device:** Unplug the dongle from your computer and plug it back in to reboot it.

- **Wi-Fi Credentials:** Connect to `FrameFi-<MAC>` access point and enter Wi-Fi credentials.

!!! tip

    If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.

- **Digital Picture Frame:** Plug in dongle to digital picture frame.

- **Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

!!! code ""

    === "FTP Mode"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/ftp
        ```

    === "USB MSC Mode"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/msc
        ```

- **FTP Access:** Connect to the device with an FTP client using the IP on the display and credentials from `include/secrets.h` to upload files.

- **FTP Mode:** Upload pictures to the dongle via FTP using `lftp` or `scripts/sync.sh`.

!!! code ""

    ```shell
    lftp -c "
    set ftp:ssl-allow no;
    open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
    mirror -R --delete --verbose --parallel=1 '<REMOTE_DIR>' '<LOCAL_DIR>';
    "
    ```

- **MQTT:** Publish a message to the MQTT broker to turn the display on or off.

!!! code ""

    ```shell
    mosquitto_pub -h <MQTT_BROKER_IP> -t "frame-fi/display/set" -m "ON"
    ```

## :sparkles: Features

- **Dual-Mode Operation:** Seamlessly switch between a USB Mass Storage (MSC) device and an FTP server.
- **Web Interface & API:** A built-in web server provides an API to check the device's status and switch between modes.
- **Wireless File Management:** In FTP mode, you can connect to the device over your Wi-Fi network to add, remove, and manage files on the microSD card.
!!! warning
    FTP is an insecure protocol. Only use this feature on a trusted network.
- **USB Mass Storage Mode:** In MSC mode, the device mounts the microSD card as a standard USB thumb drive, allowing for high-speed file transfers directly from your computer.
- **Fast Data Transfer:** Utilizes the `SD_MMC` interface for the microSD card, offering significantly faster read/write speeds compared to the standard SPI interface.
- **Dynamic Wi-Fi Configuration:** Uses `WiFiManager` to create a captive portal for easy Wi-Fi setup without hardcoding credentials.
- **Home Assistant Integration:** Publishes status and accepts commands via MQTT for seamless integration with home automation systems.
- **Easy Wi-Fi Reset:** Hold the button for 3 seconds to clear saved Wi-Fi credentials and re-enter setup mode.
- **Boot-up Screen:** Displays a welcome screen with the current firmware version on startup.
- **LED Status Indicators:** A built-in LED provides at-a-glance status updates for different modes.
- **LCD Display:** Displays relevant information on the LCD display depending on the mode, including a bar graph showing SD card usage. It also utilizes catppuccin color schemes.

## :white_check_mark: To Do

- [ ] Use hard-coded Wi-Fi credentials in addition to the captive portal. ([#9](https://github.com/nicholaswilde/frame-fi/issues/9))

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
