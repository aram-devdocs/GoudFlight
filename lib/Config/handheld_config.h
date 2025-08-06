#ifndef HANDHELD_CONFIG_H
#define HANDHELD_CONFIG_H

#include "hardware_profiles.h"

// ESP32-S3-N16R8 Pin Mapping
// Available GPIOs: 1-21, 33-48
// Avoid: GPIO 19-20 (USB), GPIO 26-32 (SPI Flash)
// I2C recommended: GPIO 8 (SDA), GPIO 9 (SCL)

static const hardware_profile_t handheld_profile = {
    .header = {
        .magic_number = HAL_CONFIG_MAGIC,
        .version = HAL_CONFIG_VERSION,
        .board_type = HAL_BOARD_HANDHELD
    },
    .profile_id = HW_PROFILE_HANDHELD_V1,
    .board_name = "Handheld Controller",
    .display = {
        .driver_type = DISPLAY_DRIVER_SSD1306,
        .params = {
            .ssd1306 = {
                .width = 128,
                .height = 64,
                .i2c_address = 0x3C,
                .reset_pin = -1,
                .sda_pin = 8,  // ESP32-S3 I2C SDA
                .scl_pin = 9   // ESP32-S3 I2C SCL
            }
        }
    },
    .input = {
        .driver_type = INPUT_DRIVER_SHIFT_REGISTER,
        .params = {
            .shift_register = {
                .load_pin = 10,  // ESP32-S3 GPIO
                .clock_pin = 11, // ESP32-S3 GPIO
                .data_pin = 12,  // ESP32-S3 GPIO
                .num_registers = 1,
                .clock_delay_us = 5,
                .load_delay_us = 5
            }
        }
    },
    .resource_limits = {
        .max_stack_bytes = 4096,
        .max_heap_bytes = 8192,
        .max_cpu_percent = 80
    }
};

#define HANDHELD_HARDWARE_PROFILE handheld_profile

#endif