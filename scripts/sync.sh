#!/usr/bin/env bash

# ==============================================================================
#
# sync.sh
# -------
# This script syncs a local directory to the ESP32-S3 device via FTP.
# It uses lftp to mirror the local directory to the remote target.
#
# Dependencies:
#   - lftp: A sophisticated command-line FTP client.
#     Install on Debian/Ubuntu: sudo apt install lftp
#     Install on macOS (Homebrew): brew install lftp
#
# Usage:
#   1. Set the environment variables below or pass them on the command line.
#   2. Run the script: ./scripts/sync.sh
#
# Example:
#   FTP_HOST="192.168.1.123" LOCAL_DIR="./my_pictures" ./scripts/sync.sh
#
# @author Nicholas Wilde, 0xb299a622                                                        │
# @date 28 Jul 2025  
# @version 0.3.0
#
# ==============================================================================

# Get the absolute path of the directory where the script is located.
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Source the .env file
[ -f "${SCRIPT_DIR}/.env" ] && source "${SCRIPT_DIR}/.env"

# --- Configuration ---
# (Can be overridden by environment variables)
: "${FTP_HOST:?FTP_HOST is not set. Please set the device IP address.}"
: "${FTP_USER:="user"}"
: "${FTP_PASSWORD:="password"}"
: "${LOCAL_DIR:="data"}" # Default local directory to sync
: "${REMOTE_DIR:="/"}"   # Default remote directory on the device

# --- Constants ---
readonly BLUE=$(tput setaf 4)
readonly RED=$(tput setaf 1)
readonly YELLOW=$(tput setaf 3)
readonly RESET=$(tput sgr0)

readonly SCRIPT_NAME=$(basename "$0")

# --- functions ---

# Logging function
function log() {
  local type="$1"
  local message="$2"
  local color="$RESET"

  case "$type" in
    INFO)
      color="$BLUE";;
    WARN)
      color="$YELLOW";;
    ERRO)
      color="$RED";;
  esac

  echo -e "${color}${type}${RESET}[$(date +'%Y-%m-%d %H:%M:%S')] ${message}"
}

# --- Pre-flight Checks ---
# Check if lftp is installed
function check_lftp(){
  if ! command -v lftp &> /dev/null; then
    log "ERRO" "lftp is not installed."
    log "ERRO" "Please install it to use this script."
    log "ERRO" "  - Debian/Ubuntu: sudo apt install lftp"
    log "ERRO" "  - macOS (Homebrew): brew install lftp"
    exit 1
  fi
}

# Check if the local directory exists
function check_dir(){
  if [ ! -d "${SCRIPT_DIR}/$LOCAL_DIR" ]; then
    log "ERRO" "Local directory '${SCRIPT_DIR}/$LOCAL_DIR' not found."
    log "ERRO" "Please create it or specify a different LOCAL_DIR."
    exit 1
  fi
}

# Check if curl is installed
function check_curl(){
  if ! command -v curl &> /dev/null; then
    log "ERRO" "curl is not installed."
    log "ERRO" "Please install it to use this script for device checks."
    log "ERRO" "  - Debian/Ubuntu: sudo apt install curl"
    log "ERRO" "  - macOS (Homebrew): brew install curl"
    exit 1
  fi
}

# Check if the device is online by pinging it
function check_device_online(){
  log "INFO" "Checking if device at $FTP_HOST is online..."
  if ping -c 1 -W 1 "$FTP_HOST" &> /dev/null; then
    log "INFO" "Device is online."
  else
    log "ERRO" "Device at $FTP_HOST is offline or unreachable."
    exit 1
  fi
}

# Check the device's current mode via its web API
function check_device_mode(){
  log "INFO" "Checking device mode..."
  local mode_url="http://$FTP_HOST/"
  local response=$(curl -s "$mode_url")

  if [ $? -ne 0 ]; then
    log "ERRO" "Failed to connect to device API at $mode_url. Is the web server running?"
    exit 1
  fi

  if echo "$response" | grep -q '"mode":"Application (FTP Server)"'; then
    log "INFO" "Device is in FTP mode."
  elif echo "$response" | grep -q '"mode":"USB MSC"'; then
    log "ERRO" "Device is in USB MSC mode. Please switch to FTP mode."
    log "ERRO" "You can switch by pressing the button on the device or by sending a POST request to http://$FTP_HOST/mode/ftp"
    exit 1
  else
    log "ERRO" "Could not determine device mode from API response: $response"
    exit 1
  fi
}

# --- Main Sync Logic ---
function start_sync(){
  log "INFO" "Starting FTP sync..."
  log "INFO" "  - Host: $FTP_HOST"
  log "INFO" "  - User: $FTP_USER"
  log "INFO" "  - Local Dir: $LOCAL_DIR"
  log "INFO" "  - Remote Dir: $REMOTE_DIR"

  # Use lftp to mirror the directory.
  # -R: Reverse mirror (uploads from local to remote)
  # --delete: Deletes files on the remote that are not present locally
  # --verbose: Shows detailed transfer information
  # --parallel=1: Disables parallel transfers to avoid overloading the ESP32
  # --no-perms: Don't set file permissions
  # --only-missing: download only missing files
  lftp -c "
  set ftp:ssl-allow no;
  open -u '$FTP_USER','$FTP_PASSWORD' '$FTP_HOST';
  mirror -R --delete --verbose --only-missing --no-perms --parallel=1 '$LOCAL_DIR' '$REMOTE_DIR';
  "
}

function main(){
  check_lftp
  check_dir
  check_curl
  check_device_online
  check_device_mode
  start_sync
  log "INFO" "--- Sync complete. ---"  
}

main "@"
