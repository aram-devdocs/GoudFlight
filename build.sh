#!/bin/bash

# Build script for GoudFlight monorepo
# Usage: ./build.sh [environment] or ./build.sh all

PIO=/home/aram/.platformio/penv/bin/platformio

# Check for .env file
if [ ! -f ".env" ]; then
    echo ""
    echo "============================================================"
    echo "ERROR: .env file not found!"
    echo "============================================================"
    echo ""
    echo "Please create a .env file with your ESP32 MAC addresses."
    echo "You can copy .env.template as a starting point:"
    echo ""
    echo "  cp .env.template .env"
    echo ""
    echo "Then edit .env and add your device MAC addresses:"
    echo "  HANDHELD_MAC_ADDRESS=XX:XX:XX:XX:XX:XX"
    echo "  BASE_STATION_MAC_ADDRESS=XX:XX:XX:XX:XX:XX"
    echo ""
    echo "To find your ESP32 MAC addresses, use the upload_test.sh"
    echo "script or upload a sketch that prints WiFi.macAddress()"
    echo "============================================================"
    echo ""
    exit 1
fi

# Validate .env file has required entries
if ! grep -q "HANDHELD_MAC_ADDRESS=" .env || ! grep -q "BASE_STATION_MAC_ADDRESS=" .env; then
    echo ""
    echo "============================================================"
    echo "ERROR: .env file is missing required MAC addresses!"
    echo "============================================================"
    echo ""
    echo "Your .env file must contain:"
    echo "  HANDHELD_MAC_ADDRESS=XX:XX:XX:XX:XX:XX"
    echo "  BASE_STATION_MAC_ADDRESS=XX:XX:XX:XX:XX:XX"
    echo ""
    echo "Current .env contents:"
    cat .env | grep -E "MAC_ADDRESS=" || echo "  (no MAC addresses found)"
    echo "============================================================"
    echo ""
    exit 1
fi

if [ "$1" == "all" ] || [ -z "$1" ]; then
    echo "Building all environments..."
    $PIO run
elif [ "$1" == "handheld" ]; then
    echo "Building handheld controller..."
    $PIO run -e handheld_controller
elif [ "$1" == "drone" ]; then
    echo "Building drone flight controller..."
    $PIO run -e drone_flight_controller
elif [ "$1" == "base" ]; then
    echo "Building base station..."
    $PIO run -e base_station
else
    echo "Usage: $0 [handheld|drone|base|all]"
    echo "  handheld - Build handheld controller"
    echo "  drone    - Build drone flight controller"
    echo "  base     - Build base station"
    echo "  all      - Build all environments (default)"
    exit 1
fi