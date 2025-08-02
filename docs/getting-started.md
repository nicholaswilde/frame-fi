---
tags:
  - getting-started
---
# :gear: Getting Started

Before building, you need to configure your credentials and format your SD card.

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
    This project uses `WiFiManager` to handle Wi-Fi connections via a captive portal, so you don't need to hardcode your network credentials. The `WIFI_SSID` and `WIFI_PASSWORD` fields in `secrets.h` are placeholders for a potential future feature and are not currently used.

### :lock: Secrets Management

This project uses [sops](https://github.com/getsops/sops) for encrypting and decrypting secrets. The following files are encrypted:

- `include/secrets.h`
- `scripts/.env`

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
    Before encrypting any files, you must add your GPG key's fingerprint or other public key information to the `.sops.yaml` file at the root of the project. This authorizes you to encrypt and decrypt the files.

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

### :art: Customization

#### :art: Theme

The LCD display uses the [Catppuccin](https://github.com/catppuccin/catppuccin) color theme. You can change the active theme by editing `include/catppuccin_colors.h`.

To change the theme, simply uncomment the block for the theme you wish to use and comment out the currently active theme.

**Example: Switching from Mocha to Macchiato**

1.  Open `include/catppuccin_colors.h`.
2.  Find the `Mocha` theme block and comment it out.
3.  Find the `Macchiato` theme block and uncomment it.
4.  Rebuild and upload the firmware.

!!! abstract "include/catppuccin_colors.h"

    ```cpp
    // --- Theme: Mocha (Now commented out) ---
    /*
    #define CATPPUCCIN_ROSEWATER  0xF73B
    ...
    */

    // --- Theme: Macchiato (Now active) ---
    #define CATPPUCCIN_ROSEWATER  0xF73B
    ...
    ```

??? abstract "include/catppuccin_colors.h"

    ```cpp
    --8<-- "catppuccin_colors.h"
    ```
