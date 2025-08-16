---
tags:
  - usage
  - flashing
---
# :inbox_tray: Flashing the Firmware

This guide provides instructions on how to flash the latest firmware to your device. You can use the automated `flash.sh` script for a streamlined experience, or follow the manual instructions for more control.

## :zap: Automated Flashing with `flash.sh`

The `flash.sh` script automates the process of downloading the latest release and flashing it to your device.

### :warning: Prerequisites

Before you begin, ensure you have the following dependencies installed:

- `curl`: For downloading files from the internet.
- `grep`: For text searching.
- `unzip`: For extracting zip archives.
- `esptool`: A tool for communicating with Espressif chips.

You can typically install these using your system's package manager. For `esptool`, you can install it with pip:

!!! code ""

    ```shell
    pip install esptool
    ```

### :hammer_and_wrench: Usage

1.  **Connect your device:** Ensure your ESP32-S3 device is connected to your computer.

2.  **Run the script:** Execute the `flash.sh` script from the `scripts` directory or remotely from GitHub.

    !!! code ""

        === "Task  (Default Port)"

            ```
            task flash
            ```

        === "Local Execution (Default Port)"

            By default, the script uses `/dev/ttyACM0` as the serial port.

            ```shell
            ./scripts/flash.sh
            ```
        
        === "Local Execution (Custom Port)"

            If your device is on a different port, you can specify it as an argument.

            ```shell
            ./scripts/flash.sh /dev/ttyUSB0
            ```
        
        === "Remote Execution (Default Port)"

            You can run the script directly from GitHub without cloning the repository.

            ```shell
            bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/frame-fi/main/scripts/flash.sh)"
            ```

        === "Remote Execution (Custom Port)"

            To specify a custom port when running remotely, pass it as an argument after the script execution command.

            ```shell
            bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/frame-fi/main/scripts/flash.sh)" _ /dev/ttyUSB0
            ```

!!! warning "Security Risk"

    Running a script directly from the internet with `bash -c "$(curl...)"` is a potential security risk. Always review the script's source code before executing it to ensure it is safe. You can view the script [here](https://github.com/nicholaswilde/frame-fi/blob/main/scripts/flash.sh).

The script will then:

1.  Fetch the latest release from the [nicholaswilde/frame-fi](https://github.com/nicholaswilde/frame-fi) repository.
2.  Download the release archive to a temporary directory.
3.  Extract the necessary `.bin` files.
4.  Flash the firmware to your device using `esptool`.

## :wrench: Manual Flashing

If you don't want to build the project from source, you can flash a pre-compiled release directly to your device.

- **Download the Latest Release:**
    - Go to the [Releases page][2].
    - Download the zip file from the [latest release][1].
    - Unzip the archive. It will contain `firmware.bin`, `partitions.bin`, and `bootloader.bin`.
    - Optionally, download the `boot_app0.bin` file from [espressive/arduino-esp32][3].

!!! code ""

    ```shell
    wget https://github.com/espressif/arduino-esp32/raw/refs/heads/master/tools/partitions/boot_app0.bin
    ```

!!! warning
    The version of `boot_app0.bin` is critical. Using a version that is incompatible with your ESP32-S3's silicon revision can result in a soft-bricked device that is difficult to recover. The link provided is for the master branch of the `arduino-esp32` repository and should be compatible with most devices.

- **Install esptool:**
    If you have PlatformIO installed, you already have `esptool.py`. If not, you can install it with pip:

!!! code ""

    ```shell
    pip install esptool
    ```

- **Flash the Device:**
    - Put your T-Dongle-S3 into bootloader mode. You can usually do this by holding down the `BOOT` button (the one on the side), plugging it into your computer, and then releasing the button.
    - Find the serial port of your device. It will be something like `COM3` on Windows, `/dev/ttyUSB0` on Linux, or `/dev/cu.serial-XXXX` on macOS.
    - Run the following command, replacing `<YOUR_SERIAL_PORT>` with your device's port:

!!! code ""

    === "Without boot_app0"

        ```shell
        esptool.py \
            --chip esp32s3 \
            --port <YOUR_SERIAL_PORT> \
            --baud 921600 \
            --before default_reset \
            --after hard_reset \
            write_flash \
              -z \
              --flash_mode dio \
              --flash_freq 80m \
              --flash_size 16MB \
              0x0000 bootloader.bin \
              0x8000 partitions.bin \
              0x10000 firmware.bin
        ```

    === "With boot_app0"

        ```shell
        esptool.py \
            --chip esp32s3 \
            --port <YOUR_SERIAL_PORT> \
            --baud 921600 \
            --before default_reset \
            --after hard_reset \
            write_flash \
              -z \
              --flash_mode dio \
              --flash_freq 80m \
              --flash_size 16MB \
              0x0000 bootloader.bin \
              0xe000 boot_app0.bin \
              0x8000 partitions.bin \
              0x10000 firmware.bin
        ```

| Address | Bin File         |
|:-------:|------------------|
| 0x0000  | `bootloader.bin` |
| 0xe000  | `boot_app0.bin`  |
| 0x8000  | `partitions.bin` |
| 0x10000 | `firmware.bin`   |

- **Reboot the Device:**
    - After the flashing process is complete, unplug the dongle from your computer and plug it back in to reboot it.

!!! tip
    If you have PlatformIO installed, you can use the `pio run --target upload` command, which handles the flashing process automatically.

- **Flash with Web Installer (Recommended for beginners):**
    - Go to the [ESP Web Flasher][4] website.
    - Put your T-Dongle-S3 into bootloader mode. You can usually do this by holding down the `BOOT` button (the one on the side), plugging it into your computer, and then releasing the button.
    - Click the "Connect" button and select your device's serial port.
    - Select the `.bin` files you downloaded from the [latest release][1] and enter their corresponding addresses:

      | Address | Bin File         |
      |:-------:|------------------|
      | 0x0000  | `bootloader.bin` |
      | 0xe000  | `boot_app0.bin`  |
      | 0x8000  | `partitions.bin` |
      | 0x10000 | `firmware.bin`   |

    - Click "Install" to flash the firmware.

## :link: References

- <[FrameFi Latest Release][1]>
- <[FrameFi Releases][2]>
- <[espressif/arduino-esp32][3]>
- <[ESP Web Flasher][4]>

[1]: <https://github.com/nicholaswilde/frame-fi/releases/latest>
[2]: <https://github.com/nicholaswilde/frame-fi/releases>
[3]: <https://github.com/espressif/arduino-esp32/tree/master/tools/partitions>
[4]: <https://esp.huhn.me>
