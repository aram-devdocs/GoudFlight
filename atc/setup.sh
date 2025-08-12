#!/bin/bash

# Base Station Monitor - Setup Script
# Creates virtual environment, installs dependencies, and configures UART

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VENV_DIR="${SCRIPT_DIR}/.venv"

echo "=========================================="
echo "Base Station Monitor - Setup"
echo "=========================================="

# Detect platform
PLATFORM="unknown"
if [ -f /proc/device-tree/model ]; then
    MODEL=$(cat /proc/device-tree/model | tr -d '\0')
    if [[ "$MODEL" == *"Raspberry Pi"* ]]; then
        PLATFORM="raspberry_pi"
        echo "Detected: $MODEL"
        
        # Check if it's RPi 5
        if [[ "$MODEL" == *"Raspberry Pi 5"* ]]; then
            RPI_VERSION=5
        elif [[ "$MODEL" == *"Raspberry Pi 4"* ]]; then
            RPI_VERSION=4
        elif [[ "$MODEL" == *"Raspberry Pi 3"* ]]; then
            RPI_VERSION=3
        else
            RPI_VERSION=0
        fi
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    echo "Detected: macOS"
else
    echo "Platform: Generic Linux"
fi

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

# Configure UART for Raspberry Pi
if [ "$PLATFORM" == "raspberry_pi" ]; then
    echo ""
    echo "=========================================="
    echo "Raspberry Pi UART Configuration"
    echo "=========================================="
    
    # Check and configure UART
    CONFIG_FILE="/boot/firmware/config.txt"
    if [ ! -f "$CONFIG_FILE" ]; then
        CONFIG_FILE="/boot/config.txt"
    fi
    
    if [ -f "$CONFIG_FILE" ]; then
        echo "Checking UART configuration..."
        
        if ! grep -q "^enable_uart=1" "$CONFIG_FILE"; then
            echo "UART is not enabled. Would you like to enable it?"
            read -p "Enable UART on GPIO pins? (y/N): " -n 1 -r
            echo
            
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                # Backup config
                sudo cp "$CONFIG_FILE" "${CONFIG_FILE}.backup.$(date +%Y%m%d_%H%M%S)"
                
                # Enable UART
                echo "" | sudo tee -a "$CONFIG_FILE" > /dev/null
                echo "# Enable UART for ESP32 communication" | sudo tee -a "$CONFIG_FILE" > /dev/null
                echo "enable_uart=1" | sudo tee -a "$CONFIG_FILE" > /dev/null
                
                # Add RPi5 specific overlay if needed
                if [ "$RPI_VERSION" == "5" ]; then
                    if ! grep -q "dtoverlay=uart0-pi5" "$CONFIG_FILE"; then
                        echo "dtoverlay=uart0-pi5" | sudo tee -a "$CONFIG_FILE" > /dev/null
                        echo "Added RPi5 UART overlay"
                    fi
                fi
                
                echo "UART enabled in $CONFIG_FILE"
                REBOOT_REQUIRED=true
            fi
        else
            echo "UART is already enabled"
        fi
        
        # Check for serial console
        CMDLINE_FILE="/boot/firmware/cmdline.txt"
        if [ ! -f "$CMDLINE_FILE" ]; then
            CMDLINE_FILE="/boot/cmdline.txt"
        fi
        
        if [ -f "$CMDLINE_FILE" ]; then
            if grep -q "console=serial\|console=ttyAMA" "$CMDLINE_FILE"; then
                echo ""
                echo "Serial console is enabled, which may interfere with UART communication."
                read -p "Disable serial console? (y/N): " -n 1 -r
                echo
                
                if [[ $REPLY =~ ^[Yy]$ ]]; then
                    sudo cp "$CMDLINE_FILE" "${CMDLINE_FILE}.backup.$(date +%Y%m%d_%H%M%S)"
                    sudo sed -i 's/console=serial[0-9]*,[0-9]* *//g' "$CMDLINE_FILE"
                    sudo sed -i 's/console=ttyAMA[0-9]*,[0-9]* *//g' "$CMDLINE_FILE"
                    echo "Serial console disabled"
                    REBOOT_REQUIRED=true
                fi
            fi
        fi
    fi
    
    # Add user to dialout group
    if ! groups $USER | grep -q "dialout"; then
        echo ""
        echo "Adding user $USER to dialout group for serial port access..."
        sudo usermod -a -G dialout $USER
        echo "Added to dialout group (requires logout/login to take effect)"
    fi
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
    
    if [ "$PLATFORM" == "raspberry_pi" ]; then
        echo ""
        echo "Raspberry Pi UART ports:"
        ls -la /dev/ttyAMA* 2>/dev/null || echo "  No /dev/ttyAMA* ports found"
        ls -la /dev/ttyS* 2>/dev/null || echo "  No /dev/ttyS* ports found"
        
        if [ "$RPI_VERSION" == "5" ]; then
            echo ""
            echo "Note: After reboot, RPi5 UART will be available as /dev/ttyAMA10"
        fi
    fi
    
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
    
    # Set default serial port based on platform
    if [ "$PLATFORM" == "raspberry_pi" ]; then
        if [ "$RPI_VERSION" == "5" ]; then
            DEFAULT_PORT="/dev/ttyAMA10"
        else
            DEFAULT_PORT="/dev/ttyAMA0"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        DEFAULT_PORT="/dev/tty.usbserial"
    else
        DEFAULT_PORT="/dev/ttyUSB0"
    fi
    
    cat > "$ENV_FILE" << EOF
