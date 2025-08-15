---
tags:
  - usage
  - led
---
# :art: LED Status Indicators

The onboard LED provides visual feedback on the device's status:

| Color  | Meaning                               |
| :----: | :------------------------------------ |
| :yellow_circle:    | Initializing on boot                  |
| :blue_circle:   | Connecting to Wi-Fi or in setup mode  |
| :green_circle:  | USB Mass Storage (MSC) mode active    |
| :orange_circle: | FTP mode active                       |
| :purple_circle: | MQTT connected                        |

## :bulb: LED Brightness

You can adjust the brightness of the status LED in two ways:

- **Via Web API (Recommended):**
    - Send a `POST` request to `/led/brightness` with a plain text integer between `0` and `255` in the request body.
    - This setting is saved to the device's configuration and will persist across reboots.

- **Via Firmware Build Flags:**
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
    If the `LED_BRIGHTNESS` flag is not defined, the brightness will default to `13` (approximately 5%). The value set via the API will override the build flag value after the first API call.
