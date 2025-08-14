---
tags:
  - usage
---
# :pencil: Usage

The device boots into **USB Mass Storage (MSC) mode** by default. You can switch between modes by pressing the onboard button or by using the web API.

## :detective: Modes of Operation

- **USB Mass Storage Mode (Default):**
    1. Plug the T-Dongle-S3 into your computer's USB port.
    2. The device will connect to the configured Wi-Fi network. If no credentials are saved, it will go into AP mode.
    3. The device will be recognized as a USB Mass Storage device (thumb drive), giving you direct access to the microSD card.

- **AP Mode:**
    1. If the device has no saved Wi-Fi credentials, it will automatically start in AP mode.
    2. The device will create a Wi-Fi Access Point named `FrameFi-<MAC>`.
    3. Connect to this AP. If the captive portal does not open automatically, navigate to <http://192.168.4.1> in your web browser to configure Wi-Fi.

- **FTP Server Mode:**
    1. Press the onboard button (single click) to switch from MSC to FTP mode or use the web API.
    2. Use an FTP client to connect to the device's IP address (visible on the LCD display).

- **Reset Wi-Fi Settings:**
    1. Press and hold the onboard button for at least 3 seconds or use the web API.
    2. The device will clear its stored Wi-Fi credentials and restart.
    3. Follow the steps for the first-time Wi-Fi setup using the captive portal.

!!! warning "Insecure Protocol"

    FTP is an inherently insecure protocol that transmits data, including credentials, in plain text. Only use this feature on a trusted, private network.

## :inbox_tray: FTP Access

When the device is in **FTP Server Mode**, you can access the microSD card over the network using an FTP client.

1.  **Switch to FTP Mode:**
    - Press the onboard button (single click) to switch from MSC to FTP mode.
    - Alternatively, use the web API by sending a `POST` request to `/mode/ftp`.

2.  **Connect with an FTP Client:**
    - Use any standard FTP client (e.g., [FileZilla][8], [WinSCP][9], or the command-line `ftp`).
    - **Host:** The IP address of your device (shown on the LCD).
    - **Port:** `21` (the default FTP port).
    - **Username:** The username you configured in the WiFiManager setup page (default: `user`).
    - **Password:** The password you configured in the WiFiManager setup page (default: `password`).
  
- **Upload File:**

    1. **Open the Command Line:**
        1. **Windows:** Open the Command Prompt or PowerShell.
        2. **macOS or Linux:** Open the Terminal application.

     2. **Connect to the FTP Server:** Type the ftp command followed by the server address:

        !!! code ""
        
            ```shell
            ftp <HOST>
            ```

    3. **Enter Your Credentials:** The server will prompt you for your username and password.

    4. **List Remote Files (Optional):** You can list the files on the device by using the `ls` command:

        !!! code ""
        
            ```shell
            ls
            ```

    5. **Navigate to the Local Directory (Optional):** If the file you want to upload is not in your current local directory, you can change your local directory using the `lcd` (local change directory) command:

        !!! code ""
        
            ```shell
            lcd /path/to/data
            ```
    
    6. **Upload a Single File:** Use the `put` command followed by the name of the file you want to upload:

        !!! code ""
        
            ```shell
            put my-picture.png
            ```

