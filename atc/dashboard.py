#!/usr/bin/env python3
"""
Dashboard - Main dashboard orchestrator that manages all UI components
Implements a React-like component architecture with Rich terminal UI
"""

import threading
import time
import sys
import select
import termios
import tty
from typing import Dict, List, Optional, Callable
from rich.console import Console
from rich.layout import Layout
from rich.live import Live
from rich.panel import Panel
from rich.text import Text

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
    
    def __init__(self, state_manager: StateManager, keyboard_enabled: bool = True):
        self.state_manager = state_manager
        self.console = Console()
        self.terminal_width, self.terminal_height = self._get_terminal_size()
        self.layout = self._create_layout()
        self.components: Dict[str, BaseComponent] = {}
        self.running = False
        self.focused_component = "command"
        self.keyboard_enabled = keyboard_enabled
        self.keyboard_thread: Optional[threading.Thread] = None
        self.command_callback: Optional[Callable[[str, dict], None]] = None
        self.old_settings = None
        
        # Initialize components
        self._init_components()
        
    def _get_terminal_size(self) -> tuple[int, int]:
        """Get current terminal dimensions"""
        return self.console.size
        
    def _create_layout(self) -> Layout:
        """Create responsive dashboard layout based on terminal size"""
        layout = Layout(name="root")
        
        # Responsive header/footer sizes - ensure they fit
        header_size = 3 if self.terminal_height > 30 else 2
        footer_size = 2  # Give footer more room to ensure it's visible
        
        # Split into header and body
        layout.split_column(
            Layout(name="header", size=header_size),
            Layout(name="body"),
            Layout(name="footer", size=footer_size)
        )
        
        # Responsive column layout based on width
        if self.terminal_width < 120:
            # Narrow layout - prioritize center column
            if self.terminal_width < 80:
                # Ultra narrow - single column
                layout["body"].split_column(
                    Layout(name="logs", ratio=3),
                    Layout(name="status", size=8),
                    Layout(name="command", size=10)
                )
            else:
                # Two column layout
                layout["body"].split_row(
                    Layout(name="left", ratio=2),
                    Layout(name="right", ratio=1)
                )
                
                # Adjust command size based on available height
                command_size = min(10, max(6, (self.terminal_height - header_size - footer_size) // 3))
                
                layout["left"].split_column(
                    Layout(name="logs", ratio=2),
                    Layout(name="command", size=command_size)
                )
                
                # Adjust status size based on available height
                status_size = min(10, max(6, (self.terminal_height - header_size - footer_size) // 3))
                
                layout["right"].split_column(
                    Layout(name="status", size=status_size),
                    Layout(name="telemetry")
                )
        else:
            # Wide layout - full three columns
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
            
            # Split center column with responsive command size
            # Account for header and footer when calculating sizes
            available_height = self.terminal_height - header_size - footer_size - 2  # -2 for borders
            command_size = min(12, max(6, available_height // 3))
            layout["center"].split_column(
                Layout(name="logs", ratio=2),
                Layout(name="command", size=command_size)
            )
            
            # Right column IS telemetry
        
        return layout
    
    def _init_components(self) -> None:
        """Initialize all dashboard components"""
        # Calculate responsive sizes - be more conservative
        # Account for header(3) + footer(2) + borders(2) + other components
        available_height = self.terminal_height - 7
        log_lines = min(15, max(5, available_height // 2))  # Use half of available height
        
        # Create component instances with terminal size info
        self.components = {
            "telemetry": TelemetryPanel(),
            "logs": LogPanel(max_lines=log_lines),
            "status": StatusPanel(),
            "command": CommandPanel(),
            "metrics": MetricsPanel()
        }
        
        # Pass terminal dimensions to components
        for component in self.components.values():
            component.set_props({
                "terminal_width": self.terminal_width,
                "terminal_height": self.terminal_height
            })
        
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
        # Create header text directly
        header_lines = [
            "🛸 BASE STATION MONITOR DASHBOARD",
            "Real-time ESP32 Monitoring & Control"
        ]
        header_text = Text("\n".join(header_lines), justify="center", style="bold cyan")
        
        return Panel(
            header_text,
            style="bright_blue",
            border_style="blue"
        )
    
    def _create_footer(self) -> Panel:
        """Create responsive dashboard footer with help text"""
        # Build help text as a string first, then create Text object
        if self.keyboard_enabled:
            # Responsive help text based on terminal width
            if self.terminal_width < 60:
                # Ultra compact
                help_str = "Q:Quit Tab:Switch ↑↓:Scroll"
            elif self.terminal_width < 100:
                # Compact
                help_str = "Q: Quit | Tab: Focus | ↑↓: Scroll | S/T/R: Commands"
            else:
                # Full
                help_str = "Tab: Focus | ↑↓: Scroll | Q: Quit | C: Clear | S/T/R: Commands"
        else:
            if self.terminal_width < 60:
                help_str = "Ctrl+C to quit"
            else:
                help_str = "Keyboard disabled. Press Ctrl+C to quit."
        
        # Create Text object with the string
        help_text = Text(help_str, style="bold white")
        
        return Panel(
            help_text,
            style="white on grey23",
            border_style="bright_blue",
            padding=(0, 1)
        )
    
    def _update_layout(self) -> None:
        """Update layout with component renders"""
        # Update terminal size and recreate layout if needed
        new_width, new_height = self._get_terminal_size()
        if new_width != self.terminal_width or new_height != self.terminal_height:
            self.terminal_width = new_width
            self.terminal_height = new_height
            # Recreate layout for new size
            self.layout = self._create_layout()
            # Update all components with new size
            for component in self.components.values():
                component.set_props({
                    "terminal_width": self.terminal_width,
                    "terminal_height": self.terminal_height
                })
        
        # Header
        self.layout["header"].update(self._create_header())
        
        # Components - use responsive layout paths
        if self.terminal_width < 80:
            # Ultra narrow - single column
            self.layout["body"]["logs"].update(self.components["logs"].render())
            self.layout["body"]["status"].update(self.components["status"].render())
            self.layout["body"]["command"].update(self.components["command"].render())
        elif self.terminal_width < 120:
            # Two column layout
            self.layout["body"]["left"]["logs"].update(self.components["logs"].render())
            self.layout["body"]["left"]["command"].update(self.components["command"].render())
            self.layout["body"]["right"]["status"].update(self.components["status"].render())
            self.layout["body"]["right"]["telemetry"].update(self.components["telemetry"].render())
        else:
            # Full three column layout
            self.layout["body"]["right"].update(self.components["telemetry"].render())
            self.layout["body"]["center"]["logs"].update(self.components["logs"].render())
            self.layout["body"]["left"]["status"].update(self.components["status"].render())
            self.layout["body"]["center"]["command"].update(self.components["command"].render())
            self.layout["body"]["left"]["metrics"].update(self.components["metrics"].render())
        
        # Footer
        self.layout["footer"].update(self._create_footer())
    
    def _keyboard_handler(self) -> None:
        """Handle keyboard input in a separate thread"""
        try:
            while self.running:
                # Check if input is available
                if sys.stdin in select.select([sys.stdin], [], [], 0.1)[0]:
                    char = sys.stdin.read(1)
                    self._process_key(char)
        except Exception as e:
            from state.actions import log_message
            self.state_manager.dispatch(
                log_message("ERROR", f"Keyboard handler error: {e}", "Dashboard")
            )
    
    def _process_key(self, char: str) -> None:
        """Process a single key press"""
        try:
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
                elif char == '\t':  # Tab key
                    # Cycle focus
                    components = list(self.components.keys())
                    current_idx = components.index(self.focused_component)
                    next_idx = (current_idx + 1) % len(components)
                    self.focused_component = components[next_idx]
                    
                    # Update UI focus in state
                    self.state_manager.dispatch(
                        Action(ActionType.UI_FOCUS_CHANGE, {"focus": self.focused_component})
                    )
                elif char == '\x1b':  # ESC sequence (for arrow keys)
                    # Read the rest of the escape sequence
                    next1 = sys.stdin.read(1)
                    next2 = sys.stdin.read(1)
                    
                    if next1 == '[' and self.focused_component == "logs":
                        log_panel = self.components["logs"]
                        if isinstance(log_panel, LogPanel):
                            if next2 == 'A':  # Up arrow
                                log_panel.scroll_up()
                            elif next2 == 'B':  # Down arrow
                                log_panel.scroll_down()
                            elif next2 == '5':  # Page up
                                sys.stdin.read(1)  # consume ~
                                log_panel.scroll_up(10)
                            elif next2 == '6':  # Page down
                                sys.stdin.read(1)  # consume ~
                                log_panel.scroll_down(10)
                            elif next2 == 'H':  # Home
                                log_panel.scroll_to_top()
                            elif next2 == 'F':  # End
                                log_panel.scroll_to_bottom()
                                
        except Exception as e:
            # Log error but don't crash
            from state.actions import log_message
            self.state_manager.dispatch(
                log_message("ERROR", f"Key processing error: {e}", "Dashboard")
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
        
        if self.keyboard_enabled:
            # Try to set terminal to raw mode for keyboard input
            try:
                self.old_settings = termios.tcgetattr(sys.stdin)
                tty.setcbreak(sys.stdin.fileno())
                
                # Start keyboard handler thread
                self.keyboard_thread = threading.Thread(target=self._keyboard_handler, daemon=True)
                self.keyboard_thread.start()
            except Exception as e:
                # If we can't set raw mode, disable keyboard
                self.keyboard_enabled = False
                from state.actions import log_message
                self.state_manager.dispatch(
                    log_message("WARNING", f"Keyboard input disabled: {e}", "Dashboard")
                )
        
        # Log startup
        from state.actions import log_message
        mode = "with keyboard" if self.keyboard_enabled else "without keyboard"
        self.state_manager.dispatch(
            log_message("INFO", f"Dashboard started {mode}", "Dashboard")
        )
    
    def stop(self) -> None:
        """Stop the dashboard"""
        self.running = False
        
        # Restore terminal settings if we changed them
        if self.keyboard_enabled and self.old_settings:
            try:
                termios.tcsetattr(sys.stdin, termios.TCSADRAIN, self.old_settings)
            except:
                pass
        
        # Unmount components
        for component in self.components.values():
            component.unmount()
    
    def run(self) -> None:
        """Run the dashboard with live updates"""
        self.start()
        
        try:
            # Use vertical_overflow="crop" to prevent content from exceeding terminal height
            with Live(
                self.layout,
                console=self.console,
                screen=True,
                refresh_per_second=10,
                transient=False,
                vertical_overflow="crop"  # Crop content that exceeds terminal height
            ) as live:
                while self.running:
                    self._update_layout()
                    time.sleep(0.1)  # Small delay to prevent CPU hogging
                    
        except KeyboardInterrupt:
            pass
        finally:
            self.stop()