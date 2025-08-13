#!/usr/bin/env python3
"""
Command Panel - Interactive command input and history display
"""

from rich.console import RenderableType, Group
from rich.table import Table
from rich.text import Text
from typing import List, Callable, Optional

from components.base_component import BaseComponent
from state.types import CommandHistory


class CommandPanel(BaseComponent):
    """Component for command input and history"""
    
    def __init__(self, max_history: int = 5):
        super().__init__(title="⌨ Command Interface", refresh_rate=1.0)
        self.max_history = max_history
        self.command_history: List[CommandHistory] = []
        self.available_commands = {
            "PING": "Send heartbeat ping",
            "GET_TELEMETRY": "Request telemetry data",
            "GET_STATUS": "Get system status",
            "RESET": "Reset the ESP32",
            "CLEAR_LOGS": "Clear local logs",
            "SET_MODE": "Change operating mode",
            "CALIBRATE": "Start calibration"
        }
        self.send_callback: Optional[Callable[[str, dict], None]] = None
    
    def on_mount(self):
        """Subscribe to command history updates"""
        if self._state_manager:
            self._state_manager.subscribe("command_history", self._on_history_update)
    
    def _on_history_update(self, history: List[CommandHistory]) -> None:
        """Handle command history update"""
        self.command_history = history[-self.max_history:] if history else []
        self.set_state({"history_count": len(history)})
    
    def set_send_callback(self, callback: Callable[[str, dict], None]) -> None:
        """Set callback for sending commands"""
        self.send_callback = callback
    
    def render(self) -> RenderableType:
        """Render command panel"""
        elements = []
        
        # Get terminal dimensions to decide what to show
        terminal_height = self.props.get("terminal_height", 24)
        terminal_width = self.props.get("terminal_width", 80)
        
        # Show compact view for small terminals
        if terminal_height < 30 or terminal_width < 80:
            # Just show essential commands in a single line
            cmd_text = Text()
            cmd_text.append("Commands: ", style="bold")
            cmd_text.append("PING ", style="cyan")
            cmd_text.append("GET_TELEMETRY ", style="cyan")
            cmd_text.append("GET_STATUS ", style="cyan")
            cmd_text.append("RESET", style="cyan")
            elements.append(cmd_text)
        else:
            # Show full command table
            cmd_table = Table(show_header=False, expand=True, box=None, padding=(0, 1))
            cmd_table.add_column("Command", style="cyan", width=12)
            cmd_table.add_column("Description", style="dim white")
            
            # Show only most important commands
            essential_cmds = {
                "PING": "Heartbeat",
                "GET_TELEMETRY": "Get data",
                "GET_STATUS": "Status",
                "RESET": "Reset ESP32"
            }
            
            for cmd, desc in essential_cmds.items():
                cmd_table.add_row(cmd, desc)
            
            elements.append(cmd_table)
        
        # Command history section
        if self.command_history:
            elements.append(Text(""))  # Spacing
            elements.append(Text("Recent Commands:", style="bold"))
            
            history_table = Table(show_header=False, expand=True, box=None, padding=(0, 1))
            history_table.add_column("Time", style="dim cyan", width=8)
            history_table.add_column("Command", style="white")
            history_table.add_column("Status", width=8)
            
            for entry in reversed(self.command_history):
                time_str = entry["timestamp"].strftime("%H:%M:%S")
                cmd = entry["command"]
                
                # Status indicator
                if entry.get("response") is None:
                    status = Text("⟳", style="yellow")
                elif entry.get("success", False):
                    status = Text("✓", style="green")
                else:
                    status = Text("✗", style="red")
                
                history_table.add_row(time_str, cmd, status)
            
            elements.append(history_table)
        
        # Only show shortcuts on larger displays
        if terminal_height >= 30:
            elements.append(Text(""))  # Spacing
            shortcuts = Text("Keys: ", style="dim white")
            shortcuts.append("Q ", style="yellow")
            shortcuts.append("S ", style="yellow")
            shortcuts.append("T ", style="yellow")
            shortcuts.append("R ", style="yellow")
            shortcuts.append("C", style="yellow")
            elements.append(shortcuts)
        
        return self.create_panel(Group(*elements))
    
    def send_command(self, command: str, params: Optional[dict] = None) -> None:
        """Send a command through the callback"""
        if self.send_callback:
            self.send_callback(command, params or {})
            
            # Dispatch to state manager
            if self._state_manager:
                from state.actions import command_sent
                self._state_manager.dispatch(command_sent(command, params))