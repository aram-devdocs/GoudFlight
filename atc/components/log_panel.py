#!/usr/bin/env python3
"""
Log Panel - Scrollable log viewer with filtering and color coding
"""

from rich.table import Table
from rich.console import RenderableType
from rich.text import Text
from typing import List, Optional

from .base_component import BaseComponent
from ..state.types import LogEntry


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
        
        # Add columns
        table.add_column("Time", style="dim cyan", width=12)
        table.add_column("Level", width=8)
        table.add_column("Message", ratio=1)
        table.add_column("Source", style="dim white", width=15)
        
        # Filter logs if needed
        filtered_logs = self._filter_logs()
        
        # Get visible logs based on scroll position
        start_idx = self.scroll_offset
        end_idx = min(start_idx + self.max_lines, len(filtered_logs))
        visible_logs = filtered_logs[start_idx:end_idx]
        
        # Add log entries to table
        for log in visible_logs:
            time_str = log["timestamp"].strftime("%H:%M:%S.%f")[:-3]
            level_text = self._format_level(log["level"])
            message = self._truncate_message(log["message"])
            source = log.get("source", "-")
            
            table.add_row(time_str, level_text, message, source)
        
        # Add empty rows if needed to maintain consistent height
        for _ in range(self.max_lines - len(visible_logs)):
            table.add_row("", "", "", "")
        
        # Create panel with scroll indicator
        panel_title = f"📋 System Logs [{len(visible_logs)}/{len(filtered_logs)}]"
        if len(filtered_logs) > self.max_lines:
            scroll_pct = int((self.scroll_offset / max(1, len(filtered_logs) - self.max_lines)) * 100)
            panel_title += f" [{scroll_pct}%]"
        
        return self.create_panel(table, title=panel_title)
    
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
    
    def _truncate_message(self, message: str, max_length: int = 80) -> str:
        """Truncate long messages"""
        if len(message) <= max_length:
            return message
        return message[:max_length - 3] + "..."
    
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
            from ..state.actions import Action, ActionType
            self._state_manager.dispatch(Action(ActionType.LOG_CLEAR))