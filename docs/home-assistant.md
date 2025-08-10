---
tags:
  - home-assistant
  - mqtt
---
# :simple-homeassistant: Home Assistant Integration

This page provides an example of how to integrate FrameFi with [Home Assistant][1] using MQTT.

## :package: Prerequisites

- MQTT is enabled and configured on your FrameFi device.
- You have a running MQTT broker that is accessible by both Home Assistant and your FrameFi.
- You have the [MQTT integration][2] set up in Home Assistant.

## :gear: Configuration

To enable MQTT, you must first configure it in the WiFiManager setup page. See [Getting Started](getting-started.md#satellite-wi-fi-and-mqtt-setup) for more details.

Next, add the following configuration to your `configuration.yaml` file in Home Assistant.

!!! abstract "configuration.yaml"

    ```yaml
    mqtt:
      - sensor:
        - name: "FrameFi Status"
          state_topic: "frame-fi/state"
          value_template: "{{ value_json.mode }}"
          icon: "mdi:image-frame"
        - name: "FrameFi File Count"
          state_topic: "frame-fi/state"
          value_template: "{{ value_json.sd_card.file_count }}"
          unit_of_measurement: "files"
          icon: "mdi:file-multiple"
        - name: "FrameFi Used Space"
          state_topic: "frame-fi/state"
          value_template: "{{ (value_json.sd_card.used_size | float / 1024 / 1024 / 1024) | round(2) }}"
          unit_of_measurement: "GB"
          icon: "mdi:sd"
        - name: "FrameFi Total Space"
          state_topic: "frame-fi/state"
          value_template: "{{ (value_json.sd_card.total_size | float / 1024 / 1024 / 1024) | round(2) }}"
          unit_of_measurement: "GB"
          icon: "mdi:sd"
      - switch:
        - name: "FrameFi Display"
          state_topic: "frame-fi/display/status"
          command_topic: "frame-fi/display/set"
          payload_on: "ON"
          payload_off: "OFF"
          state_on: "ON"
          state_off: "OFF"
          optimistic: false
          qos: 0
          retain: true
          icon: "mdi:monitor"
      - button:
        - name: "FrameFi Restart"
          command_topic: "frame-fi/restart"
          payload_press: "RESTART"
          icon: "mdi:restart"
      - device_tracker:
        - name: "FrameFi"
          state_topic: "frame-fi/status"
          availability_topic: "frame-fi/status"
          payload_available: "online"
          payload_not_available: "offline"
          payload_home: "online"
          payload_not_home: "offline"
          source_type: "router"
    ```

## :mag: Explanation

- **`mqtt.sensor`**:
    - This creates a new sensor entity in Home Assistant named `FrameFi Status`.
    - It listens to the `frame-fi/state` topic and uses a `value_template` to extract the `mode` from the JSON payload.
    - The icon is set to `mdi:image-frame`.

- **`mqtt.switch`**:
    - This creates a new switch entity named `FrameFi Display`.
    - It allows you to turn the FrameFi's display on and off from the Home Assistant UI.
    - `command_topic`: When you toggle the switch, it sends either `ON` or `OFF` to the `frame-fi/display/set` topic.
    - `state_topic`: It listens to the `frame-fi/display/status` topic to get the current state of the display.
    - `retain: true`: This ensures that the last command is retained by the MQTT broker, so the device will pick up the correct state when it reconnects.

- **`mqtt.button`**:
    - This creates a new button entity named `FrameFi Restart`.
    - When you press the button, it sends `RESTART` to the `frame-fi/restart` topic.

- **`mqtt.device_tracker`**:
    - This creates a new device tracker entity named `FrameFi`.
    - It listens to the `frame-fi/status` topic to determine the device's online/offline status.
    - `payload_available`: When the device publishes `online` to the `frame-fi/status` topic, it will be marked as `home`.
    - `payload_not_available`: When the device publishes `offline` to the `frame-fi/status` topic, it will be marked as `away`.
    - `source_type: "router"`: This tells Home Assistant to treat the device as a network-based tracker.

- **Additional Sensors**:
    - The configuration also adds sensors for `File Count`, `Used Space`, and `Total Space`.
    - These sensors also listen to the `frame-fi/state` topic and use `value_template` to extract their respective values from the JSON payload.
    - They are configured with appropriate units (`files`, `GB`) and icons.

## :link: References

- [Home Assistant][1]
- [Home Assistant MQTT Integration][2]

[1]: <https://www.home-assistant.io/>
[2]: <https://www.home-assistant.io/integrations/mqtt/>