!!! tip "Using lftp"

    For automated synchronization, the `scripts/sync.sh` script uses `lftp` to mirror a local directory to the device. See the [Synchronizing Files](#synchronizing-files) section for more details.

## :satellite: MQTT Integration

The device can connect to an MQTT broker to integrate with home automation platforms like Home Assistant. For a detailed guide, see the [Home Assistant Integration](home-assistant.md) page.

- **Enable/Disable MQTT:**
    You can enable or disable the MQTT client at runtime using the [Web API](#web-api) or by physically recompiling the firmware with the appropriate build flags.

    - **Via Web API (Recommended):**
        - Send a `POST` request to `/mqtt/enable` or `/mqtt/disable`.
        - This setting is saved to the device's configuration and will persist across reboots.

    - **Via Firmware Build Flags:**
        1.  **Open `platformio.ini`**: Open the `platformio.ini` file in the root of the project.
        2.  **Find `MQTT_ENABLED`**: Locate the `build_flags` section and find the `-D MQTT_ENABLED` line.
        3.  **Change the Value**:

        | Value | Description            |
        |:-----:|------------------------|
        | `1`   | Enable MQTT (Default)  |
        | `0`   | Disable MQTT           |

!!! abstract "platformio.ini"

    ```ini
    [env]
    ...
    build_flags =
      ...
      -D MQTT_ENABLED=1
    ```

- **Set MQTT Settings:**
    Configure your MQTT broker's IP address, port, and credentials in the WiFiManager setup page. See [Getting Started](getting-started.md#satellite-wi-fi-and-mqtt-setup) for more details.
    
- **Topics:**
    - **Status Topic:** `frame-fi/status` (publishes `USB MSC` or `Application (FTP Server)`)
    - **Command Topic:** `frame-fi/display/set` (accepts `ON` or `OFF` to control the display)

!!! warning
    MQTT is an insecure protocol. Only use this feature on a trusted network.

## :art: LED Status Indicators

The onboard LED provides visual feedback on the device's status:

| Color  | Meaning                               |
| :----: | :------------------------------------ |
| :yellow_circle:    | Initializing on boot                  |
| :blue_circle:   | Connecting to Wi-Fi or in setup mode  |
| :green_circle:  | USB Mass Storage (MSC) mode active    |
| :orange_circle: | FTP mode active                       |
| :purple_circle: | MQTT connected                        |

### :bulb: LED Brightness

You can adjust the brightness of the status LED by modifying the `platformio.ini` file.

1.  **Open `platformio.ini`**: Open the `platformio.ini` file in the root of the project.
2.  **Find `LED_BRIGHTNESS`**: Locate the `build_flags` section and find the `-D LED_BRIGHTNESS` line. If it doesn't exist, you can add it.
3.  **Change the Value**: Set the value to a number between `0` (off) and `255` (maximum brightness).
4.  **Rebuild and Upload**: Save the file, then rebuild and upload the firmware for the change to take effect.

!!! abstract "platformio.ini"

    ```ini
    [env]
    ...
    build_flags =
      ...
      -D LED_BRIGHTNESS=15
    ```

!!! note
    If the `LED_BRIGHTNESS` flag is not defined, the brightness will default to `13` (approximately 5%).

## :globe_with_meridians: Web API

The device hosts a simple web server that allows you to check status and switch modes.

!!! warning "Insecure Protocol"

    HTTP is an inherently insecure protocol that transmits data, including credentials, in plain text. Only use this feature on a trusted, private network.

### :key: Web Server Credentials

The web server can be protected by basic authentication. You can set the username and password in the WiFiManager setup page.

- **Connect with `curl`:**
    - Use the `-u` or `--user` flag to provide credentials.
    - If you omit the password, `curl` will prompt for it securely.

!!! tip

    To make the web server unauthenticated, make the `user` and `password` blank in the WiFiManager setup page.

### Web Server Commands

**`GET /`**: Returns the current mode, display status, and SD card information.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/
        ```

    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/
        ```

!!! success "Example Response"

    ```json
    {
      "mode": "USB MSC",
      "display": {
        "status": "on",
        "orientation": 1
      },
      "sd_card": {
        "total_size": 9876543210,
        "used_size": 1234567890,
        "free_size": 8641975320,
        "file_count": 42
      },
      "mqtt": {
        "enabled": true,
        "state": 0,
        "connected": true
      },
      "led": {
        "color": "green",
        "state": "on"
      }
    }
    ```

??? abstract "MQTT State"

    | State | Description                   | State | Description                   |
    |:-----:|-------------------------------|:-----:|-------------------------------|
    | `-4`  | MQTT_CONNECTION_TIMEOUT       |  `1`  | MQTT_CONNECT_BAD_PROTOCOL     |
    | `-3`  | MQTT_CONNECTION_LOST          |  `2`  | MQTT_CONNECT_BAD_CLIENT_ID    |
    | `-2`  | MQTT_CONNECT_FAILED           |  `3`  | MQTT_CONNECT_UNAVAILABLE      |  
    | `-1`  | MQTT_DISCONNECTED             |  `4`  | MQTT_CONNECT_BAD_CREDENTIALS  |
    |  `0`  | MQTT_CONNECTED                |  `5`  | MQTT_CONNECT_UNAUTHORIZED     |    

**`POST /mode/msc`**: Switches the device to USB Mass Storage (MSC) mode.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/msc
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mode/msc
        ```

!!! success "Example Responses"

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

**`POST /mode/ftp`**: Switches the device to FTP mode.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/ftp
        ```
        
    === "Authenticated"
        
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mode/ftp
        ```

!!! success "Example Responses"

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

**`POST /device/restart`**: Restarts the device.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/device/restart
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/device/restart
        ```

!!! success "Example Response:"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"Restarting device..."}
        ```

**`POST /display/toggle`**: Toggles the display on and off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/display/toggle
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/display/toggle
        ```

!!! success "Example Responses"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Display toggled on."}
        ```

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"Display toggled off."}
        ```

**`POST /display/on`**: Turns the display on.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/display/on
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/display/on
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Display turned on."}
        ```

**`POST /display/off`**: Turns the display off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/display/off
        ```
    
    === "Authenticated"
        
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/display/off
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Display turned off."}
        ```

**`POST /wifi/reset`**: Resets the Wi-Fi settings and restarts the device.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/wifi/reset
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/wifi/reset
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Resetting Wi-Fi and restarting..."}
        ```

!!! warning "Device Unreachable After Reset"

    After resetting the Wi-Fi settings, the device will restart and will no longer be connected to your Wi-Fi network. It will become unreachable at its previous IP address. You must reconnect to its Access Point (AP) to configure the new Wi-Fi credentials. See the [Modes of Operation](#detective-modes-of-operation) section for details on connecting to the AP.

**`POST /mqtt/enable`**: Enables the MQTT client.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mqtt/enable
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mqtt/enable
        ```

!!! success "Example Response"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT enabled."}
        ```

**`POST /mqtt/disable`**: Disables the MQTT client.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mqtt/disable
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mqtt/disable
        ```

!!! success "Example Response"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT disabled."}
        ```

**`POST /mqtt/toggle`**: Toggles the MQTT client on and off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mqtt/toggle
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mqtt/toggle
        ```

!!! success "Example Responses"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT enabled."}
        ```

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT disabled."}
        ```

**`GET /led/status`**: Returns the current LED color and state.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/led/status
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/led/status
        ```

!!! success "Example Response"

    ```json
    {
      "status": "success",
      "color": "green",
      "state": "on"
    }
    ```

**`POST /led/on`**: Turns the LED on.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/led/on
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/led/on
        ```

!!! success "Example Response"

    ```json
    {"status":"success","message":"LED turned on."}
    ```

**`POST /led/off`**: Turns the LED off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/led/off
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/led/off
        ```

!!! success "Example Response"

    ```json
    {"status":"success","message":"LED turned off."}
    ```

**`POST /led/toggle`**: Toggles the LED on and off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/led/toggle
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/led/toggle
        ```

!!! success "Example Responses"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"LED toggled on."}
        ```

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"LED toggled off."}
        ```

## :hammer_and_wrench: Building

This project uses a `Taskfile.yml` for common development tasks. After installing [Task][1], you can run the following commands.

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

!!! tip "Rebooting the Device"
    After uploading the firmware, you may need to unplug the dongle and plug it back in to reboot it and apply the changes.

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

## :inbox_tray: Flashing the Firmware

This guide provides instructions on how to flash the latest firmware to your device. You can use the automated `flash.sh` script for a streamlined experience, or follow the manual instructions for more control.

### :zap: Automated Flashing with `flash.sh`

The `flash.sh` script automates the process of downloading the latest release and flashing it to your device.

#### :warning: Prerequisites

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

#### :hammer_and_wrench: Usage

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

### :wrench: Manual Flashing

If you don't want to build the project from source, you can flash a pre-compiled release directly to your device.

- **Download the Latest Release:**
    - Go to the [Releases page][3].
    - Download the zip file from the [latest release][6].
    - Unzip the archive. It will contain `firmware.bin`, `partitions.bin`, and `bootloader.bin`.
    - Optionally, download the `boot_app0.bin` file from [espressive/arduino-esp32][2].

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

## :arrow_right_hook: Synchronizing Files

This guide provides instructions on how to sync files to your device. You can use the automated `sync.sh` script for a streamlined experience, or follow the manual instructions for more control.

### :zap: Automated Syncing with `sync.sh`

The `scripts/sync.sh` script provides an easy way to synchronize a local directory with the device's microSD card over FTP. It uses [lftp][4] to mirror the contents, deleting any files on the device that are not present locally.

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

- Edit `scripts/.env` with your device's IP address, FTP credentials, and web server credentials (if authentication is enabled).

!!! abstract "scripts/.env"

      ```ini
      --8<-- ".env.tmpl"
      ```

!!! tip

    You can override the `.env` file settings by passing environment variables directly.

#### :pencil: Script Usage

1. Make sure the device is in **FTP Server Mode**.
2. Run the script from the scripts directory:

!!! code ""

    === "Task"

        ```bash
        task sync
        ```

    === "Bash"
    
        ```shell
        cd scripts
        ./sync.sh
        ```

**Example with Command-Line Arguments:**

This command syncs a specific local directory to the device, overriding any settings in `.env`. If the web server is authenticated, you must also pass the credentials.

!!! code "./scripts directory"
    
    ```shell
    FTP_HOST="192.168.1.100" \
    WEB_SERVER_USER="admin" \
    WEB_SERVER_PASSWORD="password" \
    LOCAL_DIR="path/to/your/pictures" \
    ./sync.sh
    ```


### :wrench: Manual Syncing

If you don't want to use the `sync.sh` script, you can manually sync a directory using `lftp`.

!!! code ""

    ```shell
    lftp -c "
    set ftp:ssl-allow no;
    open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
    mirror -R --delete --verbose --only-missing --no-perms --parallel=1 '<LOCAL_DIR>' '<REMOTE_DIR>';
    "
    ```

## :desktop_computer: Display

The LCD display uses the [TFT_eSPI][5] library to show device status and network information. The content of the display changes depending on the current operating mode.

!!! example "Screens"

    === "Boot"
        When the device is booting, the device shows:
        
            Booting...

             version
        
    === "USB Mass Storage Mode"

        When in USB MSC mode, the display shows:

        - **Mode:** USB MSC
        - **IP:** The device's current IP address.
        - **MAC:** The device's MAC address.
        - **Size:** The total size of the microSD card.
        - **Files:** The number of files on the microSD card.
        - **Used:** The amount of used space on the microSD card.

    === "FTP Server Mode"

        When in FTP mode, the display shows:

        - **Mode:** FTP
        - **IP:** The device's current IP address.
        - **MAC:** The device's MAC address.
        - **Size:** The total size of the microSD card.
        - **Files:** The number of files on the microSD card.
        - **Used:** The amount of used space on the microSD card.

    === "AP Mode"

        When in Wi-Fi Access Point mode for configuration, the display shows:

        - **Mode:** AP
        - **IP:** The AP's IP address (usually `192.168.4.1`).
        - **MAC:** The device's MAC address.
        - **SSID:** The name of the Access Point (`AutoConnectAP-FrameFi`).

### :compass: Display Orientation

You can change the screen orientation by modifying the `platformio.ini` file.

1.  **Open `platformio.ini`**: Open the `platformio.ini` file in the root of the project.
2.  **Find `DISPLAY_ORIENTATION`**: Locate the `build_flags` section and find the `-D DISPLAY_ORIENTATION` line.
3.  **Change the Value**: Change the value to one of the following:

    | Value | Description         |
    |:-----:|---------------------|
    | `0`   | Portrait            |
    | `1`   | Landscape (Default) |
    | `2`   | Portrait Inverted   |
    | `3`   | Landscape Inverted  |

4.  **Rebuild and Upload**: Save the file, then rebuild and upload the firmware for the change to take effect.

!!! abstract "platformio.ini"

    ```ini
    [env]
    ...
    build_flags =
      ...
      -D DISPLAY_ORIENTATION=1
    ```

### :electric_plug: Enable/Disable LCD

You can completely enable or disable the LCD screen backlight by modifying the `platformio.ini` file. This is useful for saving power if you don't need the display.

!!! tip

    The screen can also be enabled and disabled via the [web API](#web-api).

1.  **Open `platformio.ini`**: Open the `platformio.ini` file in the root of the project.
2.  **Find `LCD_ENABLED`**: Locate the `build_flags` section and find the `-D LCD_ENABLED` line.
3.  **Change the Value**:

    | Value | Description               |
    |:-----:|---------------------------|
    | `1`   | Enable the LCD (Default)  |
    | `0`   | Disable the LCD           |
    
4.  **Rebuild and Upload**: Save the file, then rebuild and upload the firmware for the change to take effect.

!!! abstract "platformio.ini"

    ```ini
    [env]
    ...
    build_flags =
      ...
      -D LCD_ENABLED=1
    ```

### :art: Theme

You can customize the color scheme of the display by selecting a [Catppuccin theme][7] in the `platformio.ini` file.

1.  **Open `platformio.ini`**: Open the `platformio.ini` file in the root of the project.
2.  **Find `custom_catppuccin_theme`**: Locate the `[env]` section and find the `custom_catppuccin_theme` option.
3.  **Change the Value**: Change the value to one of the following:
    - `Mocha` (Default)
    - `Macchiato`
    - `Frappe`
    - `Latte`
4.  **Rebuild and Upload**: Save the file, then rebuild and upload the firmware for the change to take effect.

!!! abstract "platformio.ini"

    ```ini
    [env]
    ...
    custom_catppuccin_theme = Mocha
    ```

!!! note

    The file `catppuccin_colors.h` is automatically generated. Do not edit.

## :link: References

- [lftp][4]
- [TFT_eSPI][5]

[1]: <https://taskfile.dev/>
[2]: <https://github.com/espressif/arduino-esp32/tree/master/tools/partitions>
[3]: <https://github.com/nicholaswilde/frame-fi/releases>
[4]: <https://lftp.yar.ru/>
[5]: <https://github.com/Bodmer/TFT_eSPI>
[6]: <https://github.com/nicholaswilde/frame-fi/releases/latest>
[7]: <https://catppuccin.com/palette/>
[8]: <https://filezilla-project.org/>
[9]: <https://winscp.net/>
