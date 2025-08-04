---
tags:
  - software
---
# :package: Software Dependencies

This project relies on a set of software tools and libraries to ensure proper functionality, development, and documentation. The following sections provide an overview of these dependencies.

## :wrench: Development Tools

- **[PlatformIO](https://platformio.org/):** An open-source ecosystem for IoT development. It provides a command-line interface (CLI) and a VSCode extension for a seamless development experience.
- **[Task](https://taskfile.dev/):** A task runner and build tool that automates development and build tasks. See the project's `Taskfile.yml` for available commands.
- **[Docker](https://www.docker.com/):** Used to create a consistent and reproducible development environment for the documentation server.
- **[Renovate](https://www.mend.io/free-developer-tools/renovate/):** A tool for automated dependency updates, ensuring that the project's dependencies are always up-to-date.
- **[SOPS](https://github.com/getsops/sops):** A tool for managing secrets, allowing for the encryption and decryption of sensitive information.

??? abstract "Task List"

    ```ini
    --8<-- "task-list.txt"
    ```

## :books: Libraries

- **[knolleary/PubSubClient](https://github.com/knolleary/pubsubclient):** A client library for MQTT.
- **[mathertel/OneButton](https://github.com/mathertel/OneButton):** A library for debouncing and simplifying button inputs.
- **[fastled/FastLED](https://github.com/FastLED/FastLED):** A library for controlling a wide range of addressable LEDs.
- **[tzapu/WiFiManager](https://github.com/tzapu/WiFiManager):** A library for managing WiFi connections on ESP8266 and ESP32 devices.
- **[bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI):** A library for driving TFT displays.

## :scroll: Languages and Frameworks

- **[C++](https://isocpp.org/):** The primary language used for the firmware development.
- **[Python](https://www.python.org/):** Used for scripting and automation tasks.
- **[Shell Script](https://www.gnu.org/software/bash/):** Used for various automation and build tasks.
- **[MkDocs](https://www.mkdocs.org/):** A static site generator used for creating the project's documentation.

## :rocket: CI/CD

- **[GitHub Actions](https://github.com/features/actions):** Used for automating the build, test, and release process.

## :link: References
