---
tags:
  - Usage
---
# :pencil: Usage

The device boots into **USB Mass Storage (MSC) mode** by default. You can switch between modes by pressing the onboard button or by using the web API.

### :detective: Modes of Operation

- **USB Mass Storage Mode (Default):**
    1.  Plug the T-Dongle-S3 into your computer's USB port.
    2. The device will connect to the configured Wi-Fi network. If no credentials are saved, it will create a Wi-Fi Access Point named "AutoConnectAP-Frame-Fi".
    3. Connect to this AP. If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.
    4.  The device will be recognized as a USB Mass Storage device (thumb drive), giving you direct access to the microSD card.

- **FTP Server Mode:**
    1.  Press the onboard button (single click) to switch from MSC to FTP mode.
    2.  Use an FTP client to connect to the device's IP address (visible on the LCD display) using the `FTP_USER` and `FTP_PASSWORD` you set in `include/secrets.h`.

- **Reset WiFi Settings:**
    1. Press and hold the onboard button for 3 seconds.
    2. The device will clear its stored WiFi credentials and restart.
    3. Follow the steps for the first-time WiFi setup using the captive portal.

!!! warning
    FTP is an insecure protocol. Only use this feature on a trusted network.

### :art: LED Status Indicators

The onboard LED provides visual feedback on the device's status:

| Color  | Meaning                               |
| :----: | :------------------------------------ |
| :yellow_circle:    | Initializing on boot                  |
| :blue_circle:   | Connecting to Wi-Fi or in setup mode  |
| :green_circle:  | USB Mass Storage (MSC) mode active    |
| :orange_circle: | FTP mode active                       |

### :globe_with_meridians: Web API

The device hosts a simple web server that allows you to check status and switch modes.

**`GET /`**: Returns the current mode.

!!! code ""

    ```sh
    curl -X GET http://<DEVICE_IP>/
    ```

!!! code "Example Response"

    ```json
    {"mode":"USB MSC"}
    ```

**`POST /msc`**: Switches the device to USB Mass Storage (MSC) mode.

!!! code ""

    ```sh
    curl -X POST http://<DEVICE_IP>/msc
    ```

!!! code "Example Responses"

    === "Success (200 OK)"
        ```json
        {"status":"success","message":"Switched to MSC mode."}
        ```

    === "No Change (200 OK)"
        ```json
        {"status":"no_change","message":"Already in MSC mode."}
        ```

    === "Error (500 Internal Server Error)"
        ```json
        {"status":"error","message":"Failed to switch to MSC mode."}
        ```

**`POST /ftp`**: Switches the device to FTP mode.

!!! code ""

    ```sh
    curl -X POST http://<DEVICE_IP>/ftp
    ```

!!! code "Example Responses"

    === "Success (200 OK)"
        ```json
        {"status":"success","message":"Switched to Application (FTP) mode."}
        ```

    === "No Change (200 OK)"

        ```json
        {"status":"no_change","message":"Already in Application (FTP) mode."}
        ```

    === "Error (500 Internal Server Error)"

        ```json
        {"status":"error","message":"Failed to re-initialize SD card."}
        ```

**`POST /restart`**: Restarts the device.

!!! code ""

    ```sh
    curl -X POST http://<DEVICE_IP>/restart
    ```
    
!!! code "Example Response:"

    === "Success (200 OK)"
        ```json
        {"status":"success","message":"Restarting device..."}
        ```

### :hammer_and_wrench: Building

This project uses a `Taskfile.yml` for common development tasks. After installing [Task](https://taskfile.dev/), you can run the following commands.

**Build the project:**

!!! code ""

    === "Task"
        ```shell
        task build
        ```

    === "PlatformIO"
        ```shell
        pio run
        ```

**Upload the firmware:**

!!! code ""

    === "Task"

        ```shell
        task upload
        ```
  
    === "PlatformIO"
        ```shell
        pio run --target upload
        ```

**Monitor the serial output:**
!!! code ""
    === "Task"
        ```shell
        task monitor
        ```
    === "PlatformIO"
        ```shell
        pio device monitor
        ```

**Clean build files:**
!!! code ""
    === "Task"
        ```shell
        task clean
        ```

    === "PlatformIO"
        ```shell
        pio run --target clean
        ```

**List all available tasks:**

!!! code ""

    === "Task"
    
        ```shell
        task -l
        ```

### :inbox_tray: Flashing the Firmware

If you don't want to build the project from source, you can flash a pre-compiled release directly to your device.

1.  **Download the Latest Release:**
    - Go to the [Releases page](https://github.com/nicholaswilde/frame-fi/releases).
    - Download the `LILYGO-T-Dongle-S3-Firmware-binaries.zip` file from the latest release.
    - Unzip the archive. It will contain `firmware.bin`, `partitions.bin`, and `bootloader.bin`.

2.  **Install esptool:**
    If you have PlatformIO installed, you already have `esptool.py`. If not, you can install it with pip:

!!! code ""

    ```shell
    pip install esptool
    ```

4.  **Flash the Device:**
    - Put your T-Dongle-S3 into bootloader mode. You can usually do this by holding down the `BOOT` button (the one on the side), plugging it into your computer, and then releasing the button.
    - Find the serial port of your device. It will be something like `COM3` on Windows, `/dev/ttyUSB0` on Linux, or `/dev/cu.usbserial-XXXX` on macOS.
    - Run the following command, replacing `<YOUR_SERIAL_PORT>` with your device's port:

!!! code ""

      ```shell
      esptool.py \
          --chip esp32s3 \
          --port <YOUR_SERIAL_PORT> \
          --before default_reset \
          --after hard_reset \
          write_flash \
          0x0000 bootloader.bin \
          0x8000 partitions.bin \
          0x10000 firmware.bin
      ```

!!! tip
    If you have PlatformIO installed, you can use the `pio run --target upload` command, which handles the flashing process automatically.

### :arrow_right_hook: Synchronizing Files

The `scripts/sync.sh` script provides an easy way to synchronize a local directory with the device's microSD card over FTP. It uses [lftp](https://lftp.yar.ru/) to mirror the contents, deleting any files on the device that are not present locally.

#### :package: Dependencies

You must have `lftp` installed on your system.

!!! code ""

    === "Debian/Ubuntu"
        ```shell
        sudo apt install lftp
        ```

    === "macOS (Homebrew)"
        ```shell
        brew install lftp
        ```

#### :gear: Configuration

There are two ways to configure the script:

- Copy the template `.env` File (Recommended):

!!! code ""

    ```shell
    cp scripts/.env.tmpl scripts/.env
    ```
    
- Edit `scripts/.env` with your device's IP address and other settings.

!!! abstract "scripts/.env"

      ```ini
      --8<-- ".env.tmpl"
      ```

!!! tip

    You can override the `.env` file settings by passing environment variables directly.

#### :pencil: Usage

1.  Make sure the device is in **FTP Server Mode**.
2.  Run the script from the scripts directory:

!!! code "./scripts directory"

    ```shell
    ./sync.sh
    ```

**Example with Command-Line Arguments:**

This command syncs a specific local directory to the device, overriding any settings in `.env`.

!!! code "./scripts directory"

    ```shell
    FTP_HOST="192.168.1.100" LOCAL_DIR="path/to/your/pictures" ./sync.sh
    ```
