# Hardware Abstraction Layer (HAL)

## Overview

The HAL provides a clean separation between hardware-specific implementations and application logic, enabling:
- Easy hardware variant support
- Configuration-driven hardware selection
- Testable interfaces
- Resource-constrained embedded systems

## Architecture

### Layer Structure

```
Application Layer
    ↓
HAL Interface Layer
    ↓
Hardware Drivers
    ↓
Physical Hardware
```

### Key Components

#### 1. **Core** (`HAL/Core/`)
- `hal_types.h`: Common types and status codes
- `hal_errors.h`: Error handling and logging

#### 2. **Display** (`HAL/Display/`)
- `display_interface.h`: Abstract display interface
- `Drivers/`: Hardware-specific implementations
  - `ssd1306_driver`: SSD1306 OLED display driver

#### 3. **Input** (`HAL/Input/`)
- `input_interface.h`: Abstract input interface
- `Drivers/`: Hardware-specific implementations
  - `shift_register_74hc165`: 74HC165 shift register driver

#### 4. **Board** (`HAL/Board/`)
- Board Support Packages (BSP) for each hardware platform
- `handheld_bsp`: Handheld controller BSP

## Usage

### Basic Initialization

```cpp
#include "HAL/Board/handheld_bsp.h"

handheld_hardware_t hardware;

void setup() {
    hal_status_t status = handheld_bsp_init(&hardware);
    if (status != HAL_OK) {
        // Handle error
    }
}
```

### Using Display Interface

```cpp
const display_interface_t* display = hardware.display->interface;

// Clear display
display->clear(hardware.display);

// Draw text
display_point_t cursor = {10, 20};
display->set_text_cursor(hardware.display, &cursor);
display->write_text(hardware.display, "Hello", 1);

// Refresh display
display->refresh(hardware.display);
```

### Using Input Interface

```cpp
const input_interface_t* input = hardware.input->interface;

// Update input state
input->update(hardware.input);

// Read button state
bool pressed;
input->read_channel(hardware.input, 0, &pressed);

// Get events
input_event_data_t events[8];
uint8_t num_events;
input->get_events(hardware.input, events, 8, &num_events);
```

## Hardware Configuration

Hardware profiles are defined in `Config/handheld_config.h`:

```cpp
static const hardware_profile_t handheld_v1_profile = {
    .display = {
        .driver_type = DISPLAY_DRIVER_SSD1306,
        .params = {
            .ssd1306 = {
                .width = 128,
                .height = 64,
                .i2c_address = 0x3C,
                // ...
            }
        }
    },
    .input = {
        .driver_type = INPUT_DRIVER_SHIFT_REGISTER,
        .params = {
            .shift_register = {
                .load_pin = 25,
                .clock_pin = 26,
                .data_pin = 27,
                // ...
            }
        }
    }
};
```

## Adding New Hardware

### 1. Create Driver

Implement the appropriate interface:
- For displays: `display_interface_t`
- For inputs: `input_interface_t`

### 2. Add Configuration

Define hardware parameters in configuration profile.

### 3. Update BSP

Modify BSP to instantiate new driver based on configuration.

## Benefits

- **Portability**: Same application code works with different hardware
- **Testability**: Interfaces can be mocked for unit testing
- **Scalability**: Easy to add new hardware variants
- **Maintainability**: Clear separation of concerns
- **MISRA-C Compliance**: Follows embedded best practices