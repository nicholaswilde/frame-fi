#!/usr/bin/env bash
################################################################################
#
# test-api.sh
# ----------------
# Tests the FrameFi device API by performing a GET request and verifying the JSON response.
#
# @author Nicholas Wilde, 0xb299a622
# @date 16 Aug 2025
# @version 0.1.0
#
################################################################################

# Options
set -e
set -o pipefail

# These are constants
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
BLUE=$(tput setaf 4)
RESET=$(tput sgr0)
readonly RED GREEN YELLOW BLUE RESET

DELAY_SECONDS=1
readonly DELAY_SECONDS

# Log function for standardized output
function log() {
  local TYPE="$1"
  local MESSAGE="$2"
  local COLOR=""
  local EMOJI=""

  case "$TYPE" in
    "INFO") COLOR="${BLUE}"; EMOJI="";;
    "WARN") COLOR="${YELLOW}"; EMOJI="⚠️ ";;
    "ERRO") COLOR="${RED}"; EMOJI="❌ ";;
    "SUCCESS") COLOR="${BLUE}"; EMOJI="✅ "; TYPE="INFO";;
    *) COLOR="${RESET}";;
  esac

  echo "${COLOR}${TYPE}${RESET}[$(date +'%Y-%m-%d %H:%M:%S')] ${EMOJI}${MESSAGE}"
}

# Check for dependencies
function check_dependencies() {
  log "INFO" "Checking dependencies..."
  if ! command -v curl &> /dev/null; then
    log "ERRO" "curl could not be found. Please install it."
    exit 1
  fi
  if ! command -v jq &> /dev/null; then
    log "ERRO" "jq could not be found. Please install it."
    exit 1
  fi
  log "SUCCESS" "Dependencies checked."
}

function load_vars() {
  local ENV_FILE="$(dirname "$0")/.env"

  if [ ! -f "${ENV_FILE}" ]; then
    log "ERRO" "Environment file not found: ${ENV_FILE}"
    log "ERRO" "Please create it from .env.tmpl and ensure FTP_HOST is set."
    exit 1
  fi

  source "${ENV_FILE}"

  if [ -z "${FTP_HOST}" ]; then
    log "ERRO" "FTP_HOST not set in ${ENV_FILE}"
    exit 1
  fi
}

# Check if the device is online
function check_device_status() {
  log "INFO" "Checking if device at ${FTP_HOST} is online..."
  if ! curl -s -o /dev/null --fail --connect-timeout 5 "http://${FTP_HOST}/"; then
    log "ERRO" "Device at ${FTP_HOST} is not responding."
    log "ERRO" "Please ensure the device is connected to the network and the IP address is correct."
    exit 1
  fi
  log "SUCCESS" "Device is online."
}

# Check initial device mode
function check_initial_mode() {
  log "INFO" "Verifying initial device mode is USB MSC..."
  local RESPONSE
  RESPONSE=$(curl -s "http://${FTP_HOST}/")
  local CURRENT_MODE
  CURRENT_MODE=$(echo "${RESPONSE}" | jq -r '.mode')

  if [ "${CURRENT_MODE}" != "USB MSC" ]; then
    log "ERRO" "Device is not in USB MSC mode. Current mode: ${CURRENT_MODE}"
    log "ERRO" "Please set the device to USB MSC mode before running tests."
    exit 1
  fi
  log "SUCCESS" "Device is in USB MSC mode."
}

