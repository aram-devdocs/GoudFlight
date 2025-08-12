#!/usr/bin/env python3
"""
Dashboard - Main dashboard orchestrator that manages all UI components
Implements a React-like component architecture with Rich terminal UI
"""

import threading
import time
from typing import Dict, List, Optional, Callable
from rich.console import Console
from rich.layout import Layout
from rich.live import Live
from rich.panel import Panel
from rich.text import Text
from pynput import keyboard

from components.base_component import BaseComponent
from components.telemetry_panel import TelemetryPanel
from components.log_panel import LogPanel
from components.status_panel import StatusPanel
from components.command_panel import CommandPanel
from components.metrics_panel import MetricsPanel
from state.state_manager import StateManager
from state.actions import Action, ActionType


class Dashboard:
    """Main dashboard orchestrator"""
    
    def __init__(self, state_manager: StateManager):
        self.state_manager = state_manager
        self.console = Console()
        self.layout = self._create_layout()
        self.components: Dict[str, BaseComponent] = {}
        self.running = False
        self.focused_component = "command"
        self.keyboard_listener: Optional[keyboard.Listener] = None
        self.command_callback: Optional[Callable[[str, dict], None]] = None
        
        # Initialize components
        self._init_components()
        
    def _create_layout(self) -> Layout:
        """Create the dashboard layout"""
        layout = Layout(name="root")
        
        # Split into header and body
        layout.split_column(
            Layout(name="header", size=3),
            Layout(name="body"),
            Layout(name="footer", size=1)
        )
        
        # Split body into left and right columns
        layout["body"].split_row(
            Layout(name="left", ratio=1),
            Layout(name="center", ratio=2),
            Layout(name="right", ratio=1)
        )
        
        # Split left column
        layout["left"].split_column(
            Layout(name="status", size=12),
            Layout(name="metrics")
        )
        
        # Split center column
        layout["center"].split_column(
            Layout(name="logs", ratio=2),
            Layout(name="command", size=15)
        )
        
        # Right column for telemetry
        layout["right"].update(Layout(name="telemetry"))
        
        return layout
    
    def _init_components(self) -> None:
        """Initialize all dashboard components"""
        # Create component instances
        self.components = {
            "telemetry": TelemetryPanel(),
            "logs": LogPanel(max_lines=20),
            "status": StatusPanel(),
            "command": CommandPanel(),
            "metrics": MetricsPanel()
        }
        
        # Mount all components
        for name, component in self.components.items():
            component.mount(self._trigger_update, self.state_manager)
    
    def _trigger_update(self) -> None:
        """Callback to trigger dashboard update"""
        # This will be called by components when they need to update
        # The Live display will handle the actual refresh
        pass
    
    def _create_header(self) -> Panel:
        """Create dashboard header"""
        title = Text("🛸 BASE STATION MONITOR DASHBOARD", style="bold cyan")
        subtitle = Text("Real-time ESP32 Monitoring & Control", style="dim white")
        
        header_content = Text.from_markup(
            f"{title}\n{subtitle}",
            justify="center"
        )
        
        return Panel(
            header_content,
            style="bright_blue",
            box_style="blue"
        )
    
    def _create_footer(self) -> Panel:
        """Create dashboard footer with help text"""
        help_text = Text()
        help_text.append("F1", style="bold yellow")
        help_text.append(": Help  ", style="white")
        help_text.append("Tab", style="bold yellow")
        help_text.append(": Focus  ", style="white")
        help_text.append("↑↓", style="bold yellow")
        help_text.append(": Scroll  ", style="white")
        help_text.append("Q", style="bold yellow")
        help_text.append(": Quit  ", style="white")
        help_text.append("C", style="bold yellow")
        help_text.append(": Clear Logs  ", style="white")
        help_text.append("R", style="bold yellow")
        help_text.append(": Refresh", style="white")
        
        return Panel(
            help_text,
            style="dim white",
            box_style="dim"
        )
    
    def _update_layout(self) -> None:
        """Update layout with component renders"""
        # Header
        self.layout["header"].update(self._create_header())
        
        # Components
        self.layout["telemetry"].update(self.components["telemetry"].render())
        self.layout["logs"].update(self.components["logs"].render())
        self.layout["status"].update(self.components["status"].render())
        self.layout["command"].update(self.components["command"].render())
        self.layout["metrics"].update(self.components["metrics"].render())
        
        # Footer
        self.layout["footer"].update(self._create_footer())
    
    def _on_key_press(self, key) -> None:
        """Handle keyboard input"""
        try:
            if hasattr(key, 'char'):
                char = key.char
                if char:
                    char_lower = char.lower()
                    
                    # Global commands
                    if char_lower == 'q':
                        self.stop()
                    elif char_lower == 'c':
                        # Clear logs
                        self.state_manager.dispatch(Action(ActionType.LOG_CLEAR))
                    elif char_lower == 's':
                        # Send status command
                        if self.command_callback:
                            self.command_callback('GET_STATUS', {})
                    elif char_lower == 't':
                        # Send telemetry command
                        if self.command_callback:
                            self.command_callback('GET_TELEMETRY', {})
                    elif char_lower == 'r':
                        # Send reset command
                        if self.command_callback:
                            self.command_callback('RESET', {})
            
            # Special keys
            elif key == keyboard.Key.tab:
                # Cycle focus
                components = list(self.components.keys())
                current_idx = components.index(self.focused_component)
                next_idx = (current_idx + 1) % len(components)
                self.focused_component = components[next_idx]
                
                # Update UI focus in state
                self.state_manager.dispatch(
                    Action(ActionType.UI_FOCUS_CHANGE, {"focus": self.focused_component})
                )
            
            elif key == keyboard.Key.up:
                # Scroll up in focused component
                if self.focused_component == "logs":
                    log_panel = self.components["logs"]
                    if isinstance(log_panel, LogPanel):
                        log_panel.scroll_up()
            
            elif key == keyboard.Key.down:
                # Scroll down in focused component
                if self.focused_component == "logs":
                    log_panel = self.components["logs"]
                    if isinstance(log_panel, LogPanel):
                        log_panel.scroll_down()
            
            elif key == keyboard.Key.page_up:
                # Page up in logs
                if self.focused_component == "logs":
                    log_panel = self.components["logs"]
                    if isinstance(log_panel, LogPanel):
                        log_panel.scroll_up(10)
            
            elif key == keyboard.Key.page_down:
                # Page down in logs
                if self.focused_component == "logs":
                    log_panel = self.components["logs"]
                    if isinstance(log_panel, LogPanel):
                        log_panel.scroll_down(10)
            
            elif key == keyboard.Key.home:
                # Go to top of logs
                if self.focused_component == "logs":
                    log_panel = self.components["logs"]
                    if isinstance(log_panel, LogPanel):
                        log_panel.scroll_to_top()
            
            elif key == keyboard.Key.end:
                # Go to bottom of logs
                if self.focused_component == "logs":
                    log_panel = self.components["logs"]
                    if isinstance(log_panel, LogPanel):
                        log_panel.scroll_to_bottom()
                        
        except Exception as e:
            # Log error but don't crash
            from state.actions import log_message
            self.state_manager.dispatch(
                log_message("ERROR", f"Keyboard handler error: {e}", "Dashboard")
            )
    
    def set_command_callback(self, callback: Callable[[str, dict], None]) -> None:
        """Set callback for sending commands"""
        self.command_callback = callback
        
        # Also set on command panel
        cmd_panel = self.components.get("command")
        if isinstance(cmd_panel, CommandPanel):
            cmd_panel.set_send_callback(callback)
    
    def start(self) -> None:
        """Start the dashboard"""
        self.running = True
        
        # Start keyboard listener
        self.keyboard_listener = keyboard.Listener(on_press=self._on_key_press)
        self.keyboard_listener.start()
        
        # Log startup
        from state.actions import log_message
        self.state_manager.dispatch(
            log_message("INFO", "Dashboard started", "Dashboard")
        )
    
    def stop(self) -> None:
        """Stop the dashboard"""
        self.running = False
        
        # Stop keyboard listener
        if self.keyboard_listener:
            self.keyboard_listener.stop()
        
        # Unmount components
        for component in self.components.values():
            component.unmount()
    
    def run(self) -> None:
        """Run the dashboard with live updates"""
        self.start()
        
        try:
            with Live(
                self.layout,
                console=self.console,
                screen=True,
                refresh_per_second=10,
                transient=False
            ) as live:
                while self.running:
                    self._update_layout()
                    time.sleep(0.1)  # Small delay to prevent CPU hogging
                    
        except KeyboardInterrupt:
            pass
        finally:
            self.stop()