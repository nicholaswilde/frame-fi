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

Run the automated API tests to verify the device's web API functionality.

!!! code ""

    === "Task"

        ```shell
        task test-api
        ```

    === "Bash"

        ```shell
        bash scripts/test-api.sh
        ```

## :link: References

- <[Task][1]>

[1]: <https://taskfile.dev/>
