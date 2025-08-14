---
tags:
  - software
---
# :package: Software Dependencies

This project relies on a set of software tools and libraries to ensure proper
functionality, development, and documentation. The following sections provide an
overview of these dependencies.

## :wrench: Development Tools

- [PlatformIO][1]: An open-source ecosystem for IoT development. It provides a
  command-line interface (CLI) and a VSCode extension for a seamless
  development experience.
- [Task][2]: A task runner and build tool that automates development and build
  tasks. See the project's `Taskfile.yml` for available commands.
- [Docker][3]: Used to create a consistent and reproducible development
  environment for the documentation server.
- [Renovate][4]: A tool for automated dependency updates, ensuring that the
  project's dependencies are always up-to-date.
- [SOPS][5]: A tool for managing secrets, allowing for the encryption and
  decryption of sensitive information.

??? abstract "Task List"

    ```ini
    --8<-- "task-list.txt"
    ```

## :books: Libraries

- [bblanchon/ArduinoJson][6]: A C++ JSON library for Arduino and IoT (Internet
  of Things).
- [knolleary/PubSubClient][7]: A client library for MQTT.
- [mathertel/OneButton][8]: A library for debouncing and simplifying button
  inputs.
- [fastled/FastLED][9]: A library for controlling a wide range of addressable
  LEDs.
- [tzapu/WiFiManager][10]: A library for managing Wi-Fi connections on ESP8266
  and ESP32 devices.
- [SimpleFTPServer][11]: A library for creating an FTP server on an ESP32.
- [bodmer/TFT_eSPI][12]: A library for driving TFT displays.
- [Preferences][18]: A library for non-volatile storage on ESP32 devices.

## :scroll: Languages and Frameworks

- [C++][13]: The primary language used for the firmware development.
- [Python][14]: Used for scripting and automation tasks.
- [Shell Script][15]: Used for various automation and build tasks.
- [MkDocs][16]: A static site generator used for creating the project's
  documentation.

## :rocket: CI/CD

- [GitHub Actions][17]: Used for automating the build, test, and release
  process.

## :link: References

[1]: https://platformio.org/
[2]: https://taskfile.dev/
[3]: https://www.docker.com/
[4]: https://www.mend.io/free-developer-tools/renovate/
[5]: https://github.com/getsops/sops
[6]: https://github.com/bblanchon/ArduinoJson
[7]: https://github.com/knolleary/pubsubclient
[8]: https://github.com/mathertel/OneButton
[9]: https://github.com/FastLED/FastLED
[10]: https://github.com/tzapu/WiFiManager
[11]: https://github.com/xreef/SimpleFTPServer
[12]: https://github.com/Bodmer/TFT_eSPI
[13]: https://isocpp.org/
[14]: https://www.python.org/
[15]: https://www.gnu.org/software/bash/
[16]: https://www.mkdocs.org/
[17]: https://github.com/features/actions
[18]: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html
---
