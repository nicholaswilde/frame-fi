---
tags:
  - usage
  - display
---
# :desktop_computer: Display

The LCD display uses the [TFT_eSPI][1] library to show device status and network information. The content of the display changes depending on the current operating mode.

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

## :compass: Display Orientation

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

## :electric_plug: Enable/Disable LCD

You can completely enable or disable the LCD screen backlight by modifying the `platformio.ini` file. This is useful for saving power if you don't need the display.

!!! tip

    The screen can also be enabled and disabled via the [web API](api.md).

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

## :art: Theme

You can customize the color scheme of the display by selecting a [Catppuccin theme][2] in the `platformio.ini` file.

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

- <[TFT_eSPI][1]>
- <[Catppuccin][2]>

[1]: <https://github.com/Bodmer/TFT_eSPI>
[2]: <https://catppuccin.com/palette/>
