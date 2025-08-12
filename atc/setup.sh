#!/bin/bash

# Base Station Monitor - Setup Script
# Creates virtual environment and installs dependencies

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VENV_DIR="${SCRIPT_DIR}/.venv"

echo "=========================================="
echo "Base Station Monitor - Setup"
echo "=========================================="

# Check Python version
echo "Checking Python version..."
if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
elif command -v python &> /dev/null; then
    PYTHON_CMD="python"
else
    echo "Error: Python not found. Please install Python 3.7 or higher."
    exit 1
fi

# Get Python version
PYTHON_VERSION=$($PYTHON_CMD --version 2>&1 | grep -oE '[0-9]+\.[0-9]+')
echo "Found Python $PYTHON_VERSION"

# Check minimum version (3.7)
MIN_VERSION="3.7"
if [ "$(printf '%s\n' "$MIN_VERSION" "$PYTHON_VERSION" | sort -V | head -n1)" != "$MIN_VERSION" ]; then
    echo "Error: Python $MIN_VERSION or higher is required (found $PYTHON_VERSION)"
    exit 1
fi

# Create virtual environment
if [ -d "$VENV_DIR" ]; then
    echo "Virtual environment already exists at $VENV_DIR"
    read -p "Do you want to recreate it? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Removing existing virtual environment..."
        rm -rf "$VENV_DIR"
    else
        echo "Using existing virtual environment"
    fi
fi

if [ ! -d "$VENV_DIR" ]; then
    echo "Creating virtual environment..."
    $PYTHON_CMD -m venv "$VENV_DIR"
    echo "Virtual environment created at $VENV_DIR"
fi

# Activate virtual environment
echo "Activating virtual environment..."
source "$VENV_DIR/bin/activate"

# Upgrade pip
echo "Upgrading pip..."
pip install --upgrade pip

# Install requirements
echo "Installing requirements..."
if [ -f "${SCRIPT_DIR}/requirements.txt" ]; then
    pip install -r "${SCRIPT_DIR}/requirements.txt"
    echo "Requirements installed successfully"
else
    echo "Warning: requirements.txt not found"
fi

# Check serial ports
echo ""
echo "Checking available serial ports..."
if command -v ls &> /dev/null; then
    echo "USB serial ports:"
    ls -la /dev/ttyUSB* 2>/dev/null || echo "  No /dev/ttyUSB* ports found"
    echo ""
    echo "ACM serial ports:"
    ls -la /dev/ttyACM* 2>/dev/null || echo "  No /dev/ttyACM* ports found"
    echo ""
    echo "Platform serial ports:"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ls -la /dev/tty.usb* 2>/dev/null || echo "  No /dev/tty.usb* ports found"
    fi
fi

# Create .env file if it doesn't exist
ENV_FILE="${SCRIPT_DIR}/.env"
if [ ! -f "$ENV_FILE" ]; then
    echo ""
    echo "Creating default .env file..."
    cat > "$ENV_FILE" << EOF
# Base Station Monitor Configuration
SERIAL_PORT=/dev/ttyUSB0
BAUDRATE=115200
LOG_LEVEL=INFO
EOF
    echo ".env file created with default settings"
fi

echo ""
echo "=========================================="
echo "Setup completed successfully!"
echo "=========================================="
echo ""
echo "To start the monitor, run:"
echo "  ./start.sh"
echo ""
echo "Or manually:"
echo "  source .venv/bin/activate"
echo "  python main.py"
echo ""
echo "Default serial port: /dev/ttyUSB0"
echo "To use a different port, edit .env or use:"
echo "  python main.py -p /dev/YOUR_PORT"
echo ""