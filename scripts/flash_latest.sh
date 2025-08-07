#!/usr/bin/env bash
# ==============================================================================
#
# flash_latest.sh
# -------
# Downloads the latest release from GitHub to the /tmp directory,
# extracts the bin files to the /tmp directory, checks if esptool is
# installed, and flashes the device.
#
# Usage: ./flash_latest.sh <SERIAL_PORT
#
# @author Nicholas Wilde, 0xb299a622                                                        
# @date 28 Jul 2025  
# @version 0.1.0
# ==============================================================================
# 
set -euo pipefail

# --- variables ---
GITHUB_REPO="nicholaswilde/frame-fi"
TMP_DIR="/tmp/frame-fi-latest"
SERIAL_PORT="${1:-/dev/ttyUSB0}" # Default to /dev/ttyUSB0 if no port is provided

# --- functions ---

# --- Constants ---
readonly BLUE=$(tput setaf 4)
readonly RED=$(tput setaf 1)
readonly YELLOW=$(tput setaf 3)
readonly RESET=$(tput sgr0)

readonly SCRIPT_NAME=$(basename "$0")

# Logging function
function log() {
  local type="$1"
  local message="$2"
  local color="$RESET"

  case "$type" in
    INFO)
      color="$BLUE"
      ;;
    WARN)
      color="$YELLOW"
      ;;
    ERRO)
      color="$RED"
      ;;
  esac

  echo -e "${color}${type}${RESET}[$(date +'%Y-%m-%d %H:%M:%S')] ${message}"
}


# Checks if a command exists.
function commandExists() {
  command -v "$1" >/dev/null 2>&1
}

function check_dependencies() {
  # --- check for dependencies ---
  if ! commandExists curl || ! commandExists grep || ! commandExists unzip; then
    log "ERRO" "Required dependencies (curl, grep, unzip) are not installed." >&2
    exit 1
  fi  
}

function download_release(){
  # --- get the latest release download URL ---
  echo "Fetching the latest release from ${GITHUB_REPO}..."
  LATEST_RELEASE_URL=$(curl -s "https://api.github.com/repos/${GITHUB_REPO}/releases/latest" |  grep "browser_download_url" | grep -o 'https://[^"]*')

  if [ -z "${LATEST_RELEASE_URL}" ]; then
    log "ERRO" "Could not find the latest release zip file." >&2
    exit 1
  fi
  # --- download and extract the release ---
  mkdir -p "${TMP_DIR}"
  log "INFO" "Downloading latest release from ${LATEST_RELEASE_URL}..."
  curl -sL "${LATEST_RELEASE_URL}" -o "${TMP_DIR}/latest_release.zip"
}

# Downloads and flashes the latest release.
function main() {
  check_dependencies  
  download_release
  
  log "INFO" "Extracting bin files to ${TMP_DIR}..."
  unzip -o "${TMP_DIR}/latest_release.zip" -d "${TMP_DIR}" "*.bin" &> /dev/null

  # --- check for esptool ---
  if ! commandExists esptool.py; then
      log "ERRO" "esptool.py is not installed. Please install it to continue." >&2
      log "ERRO" "You can typically install it with: pip install esptool"
      exit 1
  fi
  log "INFO" "esptool found."

  # --- flash the device ---
  log "INFO" "Ready to flash the device on port ${SERIAL_PORT}."
  log "INFO" "The following .bin files were extracted:"
  # find "${TMP_DIR}" -name "*.bin" -print

  # IMPORTANT: You must customize the esptool.py command below based on
  # the specific .bin files and memory addresses for your device.
  # The following is a common example.
  #
  # Example command:
          # esptool
        # --chip esp32s3
        # --baud 921600
        # --before default-reset
        # --after hard-reset write-flash -z
        # --flash-mode dio
        # --flash-freq 80m
        # --flash-size 16MB
        # 0x0 firmware.bin
  # esptool --chip esp32 --port "${SERIAL_PORT}" --baud 460800 write_flash \
  #   -z 0x1000 "${TMP_DIR}/bootloader.bin" \
  #   0x8000 "${TMP_DIR}/partitions.bin" \
  #   0x10000 "${TMP_DIR}/firmware.bin"

  log "INFO" "Please edit this script and uncomment/modify the 'esptool.py' command below."
  # esptool.py --chip esp32 --port "${SERIAL_PORT}" --baud 460800 write_flash -z \
  #    0x1000 "${TMP_DIR}/path/to/your/bootloader.bin" \
  #    0x8000 "${TMP_DIR}/path/to/your/partitions.bin" \
  #    0x10000 "${TMP_DIR}/path/to/your/firmware.bin"

  log "INFO" "--- Flashing complete (simulation) ---"
}

main "$@"