# Function to perform an API request and validate the JSON response
function request_and_verify() {
  local METHOD="$1"
  local ENDPOINT="$2"
  local PAYLOAD="$3"
  local EXPECTED_FIELDS="$4"
  local API_URL="http://${FTP_HOST}${ENDPOINT}"

  log "INFO" "Testing ${METHOD} ${API_URL}"

  local RESPONSE
  if [ "${METHOD}" == "POST" ]; then
    if [ -n "${PAYLOAD}" ]; then
      RESPONSE=$(curl -s -X POST -H "Content-Type: application/json" -d "${PAYLOAD}" "${API_URL}")
    else
      RESPONSE=$(curl -s -X POST "${API_URL}")
    fi
  else # GET
    RESPONSE=$(curl -s "${API_URL}")
  fi

  if ! echo "${RESPONSE}" | jq -e . > /dev/null; then
    log "ERRO" "Response from ${ENDPOINT} is not valid JSON."
    log "ERRO" "Response: ${RESPONSE}"
    exit 1
  fi

  if [ "${METHOD}" == "POST" ]; then
    local STATUS
    STATUS=$(echo "${RESPONSE}" | jq -r '.status')
    if [ "${STATUS}" != "success" ]; then
      log "ERRO" "${METHOD} ${ENDPOINT} failed. Status: ${STATUS}, Message: $(echo "${RESPONSE}" | jq -r '.message')"
      exit 1
    fi
    log "INFO" "${METHOD} ${ENDPOINT} successful. Message: $(echo "${RESPONSE}" | jq -r '.message')"
  elif [ "${METHOD}" == "GET" ]; then
    for field in ${EXPECTED_FIELDS}; do
      local VALUE
      VALUE=$(echo "${RESPONSE}" | jq -r "${field}")
      if [ "${VALUE}" == "null" ]; then
        log "ERRO" "JSON response from ${ENDPOINT} missing '${field}' field or it's null."
        exit 1
      fi
      log "INFO" "${ENDPOINT} - ${field}: ${VALUE}"
    done
    log "SUCCESS" "${ENDPOINT} test completed successfully."
  fi
}

function verify_display_toggle() {
  log "INFO" "Verifying display APIs..."

  # Test display off
  log "INFO" "Testing display off..."
  request_and_verify "POST" "/display/off" "" ""
  sleep ${DELAY_SECONDS}
  local STATUS
  STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "OFF" ]; then
    log "ERRO" "Display failed to turn off. Current status: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "Display is OFF."

  # Test display off again (already off)
  log "INFO" "Testing display off again (already off)..."
  local RESPONSE
  RESPONSE=$(curl -s -X POST "http://${FTP_HOST}/display/off")
  local STATUS=$(echo "${RESPONSE}" | jq -r '.status')
  local MESSAGE=$(echo "${RESPONSE}" | jq -r '.message')
  if [ "${STATUS}" != "success" ] || [ "${MESSAGE}" != "Display turned off." ]; then
    log "ERRO" "Display off again failed. Status: ${STATUS}, Message: ${MESSAGE}"
    exit 1
  fi
  sleep ${DELAY_SECONDS}
  STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "OFF" ]; then
    log "ERRO" "Display state changed unexpectedly after turning off again. Current status: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "Display is still OFF and message is correct."

  # Test display on
  log "INFO" "Testing display on..."
  request_and_verify "POST" "/display/on" "" ""
  sleep ${DELAY_SECONDS}
  STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "ON" ]; then
    log "ERRO" "Display failed to turn on. Current status: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "Display is ON."

  # Test display on again (already on)
  log "INFO" "Testing display on again (already on)..."
  local RESPONSE
  RESPONSE=$(curl -s -X POST "http://${FTP_HOST}/display/on")
  local STATUS=$(echo "${RESPONSE}" | jq -r '.status')
  local MESSAGE=$(echo "${RESPONSE}" | jq -r '.message')
  if [ "${STATUS}" != "success" ] || [ "${MESSAGE}" != "Display turned on." ]; then
    log "ERRO" "Display on again failed. Status: ${STATUS}, Message: ${MESSAGE}"
    exit 1
  fi
  sleep ${DELAY_SECONDS}
  STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "ON" ]; then
    log "ERRO" "Display state changed unexpectedly after turning on again. Current status: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "Display is still ON and message is correct."

  # Test display toggle
  log "INFO" "Verifying display toggle..."
  local INITIAL_STATUS
  INITIAL_STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status')
  log "INFO" "Initial display status: ${INITIAL_STATUS}"

  request_and_verify "POST" "/display/toggle" "" ""
  sleep ${DELAY_SECONDS}

  local NEW_STATUS
  NEW_STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status')
  log "INFO" "New display status: ${NEW_STATUS}"

  if [ "${INITIAL_STATUS}" == "${NEW_STATUS}" ]; then
    log "ERRO" "Display status did not change after toggle."
    exit 1
  fi
  log "SUCCESS" "Display toggle verified successfully."

  # Toggle display back on
  log "INFO" "Toggling display back on..."
  request_and_verify "POST" "/display/toggle" "" ""
  sleep ${DELAY_SECONDS}

  NEW_STATUS=$(curl -s "http://${FTP_HOST}/display/status" | jq -r '.display_status' | tr '[:lower:]' '[:upper:]')
  if [ "${NEW_STATUS}" != "ON" ]; then
    log "ERRO" "Display failed to turn back on. Current status: ${NEW_STATUS}"
    exit 1
  fi
  log "SUCCESS" "Display is ON again."
}

