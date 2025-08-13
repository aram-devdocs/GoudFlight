#!/usr/bin/env python3
"""
State Manager - Central state store with Redux-like patterns
Manages application state with immutable updates and subscriptions
"""

import threading
from typing import Dict, List, Callable, Optional
from datetime import datetime
from collections import defaultdict
import copy

from state.types import AppState, StateValue, TelemetryData, ConnectionInfo, MetricsData, LogEntry, CommandHistory
from state.actions import Action, ActionType


class StateManager:
    """Central state manager with observer pattern"""
    
    def __init__(self):
        self._state: AppState = self._create_initial_state()
        self._subscribers: Dict[str, List[Callable[[StateValue], None]]] = defaultdict(list)
        self._middleware: List[Callable[[Action, AppState], Optional[Action]]] = []
        self._lock = threading.RLock()
        self._history: List[Action] = []
        self._max_history = 100
        self._max_logs = 1000
        self._max_command_history = 50
    
    def _create_initial_state(self) -> AppState:
        """Create initial application state"""
        return AppState(
            connection=ConnectionInfo(
                status="DISCONNECTED",
                port="/dev/ttyUSB0",
                baudrate=115200,
                last_heartbeat=None,
                error_message=None
            ),
            telemetry=TelemetryData(
                timestamp=None,
                system_status="UNKNOWN",
                esp_now_connected=False,
                remote_devices=0,
                signal_strength=0,
                battery_voltage=0.0,
                uptime_ms=0,
                free_heap=0,
                cpu_usage=0.0
            ),
            logs=[],
            command_history=[],
            metrics=MetricsData(
                rx_bytes=0,
                tx_bytes=0,
                rx_messages=0,
                tx_messages=0,
                error_count=0,
                uptime_seconds=0.0,
                message_rate=0.0,
                latency_ms=0.0
            ),
            ui_focus="telemetry",
            ui_dimensions={"width": 80, "height": 24},
            view_mode="dashboard",
            view_component=None
        )
    
    def get_state(self) -> AppState:
        """Get current state (returns a copy to prevent mutations)"""
        with self._lock:
            return copy.deepcopy(self._state)
    
    def get_state_value(self, key: str) -> Optional[StateValue]:
        """Get specific state value by key"""
        with self._lock:
            return self._state.get(key)
    
    def dispatch(self, action: Action) -> None:
        """Dispatch an action to update state"""
        with self._lock:
            # Set timestamp
            action.timestamp = datetime.now()
            
            # Apply middleware
            for middleware in self._middleware:
                result = middleware(action, self._state)
                if result is None:
                    return  # Middleware blocked the action
                action = result
            
            # Add to history
            self._history.append(action)
            if len(self._history) > self._max_history:
                self._history.pop(0)
            
            # Apply action to state
            self._apply_action(action)
            
            # Notify subscribers
            self._notify_subscribers(action)
    
    def _apply_action(self, action: Action) -> None:
        """Apply action to state (reducer logic)"""
        if action.type == ActionType.CONNECTION_ESTABLISHED:
            self._state["connection"]["status"] = "CONNECTED"
            self._state["connection"]["port"] = action.payload["port"]
            self._state["connection"]["baudrate"] = action.payload["baudrate"]
            self._state["connection"]["error_message"] = None
        
        elif action.type == ActionType.CONNECTION_LOST:
            self._state["connection"]["status"] = "DISCONNECTED"
            if action.payload and "reason" in action.payload:
                self._state["connection"]["error_message"] = action.payload["reason"]
        
        elif action.type == ActionType.CONNECTION_ERROR:
            self._state["connection"]["status"] = "ERROR"
            if action.payload and "message" in action.payload:
                self._state["connection"]["error_message"] = action.payload["message"]
        
        elif action.type == ActionType.TELEMETRY_UPDATE:
            if action.payload:
                for key, value in action.payload.items():
                    if key in self._state["telemetry"]:
                        self._state["telemetry"][key] = value
                self._state["telemetry"]["timestamp"] = datetime.now().isoformat()
        
        elif action.type == ActionType.TELEMETRY_RESET:
            self._state["telemetry"] = self._create_initial_state()["telemetry"]
        
        elif action.type in [ActionType.LOG_MESSAGE, ActionType.LOG_ERROR, 
                            ActionType.LOG_WARNING, ActionType.LOG_INFO, ActionType.LOG_DEBUG]:
            log_entry = LogEntry(
                timestamp=action.timestamp,
                level=action.type.name.replace("LOG_", ""),
                message=action.payload.get("message", ""),
                source=action.payload.get("source")
            )
            self._state["logs"].append(log_entry)
            
            # Trim logs if too many
            if len(self._state["logs"]) > self._max_logs:
                self._state["logs"] = self._state["logs"][-self._max_logs:]
        
        elif action.type == ActionType.LOG_CLEAR:
            self._state["logs"] = []
        
        elif action.type == ActionType.STATUS_UPDATE:
            if action.payload:
                self._state["telemetry"]["system_status"] = action.payload.get("status", "UNKNOWN")
        
        elif action.type == ActionType.HEARTBEAT_RECEIVED:
            self._state["connection"]["last_heartbeat"] = action.timestamp.timestamp()
        
        elif action.type == ActionType.HEARTBEAT_TIMEOUT:
            self._state["connection"]["status"] = "TIMEOUT"
        
        elif action.type == ActionType.COMMAND_SENT:
            cmd_entry = CommandHistory(
                command=action.payload.get("command", ""),
                params=action.payload.get("params"),
                timestamp=action.timestamp,
                response=None,
                success=False
            )
            self._state["command_history"].append(cmd_entry)
            
            # Trim command history
            if len(self._state["command_history"]) > self._max_command_history:
                self._state["command_history"] = self._state["command_history"][-self._max_command_history:]
        
        elif action.type == ActionType.COMMAND_RESPONSE:
            # Update the last command with response
            if self._state["command_history"] and action.payload:
                self._state["command_history"][-1]["response"] = action.payload.get("response")
                self._state["command_history"][-1]["success"] = action.payload.get("success", True)
        
        elif action.type == ActionType.METRICS_UPDATE:
            if action.payload:
                for key, value in action.payload.items():
                    if key in self._state["metrics"]:
                        self._state["metrics"][key] = value
        
        elif action.type == ActionType.METRICS_RESET:
            self._state["metrics"] = self._create_initial_state()["metrics"]
        
        elif action.type == ActionType.UI_FOCUS_CHANGE:
            if action.payload and "focus" in action.payload:
                self._state["ui_focus"] = action.payload["focus"]
        
        elif action.type == ActionType.VIEW_MODE_CHANGE:
            if action.payload:
                self._state["view_mode"] = action.payload.get("mode", "dashboard")
                self._state["view_component"] = action.payload.get("component", None)
        
        elif action.type == ActionType.UI_RESIZE:
            if action.payload:
                if "width" in action.payload:
                    self._state["ui_dimensions"]["width"] = action.payload["width"]
                if "height" in action.payload:
                    self._state["ui_dimensions"]["height"] = action.payload["height"]
    
    def subscribe(self, state_key: str, callback: Callable[[StateValue], None]) -> Callable[[], None]:
        """Subscribe to state changes for a specific key"""
        with self._lock:
            self._subscribers[state_key].append(callback)
            
            # Return unsubscribe function
            def unsubscribe():
                with self._lock:
                    if callback in self._subscribers[state_key]:
                        self._subscribers[state_key].remove(callback)
            
            return unsubscribe
    
    def _notify_subscribers(self, action: Action) -> None:
        """Notify relevant subscribers after state change"""
        # Determine which state keys were affected
        affected_keys = set()
        
        if action.type.name.startswith("CONNECTION_"):
            affected_keys.add("connection")
        elif action.type.name.startswith("TELEMETRY_"):
            affected_keys.add("telemetry")
        elif action.type.name.startswith("LOG_"):
            affected_keys.add("logs")
        elif action.type.name.startswith("COMMAND_"):
            affected_keys.add("command_history")
        elif action.type.name.startswith("METRICS_"):
            affected_keys.add("metrics")
        elif action.type.name.startswith("UI_"):
            affected_keys.add("ui_focus")
            affected_keys.add("ui_dimensions")
        
        # Notify subscribers for affected keys
        for key in affected_keys:
            if key in self._subscribers:
                value = self._state.get(key)
                for callback in self._subscribers[key]:
                    try:
                        callback(value)
                    except Exception as e:
                        # Log error but don't crash
                        print(f"Subscriber error for {key}: {e}")
    
    def add_middleware(self, middleware: Callable[[Action, AppState], Optional[Action]]) -> None:
        """Add middleware function to process actions before they reach the reducer"""
        self._middleware.append(middleware)
    
    def get_history(self) -> List[Action]:
        """Get action history for debugging"""
        with self._lock:
            return self._history.copy()