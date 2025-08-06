#!/bin/bash

# Build script for GoudFlight monorepo
# Usage: ./build.sh [environment] or ./build.sh all

PIO=/home/aram/.platformio/penv/bin/platformio

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