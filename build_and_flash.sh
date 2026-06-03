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

# Check for --clean flag
CLEAN=false
if [ "$1" = "--clean" ]; then
    CLEAN=true
    shift
fi

# Auto-detect macOS USB serial port (e.g. cu.usbmodem* or cu.usbserial*)
DETECTED_PORT=$(ls -1 /dev/cu.usbmodem* /dev/cu.usbserial* 2>/dev/null | head -n 1)

if [ -n "$DETECTED_PORT" ]; then
    echo "Auto-detected USB port: $DETECTED_PORT"
    PORT="${1:-$DETECTED_PORT}"
else
    echo "Warning: No USB serial port automatically detected."
    PORT="${1:-/dev/cu.usbmodem101}"
    echo "Defaulting to: $PORT"
fi

if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    idf.py fullclean
fi

echo "Building EIT Firmware..."
idf.py build

echo "Flashing EIT Firmware to port: $PORT..."
idf.py -p "$PORT" flash

echo "Successfully built and flashed!"
