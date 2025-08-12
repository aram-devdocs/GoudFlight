#!/usr/bin/env python3
"""
Base Station Monitor - MVP Python application for monitoring ESP32 base station
Communicates via UART over GPIO pins for circuit board integration
"""

import serial
import json
import time
import threading
import sys
import argparse
from datetime import datetime
from typing import Dict, Any, Optional
from queue import Queue
import logging

class BaseStationMonitor:
    """Main monitoring class for ESP32 base station communication"""
    
    def __init__(self, port: str = '/dev/ttyUSB0', baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_conn: Optional[serial.Serial] = None
        self.running = False
        self.rx_queue = Queue()
        self.tx_queue = Queue()
        self.last_heartbeat = None
        self.connection_status = "DISCONNECTED"
        
        # Setup logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger(__name__)
        
        # Data storage for monitoring
        self.telemetry_data = {
            'timestamp': None,
            'system_status': 'UNKNOWN',
            'esp_now_connected': False,
            'remote_devices': 0,
            'signal_strength': 0,
            'battery_voltage': 0.0,
            'uptime_ms': 0,
            'free_heap': 0,
            'cpu_usage': 0.0
        }
    
    def connect(self) -> bool:
        """Establish serial connection with ESP32"""
        try:
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1.0,
                write_timeout=1.0
            )
            self.serial_conn.reset_input_buffer()
            self.serial_conn.reset_output_buffer()
            
            self.connection_status = "CONNECTED"
            self.logger.info(f"Connected to {self.port} at {self.baudrate} baud")
            return True
            
        except serial.SerialException as e:
            self.logger.error(f"Failed to connect: {e}")
            self.connection_status = "ERROR"
            return False
    
    def disconnect(self):
        """Close serial connection"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            self.connection_status = "DISCONNECTED"
            self.logger.info("Disconnected from serial port")
    
    def send_command(self, command: str, params: Dict[str, Any] = None) -> bool:
        """Send command to ESP32"""
        if not self.serial_conn or not self.serial_conn.is_open:
            self.logger.error("Serial connection not established")
            return False
        
        message = {
            'cmd': command,
            'timestamp': time.time()
        }
        
        if params:
            message['params'] = params
        
        try:
            json_str = json.dumps(message) + '\n'
            self.serial_conn.write(json_str.encode('utf-8'))
            self.logger.debug(f"Sent: {command}")
            return True
            
        except serial.SerialException as e:
            self.logger.error(f"Failed to send command: {e}")
            return False
    
    def rx_thread(self):
        """Thread for receiving data from ESP32"""
        self.logger.info("RX thread started")
        
        while self.running:
            if not self.serial_conn or not self.serial_conn.is_open:
                time.sleep(0.1)
                continue
            
            try:
                if self.serial_conn.in_waiting > 0:
                    line = self.serial_conn.readline()
                    if line:
                        decoded = line.decode('utf-8').strip()
                        if decoded:
                            self.process_received_data(decoded)
                            
            except serial.SerialException as e:
                self.logger.error(f"RX error: {e}")
                self.connection_status = "ERROR"
                
            except Exception as e:
                self.logger.error(f"Unexpected RX error: {e}")
            
            time.sleep(0.01)  # Small delay to prevent CPU hogging
        
        self.logger.info("RX thread stopped")
    
    def process_received_data(self, data: str):
        """Process received JSON data from ESP32"""
        try:
            msg = json.loads(data)
            msg_type = msg.get('type', 'UNKNOWN')
            
            if msg_type == 'TELEMETRY':
                self.update_telemetry(msg.get('data', {}))
                
            elif msg_type == 'STATUS':
                self.telemetry_data['system_status'] = msg.get('status', 'UNKNOWN')
                self.logger.info(f"System status: {self.telemetry_data['system_status']}")
                
            elif msg_type == 'HEARTBEAT':
                self.last_heartbeat = time.time()
                self.logger.debug("Heartbeat received")
                
            elif msg_type == 'EVENT':
                self.logger.info(f"Event: {msg.get('event', 'UNKNOWN')} - {msg.get('data', {})}")
                
            elif msg_type == 'ERROR':
                self.logger.error(f"ESP32 Error: {msg.get('message', 'Unknown error')}")
                
            else:
                self.logger.debug(f"Received: {msg}")
                
        except json.JSONDecodeError as e:
            # Not JSON, might be plain text debug output
            if data.startswith('[') or data.startswith('DEBUG:') or data.startswith('INFO:'):
                self.logger.debug(f"ESP32 Log: {data}")
            else:
                self.logger.warning(f"Invalid JSON received: {data}")
    
    def update_telemetry(self, data: Dict[str, Any]):
        """Update telemetry data"""
        self.telemetry_data['timestamp'] = datetime.now().isoformat()
        
        for key, value in data.items():
            if key in self.telemetry_data:
                self.telemetry_data[key] = value
        
        self.logger.debug(f"Telemetry updated: {self.telemetry_data}")
    
    def heartbeat_thread(self):
        """Thread for sending periodic heartbeat/status requests"""
        self.logger.info("Heartbeat thread started")
        
        while self.running:
            # Send heartbeat every 5 seconds
            self.send_command('PING')
            
            # Request telemetry every 2 seconds
            time.sleep(2)
            if self.running:
                self.send_command('GET_TELEMETRY')
            
            # Check connection health
            if self.last_heartbeat and (time.time() - self.last_heartbeat) > 10:
                self.logger.warning("No heartbeat received for 10 seconds")
                self.connection_status = "TIMEOUT"
            
            time.sleep(3)
        
        self.logger.info("Heartbeat thread stopped")
    
    def display_status(self):
        """Display current status in terminal"""
        print("\033[2J\033[H")  # Clear screen
        print("=" * 60)
        print("BASE STATION MONITOR - MVP")
        print("=" * 60)
        print(f"Connection: {self.connection_status}")
        print(f"Port: {self.port} @ {self.baudrate} baud")
        print("-" * 60)
        print("TELEMETRY DATA:")
        print(f"  Timestamp:        {self.telemetry_data['timestamp']}")
        print(f"  System Status:    {self.telemetry_data['system_status']}")
        print(f"  ESP-NOW:          {'Connected' if self.telemetry_data['esp_now_connected'] else 'Disconnected'}")
        print(f"  Remote Devices:   {self.telemetry_data['remote_devices']}")
        print(f"  Signal Strength:  {self.telemetry_data['signal_strength']} dBm")
        print(f"  Battery:          {self.telemetry_data['battery_voltage']:.2f}V")
        print(f"  Uptime:           {self.telemetry_data['uptime_ms'] / 1000:.1f}s")
        print(f"  Free Heap:        {self.telemetry_data['free_heap']} bytes")
        print(f"  CPU Usage:        {self.telemetry_data['cpu_usage']:.1f}%")
        print("-" * 60)
        print("Commands: (q)uit, (s)tatus, (t)elemetry, (r)eset")
        print("=" * 60)
    
    def start(self):
        """Start monitoring threads"""
        self.running = True
        
        # Start RX thread
        rx_thread = threading.Thread(target=self.rx_thread, daemon=True)
        rx_thread.start()
        
        # Start heartbeat thread
        hb_thread = threading.Thread(target=self.heartbeat_thread, daemon=True)
        hb_thread.start()
        
        self.logger.info("Monitor started")
    
    def stop(self):
        """Stop monitoring threads"""
        self.running = False
        time.sleep(0.5)  # Give threads time to stop
        self.logger.info("Monitor stopped")
    
    def run_interactive(self):
        """Run interactive monitoring session"""
        if not self.connect():
            self.logger.error("Failed to establish connection")
            return
        
        self.start()
        
        try:
            while True:
                self.display_status()
                
                # Non-blocking input check
                try:
                    cmd = input().strip().lower()
                    
                    if cmd == 'q':
                        break
                    elif cmd == 's':
                        self.send_command('GET_STATUS')
                    elif cmd == 't':
                        self.send_command('GET_TELEMETRY')
                    elif cmd == 'r':
                        self.send_command('RESET')
                    else:
                        print(f"Unknown command: {cmd}")
                        
                except KeyboardInterrupt:
                    break
                
                time.sleep(1)
                
        except Exception as e:
            self.logger.error(f"Runtime error: {e}")
            
        finally:
            self.stop()
            self.disconnect()
            print("\nMonitor shutdown complete")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='Base Station Monitor - ESP32 UART Communication')
    parser.add_argument('-p', '--port', default='/dev/ttyUSB0', 
                       help='Serial port (default: /dev/ttyUSB0)')
    parser.add_argument('-b', '--baudrate', type=int, default=115200,
                       help='Baudrate (default: 115200)')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Enable verbose logging')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    monitor = BaseStationMonitor(port=args.port, baudrate=args.baudrate)
    
    try:
        monitor.run_interactive()
    except KeyboardInterrupt:
        print("\nShutting down...")
    except Exception as e:
        print(f"Fatal error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()