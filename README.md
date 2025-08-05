# :signal_strength: FrameFi: Wireless Freedom for Digital Picture Frames :framed_picture:
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/test.yaml?label=test&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/test.yaml)
[![docs](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/docs.yaml?label=docs&style=for-the-badge&branch=main)](https://github.com/nicholaswilde/frame-fi/actions/workflows/docs.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/frame-fi/ci.yaml?label=ci&style=for-the-badge&branch=v0.1.0)](https://github.com/nicholaswilde/frame-fi/actions/workflows/ci.yaml)

FrameFi transforms a [LILYGO T-Dongle S3][1] into a versatile adapter for any digital picture frame. It enables you to remotely manage your photo library via FTP or access the SD card directly in USB Mass Storage mode.

---

## :book: Documentation

Documentation can be found [here][2].

---

## :stopwatch: TL;DR

- **Secrets:** Create `include/secrets.h` and update variables.

```shell
cp includes/secrets.h.tmpl includes/secrets.h
```

- **Computer:** Plug in the LILYGO T-Dongle S3 to a computer USB port while holding the button to put it into boot mode.

- **Upload Sketch:** Upload the sketch to the dongle.

```shell
task upload
```

or

```shell
pio run --target upload
```
- **Reboot the Device:** Unplug the dongle from your computer and plug it back in to reboot it.

- **Wi-Fi Credentials:** Connect to `AutoConnectAP-FrameFi` access point and enter Wi-Fi credentials.

>[!TIP]
>If the captive portal does not open automatically, navigate to http://192.168.4.1 in your web browser to configure Wi-Fi.

- **Digital Picture Frame:** Plug in dongle to digital picture frame.

- **Mode Switching:** A button or API call switches between FTP and USB Mass Storage modes.

```sh
curl -X POST http://<DEVICE_IP>/ftp
```

```sh
curl -X POST http://<DEVICE_IP>/msc
```

- **FTP Mode:** Upload pictures to the dongle via FTP using `lftp` or `scripts/sync.sh`.

```shell
lftp -c "
set ftp:ssl-allow no;
open -u '<FTP_USER>','<FTP_PASSWORD>' '<FTP_HOST>';
mirror -R --delete --verbose --parallel=1 '<REMOTE_DIR>' '<LOCAL_DIR>';
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
