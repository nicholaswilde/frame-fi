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
# @version 0.2.0
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
  start_sync
  log "INFO" "--- Sync complete. ---"  
}

main "@"
