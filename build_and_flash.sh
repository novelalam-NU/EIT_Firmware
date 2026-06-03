#!/bin/bash
# Exit immediately if any command exits with a non-zero status
set -e

# Path to ESP-IDF export script
IDF_EXPORT_PATH="$HOME/esp-idf/export.sh"

# Source the ESP-IDF environment
if [ -f "$IDF_EXPORT_PATH" ]; then
    echo "Sourcing ESP-IDF environment from $IDF_EXPORT_PATH..."
    source "$IDF_EXPORT_PATH"
else
    echo "Error: ESP-IDF export script not found at $IDF_EXPORT_PATH"
    exit 1
fi

# Determine serial port (defaulting to /dev/cu.usbmodem101 if not specified)
PORT="${1:-/dev/cu.usbmodem101}"

echo "Building EIT Firmware..."
# Temporarily disable exit-on-error to check build status and auto-clean if needed
set +e
idf.py build
BUILD_STATUS=$?
set -e

if [ $BUILD_STATUS -ne 0 ]; then
    echo "----------------------------------------------------------------------"
    echo "Build failed. Trying 'idf.py fullclean' to resolve environment/Python version conflicts..."
    echo "----------------------------------------------------------------------"
    idf.py fullclean
    echo "Retrying build..."
    idf.py build
fi

echo "Flashing EIT Firmware to port: $PORT..."
idf.py -p "$PORT" flash

echo "Successfully built and flashed!"
