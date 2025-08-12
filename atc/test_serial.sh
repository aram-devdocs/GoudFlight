#!/bin/bash

echo "============================================"
echo "ESP32 Serial Connection Test Script"
echo "============================================"
echo ""

echo "1. Checking available serial ports:"
echo "-----------------------------------"
ls -la /dev/tty* | grep -E '(ttyS0|ttyAMA0|ttyUSB|ttyACM)' || echo "No standard serial ports found"
echo ""

echo "2. Checking kernel UART messages:"
echo "-----------------------------------"
dmesg | grep -i uart | tail -5
echo ""

echo "3. Checking UART configuration:"
echo "-----------------------------------"
cat /boot/config.txt | grep -E "(enable_uart|dtoverlay)" || echo "No UART config found in /boot/config.txt"
echo ""

echo "4. Checking user groups:"
echo "-----------------------------------"
echo "Current user: $USER"
echo "Groups: $(groups)"
echo ""

echo "5. Checking serial port permissions:"
echo "-----------------------------------"
ls -l /dev/ttyS0 2>/dev/null || echo "/dev/ttyS0 not found"
ls -l /dev/ttyAMA0 2>/dev/null || echo "/dev/ttyAMA0 not found"
echo ""

echo "6. Checking if ports are in use:"
echo "-----------------------------------"
sudo lsof /dev/ttyS0 2>/dev/null || echo "/dev/ttyS0 not in use or doesn't exist"
sudo lsof /dev/ttyAMA0 2>/dev/null || echo "/dev/ttyAMA0 not in use or doesn't exist"
echo ""

echo "7. Testing serial connection with Python:"
echo "-----------------------------------"
python3 << 'EOF'
import serial
import serial.tools.list_ports
import time

print("Available serial ports:")
ports = serial.tools.list_ports.comports()
for port in ports:
    print(f"  - {port.device}: {port.description}")

print("\nAttempting to connect to serial ports...")
test_ports = ['/dev/ttyS0', '/dev/ttyAMA0', '/dev/ttyUSB0', '/dev/ttyACM0']

for port_name in test_ports:
    try:
        print(f"\nTrying {port_name}...")
        ser = serial.Serial(
            port=port_name,
            baudrate=115200,
            timeout=2,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS
        )
        
        print(f"  ✓ Connected to {port_name}")
        print(f"  - Waiting for data (2 seconds)...")
        
        # Try to read data
        start_time = time.time()
        data_received = False
        while time.time() - start_time < 2:
            if ser.in_waiting > 0:
                data = ser.readline()
                print(f"  - Received: {data}")
                data_received = True
                break
        
        if not data_received:
            print(f"  - No data received from {port_name}")
            # Try sending a status request
            print(f"  - Sending status request...")
            ser.write(b's')
            time.sleep(0.5)
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                print(f"  - Response: {data}")
            else:
                print(f"  - No response to status request")
        
        ser.close()
        
    except serial.SerialException as e:
        print(f"  ✗ Failed to open {port_name}: {e}")
    except Exception as e:
        print(f"  ✗ Error with {port_name}: {e}")
EOF

echo ""
echo "8. Checking GPIO pin status (if available):"
echo "-----------------------------------"
if command -v gpio &> /dev/null; then
    gpio readall 2>/dev/null | grep -E "(GPIO14|GPIO15|TXD|RXD)" || echo "Could not read GPIO status"
else
    echo "gpio command not found (install with: sudo apt-get install wiringpi)"
fi

echo ""
echo "============================================"
echo "Test complete!"
echo "============================================"
echo ""
echo "TROUBLESHOOTING NOTES:"
echo "- The ESP32 should be connected: TX(GPIO17)→Pi RX(GPIO15/Pin10), RX(GPIO18)→Pi TX(GPIO14/Pin8), GND→GND"
echo "- If /dev/ttyS0 doesn't work, try /dev/ttyAMA0"
echo "- If permission denied, run: sudo chmod 666 /dev/ttyS0"
echo "- Make sure the ESP32 is powered on and running the base station firmware"
echo ""