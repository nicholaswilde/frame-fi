# :signal_strength: FrameFi Project Overview & Agent Guidelines

This document provides a comprehensive overview of the FrameFi project and instructions for AI agents working in this repository.

## Project Rules & Guidelines

### Codebase Lookup Preference
- When referring to the FrameFi project, always inspect the local repository first at `/home/nicholas/git/nicholaswilde/frame-fi/`.
- For theme definitions and color tokens, reference `/home/nicholas/git/nicholaswilde/catppuccin-rgb565/` (upstream: `https://github.com/nicholaswilde/catppuccin-rgb565`).
- For reference firmware and hardware implementations on the LILYGO T-Dongle S3, inspect the local `reference/` directory (`reference/USBArmyKnife`, `reference/maypole`, `reference/firmware`).

### Build and Test Commands
- Build firmware: `task build` or `pio run`
- Upload firmware: `task upload` or `pio run --target upload`
- Monitor serial output: `task monitor` or `pio device monitor`
- Flash release firmware: `./scripts/flash.sh <SERIAL_PORT>`

### RTK Command Guidelines
- **Git Operations**: Prefix `git` commands with `rtk` (e.g., `rtk git status`, `rtk git diff`, `rtk git log`, `rtk git commit`, `rtk git push`).
- **GitHub CLI**: Prefix `gh` commands with `rtk` (e.g., `rtk gh issue list | cat`, `rtk gh pr status | cat`). Always pipe `gh` commands to `cat` to bypass interactive pagers.
- **File & Directory Inspection**: Use `rtk ls`, `rtk tree`, `rtk find`, or `rtk read` when listing or reading files to get token-optimized output.
- **Searching**: Use `rtk grep` or `rtk rg` for line search pattern matching.
- **Build & Test Outputs**: Use `rtk err` or `rtk test` when running build commands to filter output to errors/failures only.

### What To Do Next
- When asked "what to do next" (or similar), **always check the remote repository issues first** using `gh`:
  ```bash
  rtk gh issue list | cat
  ```

### Issue Creation
- When asked to create an issue, use your best guess to determine if it is a new feature or a bug fix.
- Prefix the issue title with `[feat]: <description>` or `[bug]: <description>`.
- Add the `enhancement` or `bug` label to the issue accordingly using the `--label` flag with the `gh` command.

### Issue Resolution & Testing
- When addressing or fixing GitHub issues, verify changes with manual or automated test scripts (e.g. `scripts/test-api.sh`, `scripts/test-ftp.sh`) to prevent regressions.

### Context-Mode MCP Integration
This project uses `context-mode` for token-optimized command execution, file inspection, and searching.
- **Call Pattern**: Invoke tools via `call_mcp_tool` with `ServerName: "context-mode"`:
  - `ctx_execute`: Run commands (shell, JavaScript, Python) in-sandbox. For shell commands, pass `language: "shell"` and prefix commands with `rtk` (e.g. `rtk git status`, `rtk gh issue list | cat`).
  - `ctx_execute_file`: Inspect, summarize, or extract data from files in-sandbox (`language: "javascript"`, `FILE_CONTENT`) without dumping raw content into context. (Native `read_file` is reserved for targeted exact-byte inspection right before `edit_file`).
  - `ctx_batch_execute`: Batch execute multiple repository inspection commands and query results.
  - `ctx_search` & `ctx_index`: Search and index codebase content and session memory.
  - `ctx_doctor`: Verify context-mode health and runtime status.

### Serena MCP & CodeGraph Integration
- **Serena MCP**: Use for semantic code exploration, LSP symbol navigation, and surgical edits (`get_symbols_overview`, `find_symbol`, `find_declaration`, `find_referencing_symbols`).
- **CodeGraph**: Use `codegraph_explore` MCP tool or `codegraph explore "<symbol>"` CLI for call-graph exploration, blast radius analysis, and symbol dependencies.

---

## :memo: Project Overview

FrameFi transforms a LILYGO T-Dongle S3 (an ESP32-S3 based development board) into a versatile adapter for digital picture frames. Its primary purpose is to enable remote management of photo libraries via an FTP server or direct access to the SD card in USB Mass Storage (MSC) mode. The project is built using the PlatformIO ecosystem for embedded development with the Arduino framework.

