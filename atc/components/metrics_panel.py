#!/usr/bin/env python3
"""
Metrics Panel - Displays performance metrics and statistics
"""

from rich.console import RenderableType, Group
from rich.text import Text
from rich.progress import Progress, BarColumn, TextColumn, SpinnerColumn
from rich.table import Table
from typing import List, Deque
from collections import deque

from components.base_component import BaseComponent
from state.types import MetricsData


class MetricsPanel(BaseComponent):
    """Component for displaying performance metrics"""
    
    def __init__(self):
        super().__init__(title="📊 Performance Metrics", refresh_rate=1.0)
        self.metrics: MetricsData = self._get_default_metrics()
        self.message_rate_history: Deque[float] = deque(maxlen=20)
        self.latency_history: Deque[float] = deque(maxlen=20)
    
    def _get_default_metrics(self) -> MetricsData:
        """Get default metrics"""
        return MetricsData(
            rx_bytes=0,
            tx_bytes=0,
            rx_messages=0,
            tx_messages=0,
            error_count=0,
            uptime_seconds=0.0,
            message_rate=0.0,
            latency_ms=0.0
        )
    
    def on_mount(self):
        """Subscribe to metrics updates"""
        if self._state_manager:
            self._state_manager.subscribe("metrics", self._on_metrics_update)
    
    def _on_metrics_update(self, metrics: MetricsData) -> None:
        """Handle metrics update"""
        self.metrics = metrics
        
        # Update history
        self.message_rate_history.append(metrics.get("message_rate", 0.0))
        self.latency_history.append(metrics.get("latency_ms", 0.0))
        
        self.set_state({"last_update": True})
    
    def render(self) -> RenderableType:
        """Render metrics display"""
        elements = []
        
        # Data transfer statistics
        elements.append(Text("Data Transfer:", style="bold cyan"))
        transfer_table = Table(show_header=False, expand=True, box=None, padding=(0, 2))
        transfer_table.add_column("Metric", style="white", width=20)
        transfer_table.add_column("Value", style="green")
        
        rx_kb = self.metrics["rx_bytes"] / 1024
        tx_kb = self.metrics["tx_bytes"] / 1024
        
        transfer_table.add_row("Received", f"{rx_kb:.2f} KB ({self.metrics['rx_messages']} msgs)")
        transfer_table.add_row("Transmitted", f"{tx_kb:.2f} KB ({self.metrics['tx_messages']} msgs)")
        transfer_table.add_row("Errors", self._format_errors(self.metrics["error_count"]))
        
        elements.append(transfer_table)
        elements.append(Text(""))  # Spacing
        
        # Performance metrics
        elements.append(Text("Performance:", style="bold cyan"))
        
        # Message rate with sparkline
        rate_display = self._create_rate_display()
        elements.append(rate_display)
        
        # Latency with sparkline
        latency_display = self._create_latency_display()
        elements.append(latency_display)
        
        elements.append(Text(""))  # Spacing
        
        # System uptime
        uptime_str = self._format_uptime(self.metrics["uptime_seconds"])
        elements.append(Text(f"System Uptime: {uptime_str}", style="dim white"))
        
        # Throughput calculation
        if self.metrics["uptime_seconds"] > 0:
            throughput = (self.metrics["rx_bytes"] + self.metrics["tx_bytes"]) / self.metrics["uptime_seconds"]
            elements.append(Text(f"Avg Throughput: {throughput:.1f} B/s", style="dim white"))
        
        return self.create_panel(Group(*elements))
    
    def _format_errors(self, count: int) -> Text:
        """Format error count with color"""
        if count == 0:
            return Text(str(count), style="green")
        elif count < 10:
            return Text(str(count), style="yellow")
        else:
            return Text(str(count), style="red bold")
    
    def _create_rate_display(self) -> Group:
        """Create message rate display with sparkline"""
        current_rate = self.metrics.get("message_rate", 0.0)
        
        # Create sparkline from history
        sparkline = self._create_sparkline(self.message_rate_history)
        
        # Create progress bar for current rate (0-100 msg/s scale)
        progress = Progress(
            TextColumn("[progress.description]{task.description}"),
            BarColumn(bar_width=15),
            TextColumn("{task.completed:.1f} msg/s"),
            expand=False
        )
        
        task = progress.add_task("Message Rate", total=100, completed=min(current_rate, 100))
        
        return Group(
            progress,
            Text(f"  History: {sparkline}", style="dim cyan")
        )
    
    def _create_latency_display(self) -> Group:
        """Create latency display with sparkline"""
        current_latency = self.metrics.get("latency_ms", 0.0)
        
        # Create sparkline from history
        sparkline = self._create_sparkline(self.latency_history)
        
        # Color based on latency
        if current_latency < 10:
            style = "green"
        elif current_latency < 50:
            style = "yellow"
        else:
            style = "red"
        
        # Create progress bar for latency (0-100ms scale)
        progress = Progress(
            TextColumn("[progress.description]{task.description}"),
            BarColumn(bar_width=15),
            TextColumn("{task.completed:.1f} ms"),
            expand=False
        )
        
        task = progress.add_task("Latency", total=100, completed=min(current_latency, 100))
        
        return Group(
            progress,
            Text(f"  History: {sparkline}", style="dim cyan")
        )
    
    def _create_sparkline(self, data: Deque[float]) -> str:
        """Create a sparkline visualization from data"""
        if not data:
            return "─" * 20
        
        blocks = "▁▂▃▄▅▆▇█"
        
        # Normalize data to 0-7 range
        min_val = min(data) if data else 0
        max_val = max(data) if data else 1
        range_val = max_val - min_val if max_val != min_val else 1
        
        sparkline = ""
        for value in data:
            normalized = (value - min_val) / range_val
            index = min(int(normalized * 7), 7)
            sparkline += blocks[index]
        
        # Pad with empty if needed
        while len(sparkline) < 20:
            sparkline = "─" + sparkline
        
        return sparkline
    
    def _format_uptime(self, seconds: float) -> str:
        """Format uptime from seconds"""
        if seconds == 0:
            return "N/A"
        
        hours = int(seconds // 3600)
        minutes = int((seconds % 3600) // 60)
        secs = int(seconds % 60)
        
        if hours > 0:
            return f"{hours}h {minutes}m {secs}s"
        elif minutes > 0:
            return f"{minutes}m {secs}s"
        else:
            return f"{secs}s"