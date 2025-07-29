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
# @author Nicholas Wilde, 0xb299a622
# @date 28 Jul 2025
# @version 0.1.0
#
# ==============================================================================

# --- Configuration ---
# (Can be overridden by environment variables)
: "${FTP_HOST:?FTP_HOST is not set. Please set the device IP address.}"
: "${FTP_USER:="user"}"
: "${FTP_PASSWORD:="password"}"
: "${LOCAL_DIR:="data"}" # Default local directory to sync
: "${REMOTE_DIR:="/"}"   # Default remote directory on the device

# --- Pre-flight Checks ---
# Check if lftp is installed
function check_lftp(){
  if ! command -v lftp &> /dev/null; then
    echo "Error: lftp is not installed."
    echo "Please install it to use this script."
    echo "  - Debian/Ubuntu: sudo apt-get install lftp"
    echo "  - macOS (Homebrew): brew install lftp"
    exit 1
  fi
}

# Check if the local directory exists
function check_dir(){
  if [ ! -d "$LOCAL_DIR" ]; then
    echo "Error: Local directory '$LOCAL_DIR' not found."
    echo "Please create it or specify a different LOCAL_DIR."
    exit 1
  fi
}

# --- Main Sync Logic ---
function start_sync(){
  echo "Starting FTP sync..."
  echo "  - Host: $FTP_HOST"
  echo "  - User: $FTP_USER"
  echo "  - Local Dir: $LOCAL_DIR"
  echo "  - Remote Dir: $REMOTE_DIR"
  echo ""

  # Use lftp to mirror the directory.
  # -R: Reverse mirror (uploads from local to remote)
  # --delete: Deletes files on the remote that are not present locally
  # --verbose: Shows detailed transfer information
  # --parallel=1: Disables parallel transfers to avoid overloading the ESP32
  lftp -c "
  set ftp:ssl-allow no;
  open -u '$FTP_USER','$FTP_PASSWORD' '$FTP_HOST';
  mirror -R --delete --verbose --parallel=1 '$LOCAL_DIR' '$REMOTE_DIR';
  "
}

function main(){
  check_lftp
  check_dir
  start_sync
  echo ""
  echo "Sync complete."  
}

main "@"
