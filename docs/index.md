---
tags:
  - frame-fi
---
# :signal_strength: FrameFi: Wireless Freedom for Digital Picture Frames :frame_photo:

[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/test.yaml)
[![docs](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/docs.yaml?label=docs&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/docs.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/ci.yaml?label=ci&style=for-the-badge&branch=v0.1.0)](https://github.com/nicholaswilde/frame-fi/actions/workflows/ci.yaml)


FrameFi transforms a <[LILYGO T-Dongle S3][1]> into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

!!! warning "Development Version"

    This project is currently in a `v0.X.X` development stage. Features and configurations are subject to change, and breaking changes may be introduced at any time.

## :rocket: TL;DR

- **Computer:** Plug in the LILYGO T-Dongle S3 to a computer USB port while holding the button to put it into boot mode.

- **Flash:** Execute the `flash.sh` script remotely from GitHub.

!!! code ""

    ```shell
    bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/frame-fi/main/scripts/flash.sh)" _ /dev/ttyUSB0
    ```

!!! warning "Security Risk"

    Running a script directly from the internet with `bash -c "$(curl...)"` is a potential security risk. Always review the script's source code before executing it to ensure it is safe. You can view the script [here](https://github.com/nicholaswilde/frame-fi/blob/main/scripts/flash.sh).

- **Reboot the Device:** Unplug the dongle from your computer and plug it back in to reboot it.

- **Wi-Fi Credentials:** Connect to `FrameFi-<MAC>` access point and enter FTP, MQTT (optional), and Wi-Fi credentials.

!!! tip

    If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.

- **Digital Picture Frame:** Plug in dongle to digital picture frame.

- **Check Status:** Check the status of the device.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/
        ```

- **Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

!!! code "FTP Mode"

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/ftp
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mode/ftp
        ```

!!! code "USB MSC Mode"

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/msc
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mode/msc
        ```

- **FTP Access:** Connect to the device with an FTP client using the IP on the display and credentials set in `WiFiManager` to upload files.

!!! code "Log into the device via FTP"

    ```shell
    ftp <HOST>
    ```

!!! code "Upload pictures to the device"

    ```shell
    put my-picture.png
    ```

- **FTP Mode:** Upload pictures to the dongle via FTP using `lftp` or `scripts/sync.sh`.

!!! code ""

    ```shell
    lftp -c "
    set ftp:ssl-allow no;
    open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
    mirror -R --delete --verbose --only-missing --no-perms --parallel=1 '<REMOTE_DIR>' '<LOCAL_DIR>';
    "
    ```

- **MQTT:** Publish a message to the MQTT broker to turn the display on or off.

!!! code ""

    ```shell
    mosquitto_pub -h <MQTT_BROKER_IP> -t "frame-fi/display/set" -m "ON"
    ```

## :sparkles: Features

- **:material-sync: Dual-Mode Operation:**
    - **FTP Server Mode:** Wirelessly manage files on the SD card over your Wi-Fi network.
    !!! warning
        FTP is an insecure protocol. Only use this feature on a trusted network.
    - **USB Mass Storage (MSC) Mode:** Connect directly to a computer for high-speed file transfers, treating the device like a standard USB drive.
- **:material-lan-connect: Connectivity & Remote Management:**
    - **Dynamic Wi-Fi Setup:** An initial captive portal (`WiFiManager`) allows for easy configuration of Wi-Fi, FTP, and MQTT credentials without hardcoding.
    - **REST API:** A built-in web server provides endpoints to check device status, switch modes, control the display, and restart the device.
    - **MQTT Integration:** Publish device status and control the display via MQTT, enabling seamless integration with Home Assistant or other automation platforms.
- **:material-palette: User Interface & Experience:**
    - **Onboard LCD Display:** A vibrant color display shows the current mode, network information (IP, MAC), and SD card usage with a visual bar graph. Styled with the <[Catppuccin][3]> color theme.
    - **Simple Physical Controls:**
        - A single button press toggles between FTP and MSC modes.
        - A long press (3 seconds) resets Wi-Fi and device configuration.
    - **LED Status Indicator:** An RGB LED provides at-a-glance feedback on the device's operational state (e.g., booting, connecting, FTP transfer).
- **:material-flash: Performance & Storage:**
    - **High-Speed SD Card Access:** Utilizes the 4-bit `SD_MMC` interface for significantly faster read/write performance compared to standard SPI.
    - **Persistent Configuration:** Device settings are saved to the internal flash storage (`LittleFS`), surviving reboots and power cycles.
- **:material-code-tags: Developer Friendly:**
    - **OTA Firmware Updates:** Flash new firmware remotely using a simple script.
    - **Serial Monitor Debugging:** Provides detailed operational logs over USB serial.

## :white_check_mark: To Do

- [ ] Use hard-coded Wi-Fi credentials in addition to the captive portal. ([#9](https://github.com/nicholaswilde/frame-fi/issues/9))

## :scales: License

[Apache License 2.0](https://raw.githubusercontent.com/nicholaswilde/homelab/refs/heads/main/docs/LICENSE)

## :pencil:Author

This project was started in 2025 by <[Nicholas Wilde][2]>.

## :link: References

- <https://docs.frame-fi.sh>
- <https://github.com/espressif/arduino-esp32>
- <https://github.com/Xinyuan-LilyGO/T-Dongle-S3>
- <https://github.com/i-am-shodan/USBArmyKnife>

[1]: <https://lilygo.cc/products/t-dongle-s3>
[2]: <https://github.com/nicholaswilde>
[3]: <https://github.com/catppuccin/catppuccin>