function verify_led_toggle() {
  log "INFO" "Verifying LED APIs..."

  # Test LED off
  log "INFO" "Testing LED off..."
  request_and_verify "POST" "/led/off" "" ""
  sleep ${DELAY_SECONDS}
  local STATUS
  STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "OFF" ]; then
    log "ERRO" "LED failed to turn off. Current state: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "LED is OFF."

  # Test LED off again (already off)
  log "INFO" "Testing LED off again (already off)..."
  local RESPONSE
  RESPONSE=$(curl -s -X POST "http://${FTP_HOST}/led/off")
  local STATUS=$(echo "${RESPONSE}" | jq -r '.status')
  local MESSAGE=$(echo "${RESPONSE}" | jq -r '.message')
  if [ "${STATUS}" != "success" ] || [ "${MESSAGE}" != "LED turned off." ]; then
    log "ERRO" "LED off again failed. Status: ${STATUS}, Message: ${MESSAGE}"
    exit 1
  fi
  sleep ${DELAY_SECONDS}
  STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "OFF" ]; then
    log "ERRO" "LED state changed unexpectedly after turning off again. Current state: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "LED is still OFF and message is correct."

  # Test LED on
  log "INFO" "Testing LED on..."
  request_and_verify "POST" "/led/on" "" ""
  sleep ${DELAY_SECONDS}
  STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "ON" ]; then
    log "ERRO" "LED failed to turn on. Current state: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "LED is ON."

  # Test LED on again (already on)
  log "INFO" "Testing LED on again (already on)..."
  local RESPONSE
  RESPONSE=$(curl -s -X POST "http://${FTP_HOST}/led/on")
  local STATUS=$(echo "${RESPONSE}" | jq -r '.status')
  local MESSAGE=$(echo "${RESPONSE}" | jq -r '.message')
  if [ "${STATUS}" != "success" ] || [ "${MESSAGE}" != "LED turned on." ]; then
    log "ERRO" "LED on again failed. Status: ${STATUS}, Message: ${MESSAGE}"
    exit 1
  fi
  sleep ${DELAY_SECONDS}
  STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state' | tr '[:lower:]' '[:upper:]')
  if [ "${STATUS}" != "ON" ]; then
    log "ERRO" "LED state changed unexpectedly after turning on again. Current state: ${STATUS}"
    exit 1
  fi
  log "SUCCESS" "LED is still ON and message is correct."

  # Test LED toggle
  log "INFO" "Verifying LED toggle..."
  local INITIAL_STATUS
  INITIAL_STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state')
  log "INFO" "Initial LED state: ${INITIAL_STATUS}"

  request_and_verify "POST" "/led/toggle" "" ""
  sleep ${DELAY_SECONDS}

  local NEW_STATUS
  NEW_STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state')
  log "INFO" "New LED state: ${NEW_STATUS}"

  if [ "${INITIAL_STATUS}" == "${NEW_STATUS}" ]; then
    log "ERRO" "LED state did not change after toggle."
    exit 1
  fi
  log "SUCCESS" "LED toggle verified successfully."

  # Toggle LED back on
  log "INFO" "Toggling LED back on..."
  request_and_verify "POST" "/led/toggle" "" ""
  sleep ${DELAY_SECONDS}

  NEW_STATUS=$(curl -s "http://${FTP_HOST}/led/status" | jq -r '.state' | tr '[:lower:]' '[:upper:]')
  if [ "${NEW_STATUS}" != "ON" ]; then
    log "ERRO" "LED failed to turn back on. Current state: ${NEW_STATUS}"
    exit 1
  fi
  log "SUCCESS" "LED is ON again."
}

