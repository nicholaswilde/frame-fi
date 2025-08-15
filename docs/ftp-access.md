---
tags:
  - usage
  - ftp
---
# :inbox_tray: FTP Access

When the device is in **FTP Server Mode**, you can access the microSD card over the network using an FTP client.

!!! warning "Insecure Protocol"

    FTP is an inherently insecure protocol that transmits data, including credentials, in plain text. Only use this feature on a trusted, private network.

1.  **Switch to FTP Mode:**
    - Press the onboard button (single click) to switch from MSC to FTP mode.
    - Alternatively, use the web API by sending a `POST` request to `/mode/ftp`.

2.  **Connect with an FTP Client:**
    - Use any standard FTP client (e.g., [FileZilla][2], [WinSCP][3], or the command-line `ftp`).
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

    For automated synchronization, the `scripts/sync.sh` script uses `lftp` to mirror a local directory to the device. See the [Synchronizing Files](synchronizing-files.md) section for more details.

## :link: References

- [lftp][1]

[1]: <https://lftp.yar.ru/>
[2]: <https://filezilla-project.org/>
[3]: <https://winscp.net/>
