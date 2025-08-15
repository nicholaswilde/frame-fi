---
tags:
  - usage
  - synchronizing
---
# :arrow_right_hook: Synchronizing Files

This guide provides instructions on how to sync files to your device. You can use the automated `sync.sh` script for a streamlined experience, or follow the manual instructions for more control.

## :zap: Automated Syncing with `sync.sh`

The `scripts/sync.sh` script provides an easy way to synchronize a local directory with the device's microSD card over FTP. It uses [lftp][1] to mirror the contents, deleting any files on the device that are not present locally.

### :package: Dependencies

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

### :gear: Configuration

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

### :pencil: Script Usage

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


## :wrench: Manual Syncing

If you don't want to use the `sync.sh` script, you can manually sync a directory using `lftp`.

!!! code ""

    ```shell
    lftp -c "
    set ftp:ssl-allow no;
    open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
    mirror -R --delete --verbose --only-missing --no-perms --parallel=1 '<LOCAL_DIR>' '<REMOTE_DIR>';
    "
    ```

## :link: References

- [lftp][1]

[1]: <https://lftp.yar.ru/>
