#!/usr/bin/env bash
################################################################################
#
# test-ftp.sh
# ----------------
# Tests the FTP functionality of the FrameFi device by uploading, downloading,
# and verifying a test file.
#
# @author Nicholas Wilde, 0xb299a622
# @date 17 Aug 2025
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

TEST_FILE_NAME="test_ftp_file.txt"
readonly TEST_FILE_NAME

TEST_FILE_CONTENT="This is a test file for FTP functionality."
readonly TEST_FILE_CONTENT

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
  if ! command -v lftp &> /dev/null; then
    log "ERRO" "lftp could not be found. Please install it."
    exit 1
  fi
  log "SUCCESS" "Dependencies checked."
}

function load_vars() {
  local ENV_FILE="$(dirname "$0")/.env"

  if [ ! -f "${ENV_FILE}" ]; then
    log "ERRO" "Environment file not found: ${ENV_FILE}"
    log "ERRO" "Please create it from .env.tmpl and ensure FTP_HOST, FTP_USER, FTP_PASSWORD are set."
    exit 1
  fi

  source "${ENV_FILE}"

  if [ -z "${FTP_HOST}" ] || [ -z "${FTP_USER}" ] || [ -z "${FTP_PASSWORD}" ]; then
    log "ERRO" "FTP_HOST, FTP_USER, or FTP_PASSWORD not set in ${ENV_FILE}"
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

# Check device mode
function check_device_mode() {
  log "INFO" "Verifying device mode is FTP..."
  local RESPONSE
  RESPONSE=$(curl -s "http://${FTP_HOST}/")
  local CURRENT_MODE
  CURRENT_MODE=$(echo "${RESPONSE}" | jq -r '.mode')

  if [ "${CURRENT_MODE}" != "Application (FTP Server)" ]; then
    log "ERRO" "Device is not in FTP mode. Current mode: ${CURRENT_MODE}"
    log "ERRO" "Please set the device to FTP mode before running tests (e.g., curl -X POST http://${FTP_HOST}/mode/ftp)."
    exit 1
  fi
  log "SUCCESS" "Device is in FTP mode."
}

# Function to upload a test file
function upload_test_file() {
  log "INFO" "Creating local test file: ${TEST_FILE_NAME}"
  echo "${TEST_FILE_CONTENT}" > "${TEST_FILE_NAME}"

  log "INFO" "Uploading ${TEST_FILE_NAME} to FTP server at ${FTP_HOST}..."
  lftp -c "
  set ftp:ssl-allow no;
  open -u "${FTP_USER}","${FTP_PASSWORD}" "${FTP_HOST}";
  put -O / "${TEST_FILE_NAME}";
  "
  if [ $? -ne 0 ]; then
    log "ERRO" "Failed to upload ${TEST_FILE_NAME}."
    exit 1
  fi
  log "SUCCESS" "Successfully uploaded ${TEST_FILE_NAME}."
}

# Function to download a test file
function download_test_file() {
  log "INFO" "Downloading ${TEST_FILE_NAME} from FTP server at ${FTP_HOST}..."
  lftp -c "
  set ftp:ssl-allow no;
  set xfer:clobber true;
  open -u "${FTP_USER}","${FTP_PASSWORD}" "${FTP_HOST}";
  get -O . "${TEST_FILE_NAME}";
  "
  if [ $? -ne 0 ]; then
    log "ERRO" "Failed to download ${TEST_FILE_NAME}."
    exit 1
  fi
  log "SUCCESS" "Successfully downloaded ${TEST_FILE_NAME}."
}

# Function to verify the content of the downloaded file
function verify_downloaded_file() {
  log "INFO" "Verifying content of downloaded file: ${TEST_FILE_NAME}"
  local DOWNLOADED_CONTENT
  DOWNLOADED_CONTENT=$(cat "${TEST_FILE_NAME}")

  if [ "${DOWNLOADED_CONTENT}" == "${TEST_FILE_CONTENT}" ]; then
    log "SUCCESS" "Content of ${TEST_FILE_NAME} matches expected content."
  else
    log "ERRO" "Content mismatch for ${TEST_FILE_NAME}."
    log "ERRO" "Expected: '${TEST_FILE_CONTENT}'"
    log "ERRO" "Got:      '${DOWNLOADED_CONTENT}'"
    exit 1
  fi
}

# Function to delete the test file from the FTP server
# Function to delete the test file from the FTP server
function delete_test_file_remote() {
  log "INFO" "Attempting to delete ${TEST_FILE_NAME} from FTP server at ${FTP_HOST} for cleanup..."
  local LFTP_OUTPUT
  local LFTP_EXIT_CODE

  # Execute lftp command, capture stderr and stdout
  LFTP_OUTPUT=$(lftp -c "set ftp:ssl-allow no; open -u \"${FTP_USER}\",\"${FTP_PASSWORD}\" \"${FTP_HOST}\"; rm \"${TEST_FILE_NAME}\";")
  LFTP_EXIT_CODE=$?

  if [ ${LFTP_EXIT_CODE} -ne 0 ]; then
    if echo "${LFTP_OUTPUT}" | grep -q "550 ${TEST_FILE_NAME}: No such file or directory"; then
      log "WARN" "File ${TEST_FILE_NAME} not found on remote during cleanup. This is expected if it didn't exist."
    else
      log "ERRO" "Failed to delete ${TEST_FILE_NAME} from remote. Error: ${LFTP_OUTPUT}"
      exit 1
    fi
  else
    log "SUCCESS" "Successfully deleted ${TEST_FILE_NAME} from remote."
  fi
}

# Function to clean up local test file
function cleanup_local_file() {
  if [ -f "${TEST_FILE_NAME}" ]; then
    log "INFO" "Cleaning up local test file: ${TEST_FILE_NAME}"
    rm "${TEST_FILE_NAME}"
    log "SUCCESS" "Successfully removed local test file."
  fi
}

# Main function to orchestrate the script execution
function main() {
  local START_TIME
  START_TIME=$(date +%s)
  log "INFO" "=== Starting "$0" ==="

  check_dependencies
  load_vars
  check_device_status
  check_device_mode

  # Ensure clean state before starting tests
  delete_test_file_remote
  cleanup_local_file

  upload_test_file
  download_test_file
  verify_downloaded_file
  delete_test_file_remote
  cleanup_local_file

  local END_TIME
  END_TIME=$(date +%s)
  local DURATION=$((END_TIME - START_TIME))
  log "INFO" "Script finished in $(($DURATION / 60)) minutes and $(($DURATION % 60)) seconds."
  log "SUCCESS" "=== All FTP tests completed successfully. ==="
}

# Call main to start the script
main "$@"
