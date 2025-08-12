#!/bin/bash

# Base Station Monitor - Start Script
# Activates virtual environment and starts the monitor

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VENV_DIR="${SCRIPT_DIR}/.venv"
ENV_FILE="${SCRIPT_DIR}/.env"

echo "=========================================="
echo "Base Station Monitor - Starting"
echo "=========================================="

# Check if virtual environment exists
if [ ! -d "$VENV_DIR" ]; then
    echo "Virtual environment not found!"
    echo "Please run ./setup.sh first"
    exit 1
fi

# Activate virtual environment
echo "Activating virtual environment..."
source "$VENV_DIR/bin/activate"

# Load environment variables if .env exists
if [ -f "$ENV_FILE" ]; then
    echo "Loading environment variables from .env..."
    export $(grep -v '^#' "$ENV_FILE" | xargs)
fi

# Set default values if not provided
SERIAL_PORT="${SERIAL_PORT:-/dev/ttyUSB0}"
BAUDRATE="${BAUDRATE:-115200}"
LOG_LEVEL="${LOG_LEVEL:-INFO}"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--port)
            SERIAL_PORT="$2"
            shift 2
            ;;
        -b|--baudrate)
            BAUDRATE="$2"
            shift 2
            ;;
        -v|--verbose)
            LOG_LEVEL="DEBUG"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  -p, --port PORT        Serial port (default: $SERIAL_PORT)"
            echo "  -b, --baudrate RATE    Baudrate (default: $BAUDRATE)"
            echo "  -v, --verbose          Enable verbose logging"
            echo "  -h, --help             Show this help message"
            echo ""
            echo "Environment variables (from .env):"
            echo "  SERIAL_PORT            Serial port to use"
            echo "  BAUDRATE               Communication baudrate"
            echo "  LOG_LEVEL              Logging level (INFO/DEBUG)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h for help"
            exit 1
            ;;
    esac
done

# Check if port exists
if [ ! -e "$SERIAL_PORT" ]; then
    echo "Warning: Serial port $SERIAL_PORT does not exist"
    echo ""
    echo "Available serial ports:"
    ls -la /dev/ttyUSB* 2>/dev/null || echo "  No /dev/ttyUSB* ports found"
    ls -la /dev/ttyACM* 2>/dev/null || echo "  No /dev/ttyACM* ports found"
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ls -la /dev/tty.usb* 2>/dev/null || echo "  No /dev/tty.usb* ports found"
    fi
    
    echo ""
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check user permissions for serial port
if [ -e "$SERIAL_PORT" ]; then
    if [ ! -r "$SERIAL_PORT" ] || [ ! -w "$SERIAL_PORT" ]; then
        echo "Error: No read/write permission for $SERIAL_PORT"
        echo ""
        echo "To fix this, you can:"
        echo "1. Add your user to the dialout group:"
        echo "   sudo usermod -a -G dialout $USER"
        echo "   (logout and login again for changes to take effect)"
        echo ""
        echo "2. Or temporarily change permissions (not recommended):"
        echo "   sudo chmod 666 $SERIAL_PORT"
        exit 1
    fi
fi

# Start the monitor
echo ""
echo "Starting Base Station Monitor..."
echo "  Port: $SERIAL_PORT"
echo "  Baudrate: $BAUDRATE"
echo "  Log Level: $LOG_LEVEL"
echo ""
echo "Press Ctrl+C to stop"
echo "=========================================="
echo ""

# Build command based on log level
if [ "$LOG_LEVEL" == "DEBUG" ]; then
    VERBOSE_FLAG="-v"
else
    VERBOSE_FLAG=""
fi

# Run the monitor
python "${SCRIPT_DIR}/main.py" -p "$SERIAL_PORT" -b "$BAUDRATE" $VERBOSE_FLAG