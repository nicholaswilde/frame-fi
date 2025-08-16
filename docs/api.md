---
tags:
  - usage
  - api
---
# :globe_with_meridians: Web API

The device hosts a simple web server that allows you to check status and switch modes.

!!! warning "Insecure Protocol"

    HTTP is an inherently insecure protocol that transmits data, including credentials, in plain text. Only use this feature on a trusted, private network.

## :key: Web Server Credentials

The web server can be protected by basic authentication. You can set the username and password in the WiFiManager setup page.

- **Connect with `curl`:**
    - Use the `-u` or `--user` flag to provide credentials.
    - If you omit the password, `curl` will prompt for it securely.

!!! tip

    To make the web server unauthenticated, make the `user` and `password` blank in the WiFiManager setup page.

## Error Handling

This API uses standard HTTP status codes to indicate the success or failure of a request. Common status codes include:

*   **200 OK**: The request was successful.
*   **400 Bad Request**: The request was malformed or invalid (e.g., incorrect parameters, invalid data format).
*   **401 Unauthorized**: Authentication failed. Ensure you are providing valid credentials.
*   **403 Forbidden**: You are authenticated but do not have permission to perform the requested action.
*   **404 Not Found**: The requested resource or endpoint does not exist.
*   **500 Internal Server Error**: An unexpected error occurred on the server.

Specific endpoints may return additional details in the JSON response body to further clarify the error.

## Web Server Commands

**`GET /`**: Returns the current mode, display status, and SD card information.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/
        ```

    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/
        ```

!!! success "Example Response"

    ```json
    {
      "mode": "USB MSC",
      "display": {
        "status": "on",
        "orientation": 1
      },
      "sd_card": {
        "total_size": 9876543210,
        "used_size": 1234567890,
        "free_size": 8641975320,
        "file_count": 42
      },
      "mqtt": {
        "enabled": true,
        "state": 0,
        "connected": true
      },
      "led": {
        "color": "green",
        "state": "on",
        "brightness": 13
      }
    }
    ```

??? abstract "MQTT State"

    | State | Description                   | State | Description                   |
    |:-----:|-------------------------------|:-----:|-------------------------------|
    | `-4`  | MQTT_CONNECTION_TIMEOUT       |  `1`  | MQTT_CONNECT_BAD_PROTOCOL     |
    | `-3`  | MQTT_CONNECTION_LOST          |  `2`  | MQTT_CONNECT_BAD_CLIENT_ID    |
    | `-2`  | MQTT_CONNECT_FAILED           |  `3`  | MQTT_CONNECT_UNAVAILABLE      |  
    | `-1`  | MQTT_DISCONNECTED             |  `4`  | MQTT_CONNECT_BAD_CREDENTIALS  |
    |  `0`  | MQTT_CONNECTED                |  `5`  | MQTT_CONNECT_UNAUTHORIZED     |    

**`GET /mode/msc`**: Returns the current mode (USB Mass Storage or FTP).

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/mode/msc
        ```

    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/mode/msc
        ```

!!! success "Example Response"

    ```json
    {"status":"success","mode":"USB MSC"}
    ```

**`POST /mode/msc`**: Switches the device to USB Mass Storage (MSC) mode.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/msc
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mode/msc
        ```

!!! success "Example Responses"

    === "Success (200 OK)"
        ```json
        {"status":"success","message":"Switched to MSC mode."}
        ```

    === "No Change (200 OK)"
        ```json
        {"status":"no_change","message":"Already in MSC mode."}
        ```

    === "Error (500 Internal Server Error)"
        ```json
        {"status":"error","message":"Failed to switch to MSC mode."}
        ```

**`GET /mode/ftp`**: Returns the current mode (USB Mass Storage or FTP).

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/mode/ftp
        ```
        
    === "Authenticated"
        
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/mode/ftp
        ```

!!! success "Example Response"

    ```json
    {"status":"success","mode":"Application (FTP Server)"}
    ```

**`POST /mode/ftp`**: Switches the device to FTP mode.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mode/ftp
        ```
        
    === "Authenticated"
        
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mode/ftp
        ```

!!! success "Example Responses"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"Switched to Application (FTP) mode."}
        ```

    === "No Change (200 OK)"

        ```json
        {"status":"no_change","message":"Already in Application (FTP) mode."}
        ```

    === "Error (500 Internal Server Error)"

        ```json
        {"status":"error","message":"Failed to re-initialize SD card."}
        ```

