# :signal_strength: FrameFi Project Overview

This document provides a comprehensive overview of the FrameFi project, serving as instructional context for future interactions.

## :memo: Project Overview

FrameFi transforms a LILYGO T-Dongle S3 (an ESP32-S3 based development board) into a versatile adapter for digital picture frames. Its primary purpose is to enable remote management of photo libraries via an FTP server or direct access to the SD card in USB Mass Storage (MSC) mode. The project is built using the PlatformIO ecosystem for embedded development with the Arduino framework.

**Key Technologies:**

*   **Hardware:** LILYGO T-Dongle S3 (ESP32-S3 microcontroller)
*   **Development Environment:** PlatformIO
*   **Programming Language:** C++ (using the Arduino framework)
*   **Networking:**
    *   `WiFiManager`: For dynamic Wi-Fi configuration via a captive portal.
    *   `WebServer`: Provides a REST API for device control and status.
    *   `PubSubClient`: Enables MQTT communication for Home Assistant integration.
    *   `SimpleFTPServer`: Implements the FTP server for file management.
*   **Storage:**
    *   `SD_MMC`: High-speed SD card interface.
    *   `USBMSC`: Implements USB Mass Storage functionality.
*   **Display:** `TFT_eSPI`: Library for controlling the integrated LCD display.
*   **Build Automation:** `Taskfile` (using the `task` command runner) for streamlined development workflows.
*   **Secrets Management:** `sops` for encrypting sensitive configuration files.

## :hammer_and_wrench: Building and Running

### :key: Initial Setup & Secrets Management

1.  **Install Dependencies:** Ensure you have `PlatformIO`, `task` (Taskfile runner), and `sops` installed on your system.
2.  **Prepare Secrets:**
    ```shell
    cp include/secrets.h.tmpl include/secrets.h
    cp scripts/.env.tmpl scripts/.env
    ```
3.  **Encrypt Secrets (after modification):**
    ```shell
    task encrypt
    ```
    *Note: Before encrypting, ensure your GPG key fingerprint is added to `.sops.yaml`.*
4.  **Decrypt Secrets (for local development):**
    ```shell
    task decrypt
    ```

### :gear: Build and Upload

*   **Build Firmware:**
    ```shell
    task build
    # or
    pio run
    ```
*   **Upload Firmware to Device:**
    ```shell
    task upload
    # or
    pio run --target upload
    ```
    *Note: For initial flashing, you might need to put the LILYGO T-Dongle S3 into boot mode by holding the button while plugging it into USB.*
*   **Monitor Serial Output:**
    ```shell
    task monitor
    # or
    pio device monitor
    ```
*   **Flash Latest Release Firmware:**
    ```shell
    ./scripts/flash.sh <SERIAL_PORT>
    ```

### :computer: Usage

*   **Wi-Fi Configuration:** After initial boot, connect to the `FrameFi-<MAC>` Wi-Fi Access Point and navigate to `http://192.168.4.1` (if the captive portal doesn't open automatically) to configure Wi-Fi credentials.
*   **Mode Switching (via API):**
    *   Switch to USB Mass Storage (MSC) mode:
        ```sh
        curl -X POST http://<DEVICE_IP>/mode/msc
        ```
    *   Switch to FTP mode:
        ```sh
        curl -X POST http://<DEVICE_IP>/mode/ftp
        ```
*   **Mode Switching (via Button):** A short press of the button on the device toggles between FTP and USB MSC modes.
*   **Reset Wi-Fi Settings:** Hold the button for 3 seconds to clear saved Wi-Fi credentials and restart into AP mode.
*   **FTP Access:** Connect to the device's IP address using an FTP client and the credentials defined in `include/secrets.h`.
*   **MQTT Control:** Publish messages to `frame-fi/display/set` (e.g., `ON` or `OFF`) to control the display via MQTT.

### :page_with_curl: Documentation Server

*   **Serve Local Documentation:**
    ```shell
    task serve
    ```
    This will start a local web server for the project's documentation, typically accessible at `http://0.0.0.0:8000`.

## :pencil: Development Conventions

This project adheres to strict coding and documentation conventions to ensure consistency and maintainability.

*   **C++ Style Guide:** Defined in `src/GEMINI.md`. Key conventions include:
    *   `lowerCamelCase` for all function names.
    *   `/** ... */` for function documentation briefs.
    *   `// --- comment text ---` for general implementation comments.
*   **Bash Script Guidelines:** Defined in `scripts/GEMINI.md`. Key conventions include:
    *   `#!/usr/bin/env bash` shebang.
    *   `main` function as the entry point.
    *   `log` function for standardized output.
    *   `readonly` for constants.
*   **Markdown Documentation Guidelines:** Defined in `docs/GEMINI.md`. Key conventions include:
    *   Use of MkDocs-Material syntax extensions (admonitions, icons, emojis).
    *   Specific heading styles and section organization (`:hammer_and_wrench: Installation`, `:gear: Config`, etc.).
    *   Relative internal links.
*   **Git Commit Conventions:** Defined in `.github/GEMINI.md`. Adheres to Conventional Commits (`<type>(<scope>): <description>`) and Semantic Versioning (`MAJOR.MINOR.PATCH`).
*   **Taskfile Usage:** `Taskfile.yml` is used to define and manage common development tasks, providing a consistent interface for building, testing, and deploying.
