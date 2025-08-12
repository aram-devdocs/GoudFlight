#!/usr/bin/env python3
"""
Status Panel - Displays connection status and system health
"""

from rich.console import RenderableType, Group
from rich.text import Text
from rich.align import Align
from rich.progress import Progress, BarColumn, TextColumn
from datetime import datetime
from typing import Optional

from components.base_component import BaseComponent
from state.types import ConnectionInfo


class StatusPanel(BaseComponent):
    """Component for displaying connection and system status"""
    
    def __init__(self):
        super().__init__(title="🔌 Connection Status", refresh_rate=0.5)
        self.connection_info: ConnectionInfo = self._get_default_connection()
        self.heartbeat_timeout = 10.0  # seconds
    
    def _get_default_connection(self) -> ConnectionInfo:
        """Get default connection info"""
        return ConnectionInfo(
            status="DISCONNECTED",
            port="/dev/ttyUSB0",
            baudrate=115200,
            last_heartbeat=None,
            error_message=None
        )
    
    def on_mount(self):
        """Subscribe to connection updates when mounted"""
        if self._state_manager:
            self._state_manager.subscribe("connection", self._on_connection_update)
    
    def _on_connection_update(self, connection: ConnectionInfo) -> None:
        """Handle connection update from state manager"""
        self.connection_info = connection
        self.set_state({"last_update": datetime.now().isoformat()})
    
    def render(self) -> RenderableType:
        """Render connection status display"""
        elements = []
        
        # Connection status with icon
        status = self.connection_info["status"]
        status_display = self._format_status(status)
        elements.append(Align.center(status_display))
        
        # Connection details
        elements.append(Text(""))  # Spacing
        elements.append(Text(f"Port: {self.connection_info['port']}", style="cyan"))
        elements.append(Text(f"Baudrate: {self.connection_info['baudrate']:,} bps", style="cyan"))
        
        # Heartbeat status
        elements.append(Text(""))  # Spacing
        heartbeat_display = self._format_heartbeat()
        elements.append(heartbeat_display)
        
        # Error message if any
        if self.connection_info.get("error_message"):
            elements.append(Text(""))  # Spacing
            error_text = Text(f"⚠ {self.connection_info['error_message']}", style="yellow")
            elements.append(error_text)
        
        # Connection quality indicator
        if status == "CONNECTED":
            elements.append(Text(""))  # Spacing
            quality_bar = self._create_quality_indicator()
            elements.append(quality_bar)
        
        return self.create_panel(Group(*elements))
    
    def _format_status(self, status: str) -> Text:
        """Format connection status with icon and color"""
        status_configs = {
            "CONNECTED": ("✓ CONNECTED", "green bold"),
            "DISCONNECTED": ("✗ DISCONNECTED", "red"),
            "ERROR": ("⚠ ERROR", "red bold"),
            "TIMEOUT": ("⏱ TIMEOUT", "yellow"),
            "CONNECTING": ("⟳ CONNECTING...", "yellow blink")
        }
        
        text, style = status_configs.get(status, ("? UNKNOWN", "dim white"))
        return Text(text, style=style)
    
    def _format_heartbeat(self) -> Text:
        """Format heartbeat status"""
        last_heartbeat = self.connection_info.get("last_heartbeat")
        
        if not last_heartbeat:
            return Text("Heartbeat: No signal", style="dim white")
        
        # Calculate time since last heartbeat
        now = datetime.now().timestamp()
        elapsed = now - last_heartbeat
        
        if elapsed < 2:
            symbol = "💚"
            status = "Strong"
            style = "green"
        elif elapsed < 5:
            symbol = "💛"
            status = "Normal"
            style = "yellow"
        elif elapsed < self.heartbeat_timeout:
            symbol = "🧡"
            status = "Weak"
            style = "orange1"
        else:
            symbol = "💔"
            status = "Lost"
            style = "red"
        
        return Text(f"{symbol} Heartbeat: {status} ({elapsed:.1f}s ago)", style=style)
    
    def _create_quality_indicator(self) -> Progress:
        """Create connection quality progress bar"""
        # Calculate quality based on heartbeat
        last_heartbeat = self.connection_info.get("last_heartbeat")
        quality = 100
        
        if last_heartbeat:
            now = datetime.now().timestamp()
            elapsed = now - last_heartbeat
            # Quality decreases as heartbeat gets older
            quality = max(0, int(100 - (elapsed / self.heartbeat_timeout * 100)))
        
        progress = Progress(
            TextColumn("[progress.description]{task.description}"),
            BarColumn(bar_width=20),
            TextColumn("[progress.percentage]{task.percentage:>3.0f}%"),
            expand=False
        )
        
        task = progress.add_task("Signal Quality", total=100)
        progress.update(task, completed=quality)
        
        return progress