**`POST /device/restart`**: Restarts the device.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/device/restart
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/device/restart
        ```

!!! success "Example Response:"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"Restarting device..."}
        ```

**`GET /display/status`**: Returns the current display status.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/display/status
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/display/status
        ```

!!! success "Example Response"

    ```json
    {"status":"success","display_status":"on"}
    ```

**`POST /display/toggle`**: Toggles the display on and off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/display/toggle
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/display/toggle
        ```

!!! success "Example Responses"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Display toggled on."}
        ```

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"Display toggled off."}
        ```

**`POST /display/on`**: Turns the display on.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/display/on
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/display/on
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Display turned on."}
        ```

**`POST /display/off`**: Turns the display off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/display/off
        ```
    
    === "Authenticated"
        
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/display/off
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Display turned off."}
        ```

**`POST /wifi/reset`**: Resets the Wi-Fi settings and restarts the device.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/wifi/reset
        ```
    
    === "Authenticated"
    
        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/wifi/reset
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"Resetting Wi-Fi and restarting..."}
        ```

!!! warning "Device Unreachable After Reset"

    After resetting the Wi-Fi settings, the device will restart and will no longer be connected to your Wi-Fi network. It will become unreachable at its previous IP address. You must reconnect to its Access Point (AP) to configure the new Wi-Fi credentials. See the [Modes of Operation](modes-of-operation.md) section for details on connecting to the AP.

**`GET /mqtt/status`**: Returns the current MQTT client status.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/mqtt/status
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/mqtt/status
        ```

!!! success "Example Response"

    ```json
    {
      "status": "success",
      "mqtt_enabled": true,
      "mqtt_connected": true,
      "mqtt_state": 0
    }
    ```

**`POST /mqtt/enable`**: Enables the MQTT client.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mqtt/enable
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mqtt/enable
        ```

!!! success "Example Response"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT enabled."}
        ```

**`POST /mqtt/disable`**: Disables the MQTT client.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mqtt/disable
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mqtt/disable
        ```

!!! success "Example Response"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT disabled."}
        ```

**`POST /mqtt/toggle`**: Toggles the MQTT client on and off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/mqtt/toggle
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/mqtt/toggle
        ```

!!! success "Example Responses"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT enabled."}
        ```

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"MQTT disabled."}
        ```

**`GET /led/status`**: Returns the current LED color and state.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/led/status
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/led/status
        ```

!!! success "Example Response"

    ```json
    {
      "status": "success",
      "color": "green",
      "state": "on",
      "brightness": 13
    }
    ```

**`POST /led/on`**: Turns the LED on.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/led/on
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/led/on
        ```

!!! success "Example Response"

    ```json
    {"status":"success","message":"LED turned on."}
    ```

**`POST /led/off`**: Turns the LED off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/led/off
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/led/off
        ```

!!! success "Example Response"

    === "Success (200 OK)"
    
        ```json
        {"status":"success","message":"LED turned off."}
        ```

**`POST /led/toggle`**: Toggles the LED on and off.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST http://<DEVICE_IP>/led/toggle
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST http://<DEVICE_IP>/led/toggle
        ```

!!! success "Example Responses"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"LED toggled on."}
        ```

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"LED toggled off."}
        ```

**`GET /led/brightness`**: Returns the current LED brightness.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X GET http://<DEVICE_IP>/led/brightness
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X GET http://<DEVICE_IP>/led/brightness
        ```

!!! success "Example Response"

    ```json
    {"status":"success","brightness":128}
    ```

**`POST /led/brightness`**: Sets the LED brightness. The body of the request should be a plain text integer between 0 and 255.

!!! code ""

    === "Unauthenticated"

        ```sh
        curl -X POST --H 'Content-Type: text/plain' d '128' http://<DEVICE_IP>/led/brightness
        ```

    === "Authenticated"

        ```sh
        curl -u <USERNAME>:<PASSWORD> -X POST -H 'Content-Type: text/plain' -d '128' http://<DEVICE_IP>/led/brightness
        ```

!!! success "Example Responses"

    === "Success (200 OK)"

        ```json
        {"status":"success","message":"LED brightness set to 128."}
        ```

    === "Error (400 Bad Request)"

        ```json
        {"status":"error","message":"Invalid brightness value. Body must be a plain text integer between 0 and 255."}
        ```

## :link: References