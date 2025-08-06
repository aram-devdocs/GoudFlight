#!/bin/bash

# Serial monitor script for GoudFlight monorepo
# Usage: ./monitor.sh [environment]

PIO=/home/aram/.platformio/penv/bin/platformio

if [ -z "$1" ]; then
    echo "Error: Please specify which board to monitor"
    echo "Usage: $0 [handheld|drone|base]"
    exit 1
fi

case "$1" in
    handheld)
        echo "Monitoring handheld controller..."
        $PIO device monitor -e handheld_controller
        ;;
    drone)
        echo "Monitoring drone flight controller..."
        $PIO device monitor -e drone_flight_controller
        ;;
    base)
        echo "Monitoring base station..."
        $PIO device monitor -e base_station
        ;;
    *)
        echo "Error: Unknown environment '$1'"
        echo "Usage: $0 [handheld|drone|base]"
        echo "  handheld - Monitor handheld controller"
        echo "  drone    - Monitor drone flight controller"
        echo "  base     - Monitor base station"
        exit 1
        ;;
esac