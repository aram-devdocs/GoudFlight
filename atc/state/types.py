#!/usr/bin/env python3
"""
Type definitions for state management system
"""

from typing import TypedDict, Optional, List, Dict, Union
from datetime import datetime


class TelemetryData(TypedDict):
    """Type definition for telemetry data"""
    timestamp: Optional[str]
    system_status: str
    esp_now_connected: bool
    remote_devices: int
    signal_strength: int
    battery_voltage: float
    uptime_ms: int
    free_heap: int
    cpu_usage: float


class LogEntry(TypedDict):
    """Type definition for log entries"""
    timestamp: datetime
    level: str
    message: str
    source: Optional[str]


class ConnectionInfo(TypedDict):
    """Type definition for connection information"""
    status: str  # "CONNECTED", "DISCONNECTED", "ERROR", "TIMEOUT"
    port: str
    baudrate: int
    last_heartbeat: Optional[float]
    error_message: Optional[str]


class CommandHistory(TypedDict):
    """Type definition for command history"""
    command: str
    params: Optional[Dict[str, Union[str, int, float, bool]]]
    timestamp: datetime
    response: Optional[str]
    success: bool


class MetricsData(TypedDict):
    """Type definition for performance metrics"""
    rx_bytes: int
    tx_bytes: int
    rx_messages: int
    tx_messages: int
    error_count: int
    uptime_seconds: float
    message_rate: float  # messages per second
    latency_ms: float    # average latency


class AppState(TypedDict):
    """Complete application state type definition"""
    connection: ConnectionInfo
    telemetry: TelemetryData
    logs: List[LogEntry]
    command_history: List[CommandHistory]
    metrics: MetricsData
    ui_focus: str  # which panel has focus
    ui_dimensions: Dict[str, int]  # terminal width/height


# Type aliases for common types
StateValue = Union[str, int, float, bool, dict, list]
StateUpdate = Dict[str, StateValue]
ActionPayload = Dict[str, Union[str, int, float, bool, list, dict]]