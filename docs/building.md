---
tags:
  - usage
  - building
---
# :hammer_and_wrench: Building

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

## :test_tube: Testing the API

The `test-api.sh` script automates testing the device's web API functionality by performing various requests and verifying the responses.

### :package: Dependencies

You must have the following dependencies installed on your system:

- `curl`: For interacting with web services.
- `jq`: A lightweight and flexible command-line JSON processor.

!!! code ""

    === "Debian/Ubuntu"

        ```shell
        sudo apt install curl jq
        ```

    === "macOS (Homebrew)"

        ```shell
        brew install curl jq
        ```

### :gear: Configuration

The script requires an `.env` file with the device's IP address.

1.  **Copy the template `.env` File:**

    !!! code ""

        ```shell
        cp scripts/.env.tmpl scripts/.env
        ```

2.  **Edit `scripts/.env`:** Update the `FTP_HOST` variable with your device's IP address.

!!! abstract "scripts/.env"

    ```ini
    --8<-- ".env.tmpl"
    ```

### :pencil: Script Usage

1.  Ensure the device is online and accessible at the configured IP address.
2.  Run the script from the scripts directory:

!!! code ""

    === "Task"

        ```bash
        task test-api
        ```

    === "Bash"

        ```shell
        cd scripts
        ./test-api.sh
        ```

??? abstract "test-api.sh"

    ```bash
    --8<-- "scripts/test-api.sh"
    ```

## :test_tube: Testing the FTP Server

The `test-ftp.sh` script automates testing the FTP server functionality by uploading, downloading, and verifying a test file.

### :package: Dependencies

You must have the following dependencies installed on your system:

- `curl`: For interacting with web services (used to check device status).
- `jq`: A lightweight and flexible command-line JSON processor.
- `lftp`: A sophisticated file transfer program.

!!! code ""

    === "Debian/Ubuntu"

        ```shell
        sudo apt install curl jq lftp
        ```

    === "macOS (Homebrew)"

        ```shell
        brew install curl jq lftp
        ```

### :gear: Configuration

The script requires an `.env` file with the device's FTP host, username, and password.

1.  **Copy the template `.env` File:**

    !!! code ""

        ```shell
        cp scripts/.env.tmpl scripts/.env
        ```

2.  **Edit `scripts/.env`:** Update the `FTP_HOST`, `FTP_USER`, and `FTP_PASSWORD` variables with your device's credentials.

!!! abstract "scripts/.env"

    ```ini
    --8<-- ".env.tmpl"
    ```

### :pencil: Script Usage

1.  Ensure the device is in **FTP Server Mode**.
2.  Run the script from the scripts directory:

!!! code ""

    === "Task"

        ```bash
        task test-ftp
        ```

    === "Bash"

        ```shell
        cd scripts
        ./test-ftp.sh
        ```

??? abstract "test-ftp.sh"

    ```bash
    --8<-- "scripts/test-ftp.sh"
    ```

## :link: References

- [Task][1]

[1]: <https://taskfile.dev/>