# Base Station Monitor Configuration
SERIAL_PORT=$DEFAULT_PORT
BAUDRATE=115200
LOG_LEVEL=INFO
EOF
    echo ".env file created with default settings"
    echo "  Default port set to: $DEFAULT_PORT"
else
    echo ""
    echo "Existing .env file found:"
    grep SERIAL_PORT "$ENV_FILE" || echo "  SERIAL_PORT not set"
    
    if [ "$PLATFORM" == "raspberry_pi" ] && [ "$RPI_VERSION" == "5" ]; then
        echo ""
        echo "For RPi5, recommended port is /dev/ttyAMA10"
        if ! grep -q "/dev/ttyAMA10" "$ENV_FILE"; then
            echo "Consider updating your .env file after reboot"
        fi
    fi
fi

echo ""
echo "=========================================="
echo "Setup completed successfully!"
echo "=========================================="
echo ""

if [ "$REBOOT_REQUIRED" == "true" ]; then
    echo "⚠️  IMPORTANT: A reboot is required for UART changes to take effect!"
    echo ""
    echo "After reboot:"
    if [ "$RPI_VERSION" == "5" ]; then
        echo "  1. The UART will be available as /dev/ttyAMA10"
        echo "  2. Update your .env file or run:"
        echo "     ./start.sh -p /dev/ttyAMA10"
    else
        echo "  1. Check available ports: ls -la /dev/ttyAMA*"
        echo "  2. Update your .env file with the correct port"
    fi
    echo ""
    read -p "Would you like to reboot now? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Rebooting..."
        sudo reboot
    else
        echo "Please remember to reboot before using the serial port."
    fi
else
    echo "To start the monitor, run:"
    echo "  ./start.sh"
    echo ""
    echo "Or manually:"
    echo "  source .venv/bin/activate"
    echo "  python main.py"
    echo ""
    
    if [ "$PLATFORM" == "raspberry_pi" ]; then
        if [ "$RPI_VERSION" == "5" ]; then
            echo "For RPi5, use port: /dev/ttyAMA10"
            echo "  ./start.sh -p /dev/ttyAMA10"
        else
            echo "Default serial port: /dev/ttyAMA0"
            echo "  ./start.sh -p /dev/ttyAMA0"
        fi
    else
        echo "Default serial port: /dev/ttyUSB0"
        echo "To use a different port, edit .env or use:"
        echo "  python main.py -p /dev/YOUR_PORT"
    fi
fi
echo ""