#!/bin/bash

# This script downloads the latest release of frame-fi and flashes it to a LILYGO T-Dongle S3.

# --- Configuration ---
REPO="nicholaswilde/frame-fi"
DEVICE_PORT="/dev/ttyACM0" # Change this to your device's port

# --- Check for dependencies ---
command -v esptool.py >/dev/null 2>&1 || { echo >&2 "esptool.py is not installed. Please install it with 'pip install esptool'."; exit 1; }
command -v wget >/dev/null 2>&1 || { echo >&2 "wget is not installed. Please install it."; exit 1; }
command -v unzip >/dev/null 2>&1 || { echo >&2 "unzip is not installed. Please install it."; exit 1; }
command -v jq >/dev/null 2>&1 || { echo >&2 "jq is not installed. Please install it."; exit 1; }


# --- Get the latest release URL ---
echo "Fetching the latest release..."
LATEST_RELEASE_URL=$(wget -qO- "https://api.github.com/repos/$REPO/releases/latest" | jq -r '.zipball_url')

if [ -z "$LATEST_RELEASE_URL" ]; then
    echo "Could not find the latest release URL. Exiting."
    exit 1
fi

# --- Download the latest release ---
echo "Downloading the latest release from $LATEST_RELEASE_URL..."
wget -q --show-progress -O latest_release.zip "$LATEST_RELEASE_URL"

# --- Unzip the release ---
echo "Unzipping the release..."
unzip -o latest_release.zip -d latest_release
# The files are in a subdirectory, so we need to find it
EXTRACTED_DIR=$(ls -d latest_release/*/)

# --- Flash the device ---
echo "Flashing the device..."
esptool.py --chip esp32s3 --port "$DEVICE_PORT" --baud 921600 \
    --before default_reset --after hard_reset write_flash \
    -z --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x0000 "${EXTRACTED_DIR}bootloader.bin" \
    0x8000 "${EXTRACTED_DIR}partitions.bin" \
    0x10000 "${EXTRACTED_DIR}firmware.bin"

# --- Clean up ---
echo "Cleaning up..."
rm -rf latest_release latest_release.zip

echo "Flashing complete!"
