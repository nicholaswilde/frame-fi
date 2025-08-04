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

Add the following configuration to your `configuration.yaml` file in Home Assistant.

!!! abstract "configuration.yaml"

    ```yaml
    mqtt:
      sensor:
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

      switch:
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

- **Additional Sensors**:
    - The configuration also adds sensors for `File Count`, `Used Space`, and `Total Space`.
    - These sensors also listen to the `frame-fi/state` topic and use `value_template` to extract their respective values from the JSON payload.
    - They are configured with appropriate units (`files`, `GB`) and icons.

## :link: References

- [Home Assistant][1]
- [Home Assistant MQTT Integration][2]

[1]: <https://www.home-assistant.io/>
[2]: <https://www.home-assistant.io/integrations/mqtt/>
