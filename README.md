# GoudFlight - Advanced Drone Control System

A comprehensive ESP32-based drone control system featuring handheld controllers, base station monitoring, and advanced telemetry dashboard.

## 🎯 Project Overview

GoudFlight is a modular drone control ecosystem consisting of:

- **Handheld Controller**: ESP32-based remote control with OLED display and button input
- **Drone Flight Controller**: ESP32-based flight control system
- **Base Station**: Ground control station with LCD display and serial interface
- **ATC Dashboard**: Python-based monitoring dashboard with rich terminal UI

## 📁 Project Structure

```
GoudFlight/
├── src/                    # ESP32 firmware source code
│   ├── handheld_controller/    # Handheld remote control
│   ├── drone_flight_controller/ # Drone flight control
│   └── base_station/           # Base station firmware
├── lib/                    # Shared libraries
│   ├── Core/              # Core framework (AppFramework, Logger, State)
│   ├── HAL/               # Hardware abstraction layer
│   ├── Communication/     # ESP-NOW and serial protocols
│   ├── Display/           # Display drivers and management
│   └── Business/          # Business logic components
├── atc/                   # ATC Dashboard (Python)
│   ├── dashboard.py       # Main dashboard application
│   ├── components/        # UI components
│   └── state/            # State management
└── scripts/              # Build and utility scripts
```

## 🚀 Quick Start

### ESP32 Firmware

1. **Build firmware**:
```bash
./build.sh all              # Build all targets
./build.sh handheld        # Build handheld only
./build.sh base            # Build base station only
```

2. **Upload to board**:
```bash
./upload.sh handheld       # Upload to handheld
./upload.sh base           # Upload to base station
```

### ATC Dashboard

1. **Navigate to ATC directory**:
```bash
cd atc
```

2. **Install dependencies**:
```bash
pip install -r requirements.txt
```

3. **Start dashboard**:
```bash
./start.sh
```

## 🔧 Hardware Requirements

### Handheld Controller
- ESP32 DevKit
- SSD1306 OLED Display (128x64)
- 74HC165 Shift Register for button input
- Push buttons
- 3.7V LiPo battery

### Base Station
- ESP32 DevKit
- LCD1602 I2C Display
- UART connection for telemetry
- Power supply

### ATC Dashboard
- Linux/macOS with Python 3.9+
- Terminal with UTF-8 support
- Serial port access

## 📡 Communication

### ESP-NOW Protocol
- Direct ESP32-to-ESP32 communication
- Low latency (<10ms)
- Range up to 200m (open space)
- Automatic reconnection

### Serial Protocol
- JSON-based messages
- 115200 baud rate
- Bidirectional communication
- Real-time telemetry

## 🏗️ Architecture

### Firmware Architecture
- **AppFramework Pattern**: Consistent lifecycle management
- **Screen-Based UI**: Modular screen system
- **Hardware Abstraction**: Platform-independent interfaces
- **State Management**: Centralized state with observers

### Dashboard Architecture
- **Component-Based**: React-like component system
- **State Management**: Redux-like centralized state
- **Type Safety**: Full type hints throughout
- **Real-time Updates**: Independent component refresh rates

## 🧪 Development

### Pre-commit Hooks
Install development hooks for code quality:
```bash
cd atc && ./install-hooks.sh
```

### Validation
Always validate before pushing:
```bash
cd atc && ./pre-push.sh
```

### Code Quality
- MISRA-C compliance for firmware
- Type safety in Python (no `Any` types)
- Comprehensive validation scripts
- Pre-commit hooks for consistency

## 📚 Documentation

- [Hardware Setup Guide](docs/hardware_setup.md)
- [Communication Protocol](docs/protocol.md)
- [ATC Dashboard README](atc/README.md)
- [HAL Documentation](lib/HAL/README.md)

## 🤝 Contributing

1. Follow the design principles in [CLAUDE.md](CLAUDE.md)
2. Run validation before commits
3. Maintain type safety
4. Update documentation
5. Test on hardware



## 🛠️ Tools & Technologies

- **PlatformIO**: Build system and dependency management
- **ESP-IDF**: ESP32 development framework
- **Python 3.9+**: ATC dashboard
- **Rich**: Terminal UI framework
- **ESP-NOW**: Wireless communication protocol

