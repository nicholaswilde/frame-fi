# :inbox_tray: FTP

Run an FTP server that can connect to the SD card.

Status: :ballot_box_with_check: `working`

## :pencil: Usage

Grab the IP address from the serial output.

```shell
lftp -u <username>,<password> <ftp_server_address> -e "ls -l; bye"
```

## :pencil: Notes

AP needs to be `WPA2-Personal` and not `WPA2/WPA3-Personal`.

`lib/SimpleFTPServer` needs to modified to work with `SD_MMC`.

## :link: References

- <https://github.com/xreef/SimpleFTPServer/tree/master/examples>
