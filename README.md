# :signal_strength: FrameFi: Wireless Freedom for Digital Picture Frames :framed_picture:
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/test.yaml)
[![docs](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/docs.yaml?label=docs&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/docs.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/ci.yaml?label=ci&style=for-the-badge&branch=v0.1.0)](https://github.com/nicholaswilde/frame-fi/actions/workflows/ci.yaml)

FrameFi transforms a [LILYGO T-Dongle S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

> [!WARNING]
> This project is currently in a `v0.X.X` development stage. Features and configurations are subject to change, and breaking changes may be introduced at any time.

---

## :book: Documentation

Documentation can be found [here][2].

---

## :stopwatch: TL;DR

- **Computer:** Plug in the LILYGO T-Dongle S3 to a computer USB port while holding the button to put it into boot mode.

- **Flash:** Execute the `flash.sh` script remotely from GitHub.

```shell
bash -c "$(curl -fsSL https://raw.githubusercontent.com/nicholaswilde/frame-fi/main/scripts/flash.sh)" _ /dev/ttyUSB0
```

> [!WARNING]
> Running a script directly from the internet with `bash -c "$(curl...)"` is a potential security risk. Always review the script's source code before executing it to ensure it is safe. You can view the script [here](https://github.com/nicholaswilde/frame-fi/blob/main/scripts/flash.sh).

- **Reboot the Device:** Unplug the dongle from your computer and plug it back in to reboot it.

- **Wi-Fi Credentials:** Connect to `FrameFi-<MAC>` access point and enter FTP, MQTT (optional), and Wi-Fi credentials.

> [!TIP]
> If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.

- **Digital Picture Frame:** Plug in dongle to digital picture frame.

- **Check Status:** Check the status of the device.

```sh
curl -X GET http://<DEVICE_IP>/
```

```sh
curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/
```

- **Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

```sh
curl -X POST http://<DEVICE_IP>/mode/ftp
```

```sh
curl -X POST http://<DEVICE_IP>/mode/msc
```

- **FTP Access:** Connect to the device with an FTP client using the IP on the display and credentials set in `WiFiManager` to upload files.

```shell
ftp <HOST>
```

```shell
put my-picture.png
```

- **FTP Mode:** Upload pictures to the dongle via FTP using `lftp` or `scripts/sync.sh`.

```shell
lftp -c "
set ftp:ssl-allow no;
open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
mirror -R --delete --verbose --only-missing --no-perms --parallel=1 '<REMOTE_DIR>' '<LOCAL_DIR>';
"
```

- **MQTT:** Publish a message to the MQTT broker to turn the display on or off.

```shell
mosquitto_pub -h <MQTT_BROKER_IP> -t "frame-fi/display/set" -m "ON"
```

---

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

---

## :pencil: Author

This project was started in 2025 by [Nicholas Wilde](https://github.com/nicholaswilde/).

[1]: <https://lilygo.cc/products/t-dongle-s3>
[2]: <https://nicholaswilde.io/frame-fi>