function verify_led_brightness() {
  log "INFO" "Verifying LED brightness..."
  local INITIAL_BRIGHTNESS
  INITIAL_BRIGHTNESS=$(curl -s "http://${FTP_HOST}/led/brightness" | jq -r '.brightness')
  log "INFO" "Initial LED brightness: ${INITIAL_BRIGHTNESS}"

  local NEW_BRIGHTNESS_VAL=128
  if [ "${INITIAL_BRIGHTNESS}" == "128" ]; then
    NEW_BRIGHTNESS_VAL=255
  fi

  request_and_verify "POST" "/led/brightness" "${NEW_BRIGHTNESS_VAL}" ""

  local NEW_BRIGHTNESS
  NEW_BRIGHTNESS=$(curl -s "http://${FTP_HOST}/led/brightness" | jq -r '.brightness')
  log "INFO" "New LED brightness: ${NEW_BRIGHTNESS}"

  if [ "${NEW_BRIGHTNESS}" != "${NEW_BRIGHTNESS_VAL}" ]; then
    log "ERRO" "LED brightness did not change as expected."
    exit 1
  fi
  log "SUCCESS" "LED brightness verified successfully."
  request_and_verify "POST" "/led/brightness" "${INITIAL_BRIGHTNESS}" ""
}

function verify_mqtt_actions() {
  log "INFO" "Verifying MQTT APIs..."

  # Test MQTT disable
  log "INFO" "Testing MQTT disable..."
  request_and_verify "POST" "/mqtt/disable" "" ""
  sleep ${DELAY_SECONDS}
  local ENABLED_STATUS
  ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  if [ "${ENABLED_STATUS}" != "false" ]; then
    log "ERRO" "MQTT disable failed. Enabled status: ${ENABLED_STATUS}"
    exit 1
  fi
  log "SUCCESS" "MQTT is DISABLED."

  # Test MQTT disable again (already disabled)
  log "INFO" "Testing MQTT disable again (already disabled)..."
  request_and_verify "POST" "/mqtt/disable" "" ""
  sleep ${DELAY_SECONDS}
  ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  if [ "${ENABLED_STATUS}" != "false" ]; then
    log "ERRO" "MQTT enabled status changed unexpectedly after disabling again. Current status: ${ENABLED_STATUS}"
    exit 1
  fi
  log "SUCCESS" "MQTT is still DISABLED."

  # Test MQTT enable
  log "INFO" "Testing MQTT enable..."
  request_and_verify "POST" "/mqtt/enable" "" ""
  sleep ${DELAY_SECONDS}
  ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  if [ "${ENABLED_STATUS}" != "true" ]; then
    log "ERRO" "MQTT enable failed. Enabled status: ${ENABLED_STATUS}"
    exit 1
  fi
  log "SUCCESS" "MQTT is ENABLED."

  # Test MQTT enable again (already enabled)
  log "INFO" "Testing MQTT enable again (already enabled)..."
  request_and_verify "POST" "/mqtt/enable" "" ""
  sleep ${DELAY_SECONDS}
  ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  if [ "${ENABLED_STATUS}" != "true" ]; then
    log "ERRO" "MQTT enabled status changed unexpectedly after enabling again. Current status: ${ENABLED_STATUS}"
    exit 1
  fi
  log "SUCCESS" "MQTT is still ENABLED."

  # Test MQTT toggle
  log "INFO" "Verifying MQTT toggle..."
  local INITIAL_ENABLED_STATUS
  INITIAL_ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  log "INFO" "Initial MQTT enabled status: ${INITIAL_ENABLED_STATUS}"

  request_and_verify "POST" "/mqtt/toggle" "" ""
  sleep ${DELAY_SECONDS}

  local NEW_ENABLED_STATUS
  NEW_ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  log "INFO" "New MQTT enabled status: ${NEW_ENABLED_STATUS}"

  if [ "${INITIAL_ENABLED_STATUS}" == "${NEW_ENABLED_STATUS}" ]; then
    log "ERRO" "MQTT enabled status did not change after toggle."
    exit 1
  fi
  log "SUCCESS" "MQTT toggle verified successfully."

  # Toggle MQTT back to initial state (ON)
  log "INFO" "Toggling MQTT back to initial state..."
  request_and_verify "POST" "/mqtt/toggle" "" ""
  sleep ${DELAY_SECONDS}

  NEW_ENABLED_STATUS=$(curl -s "http://${FTP_HOST}/mqtt/status" | jq -r '.mqtt_enabled')
  if [ "${NEW_ENABLED_STATUS}" != "true" ]; then
    log "ERRO" "MQTT failed to turn back on. Current status: ${NEW_ENABLED_STATUS}"
    exit 1
  fi
  log "SUCCESS" "MQTT is ON again."
}

