#!/usr/bin/env python3
"""
Telemetry Panel - Displays real-time telemetry data from ESP32
"""

from rich.table import Table
from rich.console import RenderableType
from rich.text import Text
from datetime import datetime

from .base_component import BaseComponent
from ..state.types import TelemetryData


class TelemetryPanel(BaseComponent):
    """Component for displaying telemetry data"""
    
    def __init__(self):
        super().__init__(title="📡 Telemetry Data", refresh_rate=0.5)
        self.telemetry_data: TelemetryData = self._get_default_telemetry()
    
    def _get_default_telemetry(self) -> TelemetryData:
        """Get default telemetry values"""
        return TelemetryData(
            timestamp=None,
            system_status="UNKNOWN",
            esp_now_connected=False,
            remote_devices=0,
            signal_strength=0,
            battery_voltage=0.0,
            uptime_ms=0,
            free_heap=0,
            cpu_usage=0.0
        )
    
    def on_mount(self):
        """Subscribe to telemetry updates when mounted"""
        if self._state_manager:
            self._state_manager.subscribe("telemetry", self._on_telemetry_update)
    
    def _on_telemetry_update(self, telemetry: TelemetryData) -> None:
        """Handle telemetry update from state manager"""
        self.telemetry_data = telemetry
        self.set_state({"last_update": datetime.now().isoformat()})
    
    def render(self) -> RenderableType:
        """Render telemetry data as a table"""
        table = Table(show_header=False, expand=True, box=None, padding=(0, 1))
        table.add_column("Property", style="cyan", width=20)
        table.add_column("Value", style="white")
        
        # Format timestamp
        timestamp_str = self.telemetry_data.get("timestamp", "N/A")
        if timestamp_str and timestamp_str != "N/A":
            try:
                dt = datetime.fromisoformat(timestamp_str)
                timestamp_str = dt.strftime("%H:%M:%S")
            except:
                pass
        
        # System status with color coding
        status = self.telemetry_data.get("system_status", "UNKNOWN")
        status_color = self._get_status_color(status)
        status_text = Text(status, style=status_color)
        
        # ESP-NOW connection status
        esp_connected = self.telemetry_data.get("esp_now_connected", False)
        esp_text = Text(
            "✓ Connected" if esp_connected else "✗ Disconnected",
            style="green" if esp_connected else "red"
        )
        
        # Signal strength with visual indicator
        signal = self.telemetry_data.get("signal_strength", 0)
        signal_bars = self._get_signal_bars(signal)
        signal_text = Text(f"{signal} dBm {signal_bars}", style=self._get_signal_color(signal))
        
        # Battery voltage with color coding
        battery = self.telemetry_data.get("battery_voltage", 0.0)
        battery_text = Text(f"{battery:.2f}V", style=self._get_battery_color(battery))
        
        # Format uptime
        uptime_ms = self.telemetry_data.get("uptime_ms", 0)
        uptime_str = self._format_uptime(uptime_ms)
        
        # CPU usage with color coding
        cpu = self.telemetry_data.get("cpu_usage", 0.0)
        cpu_text = Text(f"{cpu:.1f}%", style=self._get_cpu_color(cpu))
        
        # Free heap with color coding
        heap = self.telemetry_data.get("free_heap", 0)
        heap_text = Text(
            f"{heap:,} bytes",
            style=self._get_heap_color(heap)
        )
        
        # Add rows to table
        table.add_row("Last Update", timestamp_str)
        table.add_row("System Status", status_text)
        table.add_row("ESP-NOW", esp_text)
        table.add_row("Remote Devices", str(self.telemetry_data.get("remote_devices", 0)))
        table.add_row("Signal Strength", signal_text)
        table.add_row("Battery", battery_text)
        table.add_row("Uptime", uptime_str)
        table.add_row("CPU Usage", cpu_text)
        table.add_row("Free Heap", heap_text)
        
        return self.create_panel(table)
    
    def _get_status_color(self, status: str) -> str:
        """Get color for system status"""
        colors = {
            "READY": "green",
            "RUNNING": "green",
            "WARNING": "yellow",
            "ERROR": "red",
            "UNKNOWN": "dim white"
        }
        return colors.get(status.upper(), "white")
    
    def _get_signal_bars(self, signal: int) -> str:
        """Get visual signal strength indicator"""
        if signal >= -50:
            return "▁▃▅▇"  # Excellent
        elif signal >= -60:
            return "▁▃▅_"  # Good
        elif signal >= -70:
            return "▁▃__"  # Fair
        elif signal >= -80:
            return "▁___"  # Weak
        else:
            return "____"  # No signal
    
    def _get_signal_color(self, signal: int) -> str:
        """Get color for signal strength"""
        if signal >= -50:
            return "green"
        elif signal >= -70:
            return "yellow"
        else:
            return "red"
    
    def _get_battery_color(self, voltage: float) -> str:
        """Get color for battery voltage"""
        if voltage >= 3.7:
            return "green"
        elif voltage >= 3.4:
            return "yellow"
        elif voltage > 0:
            return "red"
        else:
            return "dim white"
    
    def _get_cpu_color(self, usage: float) -> str:
        """Get color for CPU usage"""
        if usage < 50:
            return "green"
        elif usage < 80:
            return "yellow"
        else:
            return "red"
    
    def _get_heap_color(self, heap: int) -> str:
        """Get color for free heap"""
        if heap > 50000:
            return "green"
        elif heap > 20000:
            return "yellow"
        elif heap > 0:
            return "red"
        else:
            return "dim white"
    
    def _format_uptime(self, uptime_ms: int) -> str:
        """Format uptime from milliseconds to readable string"""
        if uptime_ms == 0:
            return "N/A"
        
        seconds = uptime_ms // 1000
        minutes = seconds // 60
        hours = minutes // 60
        days = hours // 24
        
        if days > 0:
            return f"{days}d {hours % 24}h {minutes % 60}m"
        elif hours > 0:
            return f"{hours}h {minutes % 60}m {seconds % 60}s"
        elif minutes > 0:
            return f"{minutes}m {seconds % 60}s"
        else:
            return f"{seconds}s"