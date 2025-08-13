#!/usr/bin/env python3
"""
Base Component - Abstract base class for dashboard UI components
Implements React-like component lifecycle and state management
"""

from abc import ABC, abstractmethod
from typing import Dict, Optional, Callable, Union
from rich.console import RenderableType
from rich.panel import Panel
from datetime import datetime
import threading


class BaseComponent(ABC):
    """Abstract base class for all dashboard components"""
    
    def __init__(self, title: str = "Component", refresh_rate: float = 1.0):
        self.title = title
        self.refresh_rate = refresh_rate
        self.state: Dict[str, Union[str, int, float, bool, dict, list]] = {}
        self.props: Dict[str, Union[str, int, float, bool, dict, list]] = {}
        self._mounted = False
        self._update_callback: Optional[Callable] = None
        self._state_manager = None
        self._last_update = datetime.now()
        self._lock = threading.Lock()
        
        self.initialize()
    
    def initialize(self):
        """Initialize component - called once during construction"""
        pass
    
    @abstractmethod
    def render(self) -> RenderableType:
        """Render the component - must return a Rich renderable object"""
        pass
    
    def set_state(self, updates: Dict[str, Union[str, int, float, bool, dict, list]], trigger_render: bool = True):
        """Update component state and optionally trigger re-render"""
        with self._lock:
            self.state.update(updates)
            if trigger_render and self._update_callback:
                self._update_callback()
    
    def get_state(self, key: str, default: Optional[Union[str, int, float, bool, dict, list]] = None) -> Optional[Union[str, int, float, bool, dict, list]]:
        """Get state value by key"""
        with self._lock:
            return self.state.get(key, default)
    
    def set_props(self, props: Dict[str, Union[str, int, float, bool, dict, list]]):
        """Update component props"""
        with self._lock:
            self.props.update(props)
    
    def mount(self, update_callback: Callable, state_manager: 'StateManager'):
        """Mount component to dashboard"""
        self._mounted = True
        self._update_callback = update_callback
        self._state_manager = state_manager
        self.on_mount()
    
    def unmount(self):
        """Unmount component from dashboard"""
        self._mounted = False
        self.on_unmount()
    
    def on_mount(self):
        """Lifecycle method - called when component is mounted"""
        pass
    
    def on_unmount(self):
        """Lifecycle method - called when component is unmounted"""
        pass
    
    def should_update(self) -> bool:
        """Determine if component should update based on refresh rate"""
        now = datetime.now()
        elapsed = (now - self._last_update).total_seconds()
        if elapsed >= self.refresh_rate:
            self._last_update = now
            return True
        return False
    
    def dispatch(self, action_type: str, payload: Optional[Dict[str, Union[str, int, float, bool]]] = None):
        """Dispatch action to state manager"""
        if self._state_manager:
            self._state_manager.dispatch(action_type, payload)
    
    def subscribe(self, state_key: str, callback: Callable):
        """Subscribe to state changes"""
        if self._state_manager:
            self._state_manager.subscribe(state_key, callback)
    
    def create_panel(self, content: RenderableType, **kwargs) -> Panel:
        """Helper to create a panel with consistent styling"""
        return Panel(
            content,
            title=self.title,
            border_style="bright_blue",
            **kwargs
        )