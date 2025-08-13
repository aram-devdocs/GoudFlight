# ATC Dashboard - Base Station Monitor

A modern, React-like terminal dashboard for monitoring and controlling the ESP32 base station. Features real-time telemetry, system logs, performance metrics, and command interface.

![Python](https://img.shields.io/badge/python-3.9%2B-blue)
![Terminal UI](https://img.shields.io/badge/UI-Rich%20Terminal-green)
![Architecture](https://img.shields.io/badge/Architecture-Component%20Based-orange)

## 🚀 Features

- **Real-time Monitoring**: Live telemetry data from ESP32 base station
- **View Switching**: Quickly switch between dashboard and fullscreen views using number keys
- **Component-Based Architecture**: React-like components with independent refresh rates
- **State Management**: Redux-like centralized state with actions and subscriptions
- **Rich Terminal UI**: Professional dashboard with panels, tables, and visualizations
- **Type Safety**: Full type hints with no `Any` types
- **Performance Metrics**: Track bytes, messages, errors, and latency
- **Scrollable Logs**: Color-coded log viewer with filtering
- **Command Interface**: Send commands with history tracking

## 📋 Requirements

- Python 3.9+
- Linux/macOS (terminal with UTF-8 support)
- Serial port access (for ESP32 connection)

## 🛠️ Installation

1. **Clone the repository**:
```bash
cd ~/GoudFlight/atc
```

2. **Create virtual environment** (recommended):
```bash
python3 -m venv .venv
source .venv/bin/activate
```

3. **Install dependencies**:
```bash
pip install -r requirements.txt
```

4. **Install development dependencies** (optional):
```bash
pip install -r requirements-dev.txt
```


## 🎮 Usage

### Quick Start
```bash
./start.sh
```

### Manual Start
```bash
python3 main.py --port /dev/ttyUSB0 --baudrate 115200
```

### Command Line Options
- `-p, --port`: Serial port (default: /dev/ttyUSB0)
- `-b, --baudrate`: Baud rate (default: 115200)
- `-v, --verbose`: Enable verbose logging
- `--no-keyboard`: Disable keyboard input (for non-TTY environments)

### Keyboard Controls

#### View Navigation
- `0` - Return to dashboard view (all panels)
- `1` - Telemetry panel (fullscreen)
- `2` - Status panel (fullscreen)
- `3` - Logs panel (fullscreen)
- `4` - Command panel (fullscreen)
- `5` - Metrics panel (fullscreen)
- `6` - Cycle through all views

#### General Controls
- `Q` - Quit application
- `Tab` - Switch focus between panels (dashboard view)
- `↑/↓` - Scroll in log panel (when focused)
- `C` - Clear logs

#### Quick Commands
- `S` - Send status command
- `T` - Send telemetry command
- `R` - Send reset command

## 🏗️ Architecture

### Component System
```
Dashboard
├── TelemetryPanel     # Real-time telemetry display
├── LogPanel          # Scrollable system logs
├── StatusPanel       # Connection status
├── CommandPanel      # Command interface
└── MetricsPanel      # Performance metrics
```

### State Management
```
StateManager (Redux-like)
├── Actions          # State update actions
├── Reducers         # State transformation logic
├── Subscriptions    # Component subscriptions
└── Middleware       # Action processing
```

### Directory Structure
```
atc/
├── main.py                 # Main application entry
├── dashboard.py            # Dashboard orchestrator
├── components/            # UI components
│   ├── base_component.py
│   ├── telemetry_panel.py
│   ├── log_panel.py
│   ├── status_panel.py
│   ├── command_panel.py
│   └── metrics_panel.py
├── state/                 # State management
│   ├── state_manager.py
│   ├── actions.py
│   └── types.py
└── start.sh              # Startup script
```

## 🧪 Development

### Code Quality
- Type hints throughout the codebase
- Component-based architecture
- Real-time monitoring capabilities

### Makefile Commands
- `make install` - Install production dependencies
- `make install-dev` - Install development dependencies
- `make clean` - Remove cache files
- `make run` - Run the application

## 📡 Serial Communication

The dashboard communicates with the ESP32 base station via UART serial connection.

### Message Format
```json
{
    "type": "TELEMETRY|STATUS|HEARTBEAT|EVENT|ERROR",
    "data": {
        "system_status": "READY",
        "esp_now_connected": true,
        "remote_devices": 2,
        "signal_strength": -45,
        "battery_voltage": 3.7,
        "uptime_ms": 120000,
        "free_heap": 45000,
        "cpu_usage": 25.5
    }
}
```

### Commands
- `PING` - Heartbeat check
- `GET_TELEMETRY` - Request telemetry data
- `GET_STATUS` - Request system status
- `RESET` - Reset ESP32
- `SET_MODE` - Change operating mode
- `CALIBRATE` - Start calibration

## 🐛 Troubleshooting

### No Display / Terminal Issues
- Ensure terminal supports UTF-8
- Try running with `--no-keyboard` flag
- Check terminal size (minimum 80x24)

### Serial Connection Issues
- Check port permissions: `sudo chmod 666 /dev/ttyUSB0`
- Verify correct port: `ls /dev/tty*`
- Ensure ESP32 is powered and connected

### Import Errors
- Ensure virtual environment is activated
- Reinstall dependencies: `pip install -r requirements.txt`
- Check Python version (3.9+ required)

## 📚 Documentation

- [Architecture Overview](docs/architecture.md)
- [Component Development](docs/components.md)
- [State Management](docs/state.md)
- [Serial Protocol](docs/protocol.md)

## 🤝 Contributing

1. Follow type hints - no `Any` types
2. Add documentation for new components
3. Test changes with hardware
4. Update documentation as needed

## 📄 License

This project is part of the GoudFlight system.

## 🙏 Acknowledgments

- Built with [Rich](https://github.com/Textualize/rich) for terminal UI
- Inspired by React component architecture
- Redux-like state management pattern