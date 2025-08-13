#!/usr/bin/env python3
"""
Actions - Define all possible state update actions
Similar to Redux actions in React
"""

from enum import Enum
from typing import Dict, Optional, Union
from datetime import datetime
from state.types import ActionPayload, TelemetryData, MetricsData


class ActionType(Enum):
    """Enumeration of all action types"""
    
    # Connection actions
    CONNECTION_ESTABLISHED = "CONNECTION_ESTABLISHED"
    CONNECTION_LOST = "CONNECTION_LOST"
    CONNECTION_ERROR = "CONNECTION_ERROR"
    
    # Telemetry actions
    TELEMETRY_UPDATE = "TELEMETRY_UPDATE"
    TELEMETRY_RESET = "TELEMETRY_RESET"
    
    # Log actions
    LOG_MESSAGE = "LOG_MESSAGE"
    LOG_ERROR = "LOG_ERROR"
    LOG_WARNING = "LOG_WARNING"
    LOG_INFO = "LOG_INFO"
    LOG_DEBUG = "LOG_DEBUG"
    LOG_CLEAR = "LOG_CLEAR"
    
    # System status actions
    STATUS_UPDATE = "STATUS_UPDATE"
    HEARTBEAT_RECEIVED = "HEARTBEAT_RECEIVED"
    HEARTBEAT_TIMEOUT = "HEARTBEAT_TIMEOUT"
    
    # Command actions
    COMMAND_SENT = "COMMAND_SENT"
    COMMAND_RESPONSE = "COMMAND_RESPONSE"
    COMMAND_ERROR = "COMMAND_ERROR"
    
    # Metrics actions
    METRICS_UPDATE = "METRICS_UPDATE"
    METRICS_RESET = "METRICS_RESET"
    
    # UI actions
    UI_REFRESH = "UI_REFRESH"
    UI_RESIZE = "UI_RESIZE"
    UI_FOCUS_CHANGE = "UI_FOCUS_CHANGE"
    VIEW_MODE_CHANGE = "VIEW_MODE_CHANGE"


class Action:
    """Action class to encapsulate action type and payload"""
    
    def __init__(self, action_type: ActionType, payload: Optional[ActionPayload] = None, metadata: Optional[Dict[str, Union[str, int, float, bool]]] = None):
        self.type = action_type
        self.payload = payload
        self.metadata = metadata or {}
        self.timestamp: Optional[datetime] = None  # Will be set by dispatcher
    
    def to_dict(self) -> Dict[str, Union[str, dict, Optional[datetime]]]:
        """Convert action to dictionary"""
        return {
            'type': self.type.value,
            'payload': self.payload,
            'metadata': self.metadata,
            'timestamp': self.timestamp
        }


# Action creators - Factory functions for creating actions

def connection_established(port: str, baudrate: int) -> Action:
    """Create action for successful connection"""
    return Action(
        ActionType.CONNECTION_ESTABLISHED,
        {'port': port, 'baudrate': baudrate}
    )


def connection_lost(reason: str = None) -> Action:
    """Create action for lost connection"""
    return Action(ActionType.CONNECTION_LOST, {'reason': reason})


def connection_error(error_data: dict) -> Action:
    """Create action for connection error"""
    return Action(ActionType.CONNECTION_ERROR, error_data)


def telemetry_update(data: TelemetryData) -> Action:
    """Create action for telemetry data update"""
    return Action(ActionType.TELEMETRY_UPDATE, data)


def log_message(level: str, message: str, source: str = None) -> Action:
    """Create action for log message"""
    action_type = {
        'ERROR': ActionType.LOG_ERROR,
        'WARNING': ActionType.LOG_WARNING,
        'INFO': ActionType.LOG_INFO,
        'DEBUG': ActionType.LOG_DEBUG
    }.get(level.upper(), ActionType.LOG_MESSAGE)
    
    return Action(
        action_type,
        {'message': message, 'source': source}
    )


def status_update(status: str, details: Optional[Dict[str, Union[str, int, float, bool]]] = None) -> Action:
    """Create action for system status update"""
    return Action(
        ActionType.STATUS_UPDATE,
        {'status': status, 'details': details or {}}
    )


def command_sent(command: str, params: Optional[Dict[str, Union[str, int, float, bool]]] = None) -> Action:
    """Create action for sent command"""
    return Action(
        ActionType.COMMAND_SENT,
        {'command': command, 'params': params}
    )


def metrics_update(metrics: MetricsData) -> Action:
    """Create action for metrics update"""
    return Action(ActionType.METRICS_UPDATE, metrics)


def heartbeat_received() -> Action:
    """Create action for heartbeat received"""
    return Action(ActionType.HEARTBEAT_RECEIVED)


def view_mode_change(mode: str, component: Optional[str] = None) -> Action:
    """Create action for view mode change"""
    return Action(
        ActionType.VIEW_MODE_CHANGE,
        {'mode': mode, 'component': component}
    )