function verify_gets(){
  log "INFO" "Starting API tests for device at ${FTP_HOST}"

  request_and_verify "GET" "/" "" ".mode .display.status .display.orientation .sd_card.total_size .sd_card.used_size .sd_card.free_size .sd_card.file_count .mqtt.enabled .mqtt.state .mqtt.connected .led.color .led.state .led.brightness"
  # request_and_verify "GET" "/mode/msc" "" ".status .message"
  # request_and_verify "GET" "/mode/ftp" "" ".status .message"
  request_and_verify "GET" "/display/status" "" ".status .display_status"
  request_and_verify "GET" "/mqtt/status" "" ".status .mqtt_enabled .mqtt_state .mqtt_connected"
  request_and_verify "GET" "/led/status" "" ".status .color .state .brightness"
  request_and_verify "GET" "/led/brightness" "" ".status .brightness"
}

# Check initial display and LED status
function check_initial_display_and_led_status() {
  log "INFO" "Verifying initial display and LED status..."
  local RESPONSE
  RESPONSE=$(curl -s "http://${FTP_HOST}/")

  local DISPLAY_STATUS
  DISPLAY_STATUS=$(echo "${RESPONSE}" | jq -r '.display.status' | tr '[:lower:]' '[:upper:]')
  if [ "${DISPLAY_STATUS}" != "ON" ]; then
    log "ERRO" "Initial display status is not ON. Current status: ${DISPLAY_STATUS}"
    exit 1
  fi
  log "SUCCESS" "Initial display status is ON."

  local LED_STATUS
  LED_STATUS=$(echo "${RESPONSE}" | jq -r '.led.state' | tr '[:lower:]' '[:upper:]')
  if [ "${LED_STATUS}" != "ON" ]; then
    log "ERRO" "Initial LED status is not ON. Current status: ${LED_STATUS}"
    exit 1
  fi
  log "SUCCESS" "Initial LED status is ON."
}

function verify_posts() {
  log "INFO" "Starting POST API tests..."

  log "INFO" "--- Display API Tests ---"
  verify_display_toggle

  log "INFO" "--- LED API Tests ---"
  verify_led_toggle
  verify_led_brightness

  log "INFO" "--- MQTT API Tests ---"
  verify_mqtt_actions
}

# Main function to orchestrate the script execution
function main() {
  check_dependencies
  load_vars
  check_device_status
  check_initial_mode
  check_initial_display_and_led_status
  verify_gets
  verify_posts
  log "SUCCESS" "All API tests completed successfully."
}

# Call main to start the script
main "$@"
