#!/bin/bash

# Upload script for GoudFlight monorepo
# Usage: ./upload.sh [environment]

PIO=/home/aram/.platformio/penv/bin/platformio

if [ -z "$1" ]; then
    echo "Error: Please specify which board to upload to"
    echo "Usage: $0 [handheld|drone|base]"
    exit 1
fi

case "$1" in
    handheld)
        echo "Uploading to handheld controller..."
        $PIO run -e handheld_controller -t upload
        ;;
    drone)
        echo "Uploading to drone flight controller..."
        $PIO run -e drone_flight_controller -t upload
        ;;
    base)
        echo "Uploading to base station..."
        $PIO run -e base_station -t upload
        ;;
    *)
        echo "Error: Unknown environment '$1'"
        echo "Usage: $0 [handheld|drone|base]"
        echo "  handheld - Upload to handheld controller"
        echo "  drone    - Upload to drone flight controller"
        echo "  base     - Upload to base station"
        exit 1
        ;;
esac