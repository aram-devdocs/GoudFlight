# ESP32 to Raspberry Pi Wiring Guide

## Overview
This guide covers the hardware connections needed to establish UART communication between the ESP32-S3 base station and Raspberry Pi for the Python monitoring application.

## Required Components
- ESP32-S3 DevKit (Base Station)
- Raspberry Pi (3/4/5 or Zero W)
- 3x Dupont wires (female-to-female or female-to-male depending on your setup)
- Optional: Logic level shifter (not needed for 3.3V tolerant RPi GPIO)

## Pin Connections

### ESP32-S3 Base Station Pins
| ESP32 Pin | Function | GPIO Number | Wire Color (suggested) |
|-----------|----------|-------------|----------------------|
| TX        | UART2 TX | GPIO 17     | Yellow/Orange        |
| RX        | UART2 RX | GPIO 18     | Green                |
| GND       | Ground   | GND         | Black                |

### Raspberry Pi GPIO Pins
| RPi Pin | Function | Physical Pin | BCM Pin | Wire From ESP32 |
|---------|----------|--------------|---------|-----------------|
| RX      | UART RX  | Pin 10       | GPIO 15 | ESP32 TX (GPIO17) |
| TX      | UART TX  | Pin 8        | GPIO 14 | ESP32 RX (GPIO18) |
| GND     | Ground   | Pin 6/9/14/20/25/30/34/39 | - | ESP32 GND |

## Wiring Diagram

```
    ESP32-S3                          Raspberry Pi
    ┌─────────────┐                   ┌─────────────┐
    │             │                   │             │
    │  GPIO17(TX) ├───────────────────┤ GPIO15(RX)  │
    │             │     (Yellow)      │   Pin 10    │
    │             │                   │             │
    │  GPIO18(RX) ├───────────────────┤ GPIO14(TX)  │
    │             │     (Green)       │   Pin 8     │
    │             │                   │             │
    │     GND     ├───────────────────┤    GND      │
    │             │     (Black)       │   Pin 6     │
    └─────────────┘                   └─────────────┘
```

## Raspberry Pi Pin Layout (40-pin header)

```
                     3.3V ● ● 5V
           (SDA) GPIO 2  ● ● 5V
           (SCL) GPIO 3  ● ● GND
                GPIO 4  ● ● GPIO 14 (TX)  ← Connect to ESP32 RX
                    GND ● ● GPIO 15 (RX)  ← Connect to ESP32 TX
                GPIO 17 ● ● GPIO 18
                GPIO 27 ● ● GND
                GPIO 22 ● ● GPIO 23
                   3.3V ● ● GPIO 24
          (MOSI) GPIO 10 ● ● GND
          (MISO) GPIO 9  ● ● GPIO 25
          (SCLK) GPIO 11 ● ● GPIO 8
                    GND ● ● GPIO 7
```

## Setup Instructions

### 1. Hardware Connection
1. Power off both devices before connecting
2. Connect the three wires as shown in the wiring diagram
3. Double-check connections:
   - ESP32 TX → RPi RX
   - ESP32 RX → RPi TX  
   - ESP32 GND → RPi GND
4. Power on both devices

### 2. Raspberry Pi Configuration

#### Enable UART on Raspberry Pi
```bash
# Edit config.txt
sudo nano /boot/config.txt

# Add or uncomment these lines:
enable_uart=1
dtoverlay=disable-bt  # Disables Bluetooth to free up UART

# Save and exit (Ctrl+X, Y, Enter)
```

#### Disable Serial Console (if needed)
```bash
sudo raspi-config
# Navigate to: Interface Options → Serial Port
# - Login shell over serial: NO
# - Serial port hardware: YES
# Save and reboot
```

#### Verify Serial Port
```bash
# Check if serial port is available
ls -l /dev/ttyS0  # For Pi 3/4/5
ls -l /dev/ttyAMA0  # Alternative port name

# Check permissions
groups $USER  # Should include 'dialout' group

# If not in dialout group:
sudo usermod -a -G dialout $USER
# Logout and login for changes to take effect
```

### 3. ESP32 Configuration
The ESP32 firmware is already configured with:
- UART2 on GPIO17 (TX) and GPIO18 (RX)
- Baudrate: 115200
- 8 data bits, no parity, 1 stop bit (8N1)

### 4. Testing the Connection

#### On Raspberry Pi:
```bash
# Navigate to the monitoring app directory
cd /home/aram/dev/GoudFlight/atc

# Install dependencies
./setup.sh

# Test the connection
./start.sh -p /dev/ttyS0 -b 115200

# Or use screen for basic testing
screen /dev/ttyS0 115200

# Or minicom
minicom -D /dev/ttyS0 -b 115200
```

#### Expected Output:
When properly connected, you should see:
- Heartbeat messages every 5 seconds
- JSON formatted telemetry data
- System status updates

## Troubleshooting

### No Communication
1. **Check wiring**: Ensure TX→RX and RX→TX crossover
2. **Verify voltage levels**: Both devices use 3.3V logic
3. **Check permissions**: `sudo chmod 666 /dev/ttyS0` (temporary fix)
4. **Verify port name**: Try `/dev/ttyAMA0` if `/dev/ttyS0` doesn't work
5. **Check if port is in use**: `sudo lsof /dev/ttyS0`

### Garbled Data
1. **Baudrate mismatch**: Ensure both sides use 115200
2. **Ground connection**: Verify solid ground connection
3. **Interference**: Keep wires short and away from power supplies

### Permission Denied
```bash
# Add user to dialout group permanently
sudo usermod -a -G dialout $USER
# Logout and login again

# Or temporary fix (until reboot)
sudo chmod 666 /dev/ttyS0
```

### Serial Port Not Found
```bash
# List all serial devices
ls -l /dev/tty*

# Check kernel messages for serial ports
dmesg | grep tty

# Verify UART is enabled
cat /boot/config.txt | grep uart
```

## Circuit Board Integration Notes

For PCB design:
1. **Trace Requirements**:
   - Signal traces: 10-15 mil width minimum
   - Keep UART traces short (<6 inches)
   - Route TX/RX as differential pair if possible

2. **Protection Components** (optional but recommended):
   - Series resistors (100-330Ω) on TX/RX lines
   - ESD protection diodes
   - Pull-up resistor (10kΩ) on RX lines

3. **Connector Options**:
   - JST-XH 4-pin connector
   - Pin headers with keying
   - Spring-loaded pogo pins for production

4. **Power Considerations**:
   - Separate power supplies recommended
   - Common ground is essential
   - Add decoupling capacitors near connectors

## Python Monitor Commands

Once connected, the Python monitor supports:
- `s` - Request status
- `t` - Request telemetry
- `r` - Reset ESP32
- `q` - Quit monitor

## Additional Resources
- [Raspberry Pi UART Documentation](https://www.raspberrypi.org/documentation/configuration/uart.md)
- [ESP32 UART Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- [Python pySerial Documentation](https://pyserial.readthedocs.io/)