---
tags:
  - usage
---
# :detective: Modes of Operation

The device boots into **USB Mass Storage (MSC) mode** by default. You can switch between modes by pressing the onboard button or by using the web API.

- **USB Mass Storage Mode (Default):**
    1. Plug the T-Dongle-S3 into your computer's USB port.
    2. The device will connect to the configured Wi-Fi network. If no credentials are saved, it will go into AP mode.
    3. The device will be recognized as a USB Mass Storage device (thumb drive), giving you direct access to the microSD card.

- **AP Mode:**
    1. If the device has no saved Wi-Fi credentials, it will automatically start in AP mode.
    2. The device will create a Wi-Fi Access Point named `FrameFi-<MAC>`.
    3. Connect to this AP. If the captive portal does not open automatically, navigate to <http://192.168.4.1> in your web browser to configure Wi-Fi.

- **FTP Server Mode:**
    1. Press the onboard button (single click) to switch from MSC to FTP mode or use the web API.
    2. Use an FTP client to connect to the device's IP address (visible on the LCD display).

- **Reset Wi-Fi Settings:**
    1. Press and hold the onboard button for at least 3 seconds or use the web API.
    2. The device will clear its stored Wi-Fi credentials and restart.
    3. Follow the steps for the first-time Wi-Fi setup using the captive portal.

## :link: References