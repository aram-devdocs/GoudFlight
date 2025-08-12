#!/usr/bin/env python3
"""
Base Station Monitor - Advanced dashboard for monitoring ESP32 base station
Features React-like component architecture with real-time updates
"""

import serial
import json
import time
import threading
import sys
import argparse
from datetime import datetime
from typing import Dict, Optional
from queue import Queue
import logging

# Import dashboard and state management
from dashboard import Dashboard
from state.state_manager import StateManager
from state.actions import (
    connection_established, connection_lost, connection_error,
    telemetry_update, log_message, status_update,
    heartbeat_received, command_sent, metrics_update,
    Action, ActionType
)
from state.types import TelemetryData, MetricsData

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
        
        # State management
        self.state_manager = StateManager()
        self.dashboard = Dashboard(self.state_manager)
        self.dashboard.set_command_callback(self.send_command)
        
        # Metrics tracking
        self.rx_bytes = 0
        self.tx_bytes = 0
        self.rx_messages = 0
        self.tx_messages = 0
        self.error_count = 0
        self.start_time = time.time()
    
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
            
            # Update state
            self.state_manager.dispatch(connection_established(self.port, self.baudrate))
            self.state_manager.dispatch(
                log_message("INFO", f"Connected to {self.port} at {self.baudrate} baud", "Serial")
            )
            return True
            
        except serial.SerialException as e:
            self.logger.error(f"Failed to connect: {e}")
            self.connection_status = "ERROR"
            
            # Update state
            self.state_manager.dispatch(
                connection_error({"message": str(e)})
            )
            self.state_manager.dispatch(
                log_message("ERROR", f"Failed to connect: {e}", "Serial")
            )
            return False
    
    def disconnect(self):
        """Close serial connection"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            self.connection_status = "DISCONNECTED"
            self.logger.info("Disconnected from serial port")
            
            # Update state
            self.state_manager.dispatch(connection_lost("User disconnected"))
            self.state_manager.dispatch(
                log_message("INFO", "Disconnected from serial port", "Serial")
            )
    
    def send_command(self, command: str, params: dict) -> bool:
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
            
            # Update metrics and state
            self.tx_bytes += len(json_str)
            self.tx_messages += 1
            self.state_manager.dispatch(command_sent(command, params))
            
            return True
            
        except serial.SerialException as e:
            self.logger.error(f"Failed to send command: {e}")
            self.error_count += 1
            
            self.state_manager.dispatch(
                log_message("ERROR", f"Failed to send command: {e}", "Serial")
            )
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
                            # Update metrics
                            self.rx_bytes += len(line)
                            self.rx_messages += 1
                            self.process_received_data(decoded)
                            
            except serial.SerialException as e:
                self.logger.error(f"RX error: {e}")
                self.connection_status = "ERROR"
                self.error_count += 1
                
                self.state_manager.dispatch(
                    connection_error({"message": str(e)})
                )
                self.state_manager.dispatch(
                    log_message("ERROR", f"RX error: {e}", "Serial")
                )
                
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
                status = msg.get('status', 'UNKNOWN')
                self.state_manager.dispatch(status_update(status))
                self.state_manager.dispatch(
                    log_message("INFO", f"System status: {status}", "ESP32")
                )
                
            elif msg_type == 'HEARTBEAT':
                self.last_heartbeat = time.time()
                self.state_manager.dispatch(Action(ActionType.HEARTBEAT_RECEIVED))
                self.logger.debug("Heartbeat received")
                
            elif msg_type == 'EVENT':
                event_msg = f"Event: {msg.get('event', 'UNKNOWN')} - {msg.get('data', {})}"
                self.state_manager.dispatch(
                    log_message("INFO", event_msg, "ESP32")
                )
                
            elif msg_type == 'ERROR':
                error_msg = msg.get('message', 'Unknown error')
                self.state_manager.dispatch(
                    log_message("ERROR", f"ESP32 Error: {error_msg}", "ESP32")
                )
                self.error_count += 1
                
            else:
                self.state_manager.dispatch(
                    log_message("DEBUG", f"Received: {msg}", "ESP32")
                )
                
        except json.JSONDecodeError as e:
            # Not JSON, might be plain text debug output
            if data.startswith('[') or data.startswith('DEBUG:') or data.startswith('INFO:'):
                self.state_manager.dispatch(
                    log_message("DEBUG", data, "ESP32")
                )
            else:
                self.state_manager.dispatch(
                    log_message("WARNING", f"Invalid JSON received: {data}", "Serial")
                )
                self.error_count += 1
    
    def update_telemetry(self, data: dict):
        """Update telemetry data"""
        # Create typed telemetry data
        telemetry = TelemetryData(
            timestamp=datetime.now().isoformat(),
            system_status=data.get('system_status', 'UNKNOWN'),
            esp_now_connected=data.get('esp_now_connected', False),
            remote_devices=data.get('remote_devices', 0),
            signal_strength=data.get('signal_strength', 0),
            battery_voltage=data.get('battery_voltage', 0.0),
            uptime_ms=data.get('uptime_ms', 0),
            free_heap=data.get('free_heap', 0),
            cpu_usage=data.get('cpu_usage', 0.0)
        )
        
        self.state_manager.dispatch(telemetry_update(telemetry))
        self.logger.debug(f"Telemetry updated: {telemetry}")
    
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
                self.state_manager.dispatch(Action(ActionType.HEARTBEAT_TIMEOUT))
                self.state_manager.dispatch(
                    log_message("WARNING", "No heartbeat received for 10 seconds", "Monitor")
                )
            
            # Update metrics
            self._update_metrics()
            
            time.sleep(3)
        
        self.logger.info("Heartbeat thread stopped")
    
    def _update_metrics(self):
        """Update performance metrics"""
        uptime = time.time() - self.start_time
        
        # Calculate message rate (messages per second)
        message_rate = (self.rx_messages + self.tx_messages) / max(1, uptime)
        
        # Estimate latency (simplified - would need proper measurement)
        latency = 5.0 if self.connection_status == "CONNECTED" else 0.0
        
        metrics = MetricsData(
            rx_bytes=self.rx_bytes,
            tx_bytes=self.tx_bytes,
            rx_messages=self.rx_messages,
            tx_messages=self.tx_messages,
            error_count=self.error_count,
            uptime_seconds=uptime,
            message_rate=message_rate,
            latency_ms=latency
        )
        
        self.state_manager.dispatch(metrics_update(metrics))
    
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
    
    def run_dashboard(self):
        """Run the modern dashboard interface"""
        # Try to connect
        if not self.connect():
            self.logger.error("Failed to establish connection")
            self.state_manager.dispatch(
                log_message("ERROR", "Failed to establish initial connection. Dashboard will start anyway.", "Monitor")
            )
        
        # Start monitoring threads
        self.start()
        
        try:
            # Run the dashboard (blocking)
            self.dashboard.run()
            
        except Exception as e:
            self.logger.error(f"Dashboard error: {e}")
            self.state_manager.dispatch(
                log_message("ERROR", f"Dashboard error: {e}", "Monitor")
            )
            
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
        print("Starting Base Station Monitor Dashboard...")
        print("Press 'Q' to quit, 'F1' for help")
        time.sleep(1)  # Give user time to read
        monitor.run_dashboard()
    except KeyboardInterrupt:
        print("\nShutting down...")
    except Exception as e:
        print(f"Fatal error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()