### :computer: Hardware Target & Architecture
*   **Microcontroller:** ESP32-S3 (esp32-s3-devkitc-1)
*   **Display:** 0.96" ST7735 color TFT LCD (80x160 resolution, ST7735_GREENTAB160x80)
    *   `TFT_MOSI`: 3, `TFT_SCLK`: 5, `TFT_CS`: 4, `TFT_DC`: 2, `TFT_RST`: 1, `TFT_LEDA` (backlight): 38
*   **Storage:** MicroSD slot via 4-bit `SD_MMC` interface
    *   `SD_MMC_CLK`: 12, `SD_MMC_CMD`: 16, `SD_MMC_D0`: 14, `SD_MMC_D1`: 17, `SD_MMC_D2`: 21, `SD_MMC_D3`: 18
*   **RGB LED:** Single onboard APA102 addressable LED (`LED_DI`: 40, `LED_CI`: 39)
*   **Button:** Onboard push button (`BTN_PIN`: 0)
*   **USB:** Native USB CDC + MSC support (`USBMSC`)

### :gear: Core Software Stack
*   **Development Environment:** PlatformIO with Arduino framework (`framework-arduinoespressif32`)
*   **Networking:**
    *   `WiFiManager`: For dynamic Wi-Fi configuration via a captive portal.
    *   `WebServer`: Provides a REST API for device control and status.
    *   `PubSubClient`: Enables MQTT communication for Home Assistant integration.
    *   `SimpleFTPServer`: Implements the FTP server for remote photo management.
*   **Storage:**
    *   `SD_MMC`: High-speed SD card interface.
    *   `USBMSC`: Implements USB Mass Storage functionality.
*   **Display & Theming:**
    *   `TFT_eSPI`: Fast SPI library for controlling the integrated LCD display.
    *   Catppuccin color palette (`catppuccin_colors.h`).
*   **Build Automation:** `Taskfile` (`task` command runner) for streamlined workflows.
*   **Secrets Management:** `sops` for encrypting sensitive configuration files.

---

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
    This starts a local web server for the project documentation at `http://0.0.0.0:8000`.

---

## :pencil: Development Conventions

This project adheres to strict coding and documentation conventions to ensure consistency and maintainability.

*   **C++ Style Guide:** Defined in [`src/AGENTS.md`](src/AGENTS.md). Key conventions include:
    *   `lowerCamelCase` for all function names.
    *   `/** ... */` for function documentation briefs.
    *   `// --- comment text ---` for general implementation comments.
*   **Bash Script Guidelines:** Defined in [`scripts/AGENTS.md`](scripts/AGENTS.md). Key conventions include:
    *   `#!/usr/bin/env bash` shebang.
    *   `main` function as the entry point.
    *   `log` function for standardized output.
    *   `readonly` for constants.
*   **Markdown Documentation Guidelines:** Defined in [`docs/AGENTS.md`](docs/AGENTS.md). Key conventions include:
    *   Use of MkDocs-Material syntax extensions (admonitions, icons, emojis).
    *   Specific heading styles and section organization (`:hammer_and_wrench: Installation`, `:gear: Config`, etc.).
    *   Relative internal links.
*   **Git Commit Conventions:** Defined in [`.github/AGENTS.md`](.github/AGENTS.md). Adheres to Conventional Commits (`<type>(<scope>): <description>`) and Semantic Versioning (`MAJOR.MINOR.PATCH`).
*   **Taskfile Usage:** `Taskfile.yml` is used to define and manage common development tasks, providing a consistent interface for building, testing, and deploying.

---

## Code Style Principles

- **Readability:** Code should be easy to read and understand by humans. Avoid overly clever or obscure constructs.
- **Consistency:** Follow existing patterns in the codebase. Maintain consistent formatting, naming, and structure.
- **Simplicity:** Prefer simple solutions over complex ones. Break down complex problems into smaller, manageable parts.
- **Maintainability:** Write code that is easy to modify and extend. Minimize dependencies and coupling.
- **Documentation:** Document *why* something is done, not just *what*. Keep documentation up-to-date with code changes.
