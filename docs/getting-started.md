---
tags:
  - getting-started
---
# :gear: Getting Started

Before building, you need to configure your credentials and format your SD card.

## :gear: Configuration

### :floppy_disk: Formatting the microSD Card

The microSD card must be formatted as **FAT32**.

!!! warning
    Formatting the card will erase all of its contents. Back up any important files before proceeding.

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

### :key: Configure Credentials

1.  **Create `secrets.h`:** In the `include/` directory, create a file named `secrets.h`.
2.  **Add Credentials:** Copy the contents of `include/secrets.h.tmpl` into your new `secrets.h` file and update the values. This file contains the credentials for the Wi-Fi Manager Access Point and the FTP server.

!!! abstract "include/secrets.h"

    ```cpp
    --8<-- "secrets.h.tmpl"
    ```
    
!!! note
    This project uses <[`WiFiManager`][1]> to handle Wi-Fi connections via a captive portal, so you don't need to hardcode your network credentials. The `WIFI_SSID` and `WIFI_PASSWORD` fields in `secrets.h` are placeholders for a potential future feature and are not currently used.

### :satellite: Wi-Fi and MQTT Setup

When you first boot the device, it will create a Wi-Fi Access Point (AP) that you can connect to from your computer or phone to configure its Wi-Fi connection.

!!! tip

    Configure FTP and MQTT settings first in the `Setup` page before configuring Wi-Fi to avoid terminating WiFiManager.

![param](./assets/images/main.png)

-   **SSID:** By default, the AP name is `FrameFi-<MAC>`, where `<MAC>` is the last 6 characters of the device's MAC address. This is configured by leaving `WIFI_AP_SSID` commented out in `secrets.h`. To set a custom SSID, uncomment `WIFI_AP_SSID` and provide your own name.
-   **Password:** By default, the AP is open and does not require a password. To set a password, uncomment `WIFI_AP_PASSWORD` in `secrets.h` and provide a password.

In the WiFiManager Configure WiFi page, you can also configure the following:

- **Wi-Fi Credentials:** Set the `SSID` and `password` for the Wi-Fi network the device is to connect to. 

![param](./assets/images/wifi.png)

In the WiFiManager setup page, you can also configure the following:

- **FTP Credentials:** Set the username and password for the FTP server. The values in `secrets.h` are used as the default values on the portal.
- **MQTT Settings:** Configure the MQTT broker, port, username, and password. The values in `secrets.h` are used as the default values on the portal.

!!! tip

    Hit the back button twice to get back to the main page after saving FTP and MQTT settings.
    
![param](./assets/images/param.png)

### :lock: Secrets Management

This project uses <[sops][2]> for encrypting and decrypting secrets. The following files are encrypted:

- `include/secrets.h`
- `scripts/.env`

The above files are ignored by git to prevent the accidental comitting of secrets.

`*.enc` files are saved encrypted secrets for making it easier to save credentials.

#### Decrypting Secrets

To decrypt the files, run the following command:

!!! code ""

    === "Task"
    
        ```shell
        task decrypt
        ```
        
    === "SOPS"
    
        ```shell
        sops -d include/secrets.h.enc > include/secrets.h
        sops -d --input-type dotenv --output-type dotenv scripts/.env.enc > scripts/.env
        ```

#### Encrypting Secrets

!!! note
    Before encrypting any files, you must add your GPG key's fingerprint or other public key information to the <[`.sops.yaml`][3]> file at the root of the project. This authorizes you to encrypt and decrypt the files.

To encrypt the files after making changes, run the following command:

!!! code ""

    === "Task"
    
        ```shell
        task encrypt
        ```
        
    === "SOPS"
    
        ```shell
        sops -e include/secrets.h > include/secrets.h.enc
        sops -e --input-type dotenv --output-type dotenv scripts/.env > scripts/.env.enc
        ```

## :link: References

- <https://github.com/catppuccin/catppuccin>
- <https://github.com/getsops/sops>
- <https://github.com/tzapu/WiFiManager>

[1]: <https://github.com/tzapu/WiFiManager>
[2]: <https://github.com/getsops/sops>
[3]: <https://getsops.io/docs/#using-sopsyaml-conf-to-select-kms-pgp-and-age-for-new-files>
