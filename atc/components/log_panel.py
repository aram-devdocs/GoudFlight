#!/usr/bin/env python3
"""
Log Panel - Scrollable log viewer with filtering and color coding
"""

from rich.table import Table
from rich.console import RenderableType
from rich.text import Text
from typing import List, Optional

from components.base_component import BaseComponent
from state.types import LogEntry


class LogPanel(BaseComponent):
    """Component for displaying scrollable logs"""
    
    def __init__(self, max_lines: int = 15):
        super().__init__(title="📋 System Logs", refresh_rate=0.2)
        self.max_lines = max_lines
        self.logs: List[LogEntry] = []
        self.filter_level: Optional[str] = None
        self.auto_scroll = True
        self.scroll_offset = 0
    
    def on_mount(self):
        """Subscribe to log updates when mounted"""
        if self._state_manager:
            self._state_manager.subscribe("logs", self._on_logs_update)
    
    def _on_logs_update(self, logs: List[LogEntry]) -> None:
        """Handle logs update from state manager"""
        self.logs = logs
        if self.auto_scroll:
            # Auto-scroll to bottom when new logs arrive
            self.scroll_offset = max(0, len(logs) - self.max_lines)
        self.set_state({"log_count": len(logs)})
    
    def render(self) -> RenderableType:
        """Render logs as a scrollable table"""
        table = Table(
            show_header=True,
            expand=True,
            box=None,
            padding=(0, 1),
            show_lines=False
        )
        
        # Get terminal width from props
        terminal_width = self.props.get("terminal_width", 80)
        
        # Responsive column configuration
        show_source = terminal_width >= 100
        show_milliseconds = terminal_width >= 80
        
        # Add columns with responsive widths
        if terminal_width < 60:
            # Ultra compact mode
            table.add_column("T", style="dim cyan", width=8)
            table.add_column("L", width=5)
            table.add_column("Message", ratio=1)
        elif terminal_width < 100:
            # Compact mode
            table.add_column("Time", style="dim cyan", width=8 if not show_milliseconds else 12)
            table.add_column("Level", width=6)
            table.add_column("Message", ratio=1)
        else:
            # Full mode
            table.add_column("Time", style="dim cyan", width=12)
            table.add_column("Level", width=8)
            table.add_column("Message", ratio=1)
            if show_source:
                table.add_column("Source", style="dim white", width=12)
        
        # Filter logs if needed
        filtered_logs = self._filter_logs()
        
        # Get visible logs based on scroll position
        start_idx = self.scroll_offset
        end_idx = min(start_idx + self.max_lines, len(filtered_logs))
        visible_logs = filtered_logs[start_idx:end_idx]
        
        # Add log entries to table
        terminal_width = self.props.get("terminal_width", 80)
        show_source = terminal_width >= 100
        show_milliseconds = terminal_width >= 80
        
        for log in visible_logs:
            # Format time based on available space
            if show_milliseconds:
                time_str = log["timestamp"].strftime("%H:%M:%S.%f")[:-3]
            else:
                time_str = log["timestamp"].strftime("%H:%M:%S")
            
            # Format level based on available space
            if terminal_width < 60:
                level_text = self._format_level_compact(log["level"])
            else:
                level_text = self._format_level(log["level"])
            
            # Calculate available width for message
            message_width = self._calculate_message_width(terminal_width, show_source)
            message = self._truncate_message(log["message"], message_width)
            
            # Build row based on visible columns
            if show_source:
                source = log.get("source", "-")[:12]  # Limit source length
                table.add_row(time_str, level_text, message, source)
            else:
                table.add_row(time_str, level_text, message)
        
        # Add empty rows if needed to maintain consistent height
        terminal_width = self.props.get("terminal_width", 80)
        show_source = terminal_width >= 100
        
        for _ in range(self.max_lines - len(visible_logs)):
            if show_source:
                table.add_row("", "", "", "")
            else:
                table.add_row("", "", "")
        
        # Update title with scroll indicator
        if len(filtered_logs) > self.max_lines:
            scroll_pct = int((self.scroll_offset / max(1, len(filtered_logs) - self.max_lines)) * 100)
            self.title = f"📋 System Logs [{len(visible_logs)}/{len(filtered_logs)}] [{scroll_pct}%]"
        else:
            self.title = f"📋 System Logs [{len(visible_logs)}/{len(filtered_logs)}]"
        
        return self.create_panel(table)
    
    def _filter_logs(self) -> List[LogEntry]:
        """Filter logs based on current filter level"""
        if not self.filter_level:
            return self.logs
        
        level_priority = {
            "DEBUG": 0,
            "INFO": 1,
            "MESSAGE": 2,
            "WARNING": 3,
            "ERROR": 4
        }
        
        min_priority = level_priority.get(self.filter_level.upper(), 0)
        
        return [
            log for log in self.logs
            if level_priority.get(log["level"].upper(), 0) >= min_priority
        ]
    
    def _format_level(self, level: str) -> Text:
        """Format log level with color coding"""
        level_upper = level.upper()
        colors = {
            "ERROR": "red bold",
            "WARNING": "yellow",
            "INFO": "blue",
            "DEBUG": "dim white",
            "MESSAGE": "white"
        }
        
        symbols = {
            "ERROR": "✗",
            "WARNING": "⚠",
            "INFO": "ℹ",
            "DEBUG": "●",
            "MESSAGE": "→"
        }
        
        symbol = symbols.get(level_upper, "•")
        color = colors.get(level_upper, "white")
        
        return Text(f"{symbol} {level_upper[:5]}", style=color)
    
    def _truncate_message(self, message: str, max_length: int = 0) -> str:
        """Truncate long messages based on available space"""
        # Use provided max_length or calculate based on terminal width
        if max_length <= 0:
            terminal_width = self.props.get("terminal_width", 80)
            max_length = self._calculate_message_width(terminal_width, False)
        
        if len(message) <= max_length:
            return message
        return message[:max_length - 3] + "..."
    
    def _calculate_message_width(self, terminal_width: int, show_source: bool) -> int:
        """Calculate available width for message column"""
        # Account for borders, padding, and other columns
        # Time: 8-12 chars, Level: 5-8 chars, Source: 12 chars (if shown)
        # Padding and borders: ~10 chars
        
        if terminal_width < 60:
            # Ultra compact: T(8) + L(5) + padding(6) = 19
            return max(20, terminal_width - 19)
        elif terminal_width < 100:
            # Compact: Time(8-12) + Level(6) + padding(8) = 22-26
            return max(30, terminal_width - 26)
        else:
            # Full mode
            if show_source:
                # Time(12) + Level(8) + Source(12) + padding(10) = 42
                return max(40, terminal_width - 42)
            else:
                # Time(12) + Level(8) + padding(8) = 28
                return max(40, terminal_width - 28)
    
    def _format_level_compact(self, level: str) -> Text:
        """Format log level in compact mode"""
        level_upper = level.upper()
        colors = {
            "ERROR": "red bold",
            "WARNING": "yellow",
            "INFO": "blue",
            "DEBUG": "dim white",
            "MESSAGE": "white"
        }
        
        symbols = {
            "ERROR": "E",
            "WARNING": "W",
            "INFO": "I",
            "DEBUG": "D",
            "MESSAGE": "M"
        }
        
        symbol = symbols.get(level_upper, "?")
        color = colors.get(level_upper, "white")
        
        return Text(symbol, style=color)
    
    def scroll_up(self, lines: int = 1) -> None:
        """Scroll up by specified number of lines"""
        self.auto_scroll = False
        self.scroll_offset = max(0, self.scroll_offset - lines)
        if self._update_callback:
            self._update_callback()
    
    def scroll_down(self, lines: int = 1) -> None:
        """Scroll down by specified number of lines"""
        max_offset = max(0, len(self._filter_logs()) - self.max_lines)
        self.scroll_offset = min(max_offset, self.scroll_offset + lines)
        
        # Re-enable auto-scroll if at bottom
        if self.scroll_offset >= max_offset:
            self.auto_scroll = True
        
        if self._update_callback:
            self._update_callback()
    
    def scroll_to_top(self) -> None:
        """Scroll to top of logs"""
        self.auto_scroll = False
        self.scroll_offset = 0
        if self._update_callback:
            self._update_callback()
    
    def scroll_to_bottom(self) -> None:
        """Scroll to bottom of logs"""
        self.auto_scroll = True
        self.scroll_offset = max(0, len(self._filter_logs()) - self.max_lines)
        if self._update_callback:
            self._update_callback()
    
    def set_filter(self, level: Optional[str]) -> None:
        """Set log level filter"""
        self.filter_level = level
        self.scroll_offset = 0  # Reset scroll when filter changes
        if self._update_callback:
            self._update_callback()
    
    def clear_logs(self) -> None:
        """Clear all logs"""
        if self._state_manager:
            from state.actions import Action, ActionType
            self._state_manager.dispatch(Action(ActionType.LOG_CLEAR))