#ifndef BASE_STATION_CONFIG_H
#define BASE_STATION_CONFIG_H

#include "hardware_profiles.h"

// ESP32-S3 Pin Mapping for Base Station
// LCD1602 I2C Connection: GPIO 8 (SDA), GPIO 9 (SCL)
// Default I2C address for LCD1602: 0x27 (may also be 0x3F)

static const hardware_profile_t base_station_profile = {
    .header = {
        .magic_number = HAL_CONFIG_MAGIC,
        .version = HAL_CONFIG_VERSION,
        .board_type = HAL_BOARD_BASE_STATION
    },
    .profile_id = HW_PROFILE_BASE_V1,
    .board_name = "Base Station",
    .display = {
        .driver_type = DISPLAY_DRIVER_LCD1602_I2C,
        .params = {
            .lcd1602 = {
                .columns = 16,
                .rows = 2,
                .i2c_address = 0x27,  // Common LCD1602 I2C address
                .sda_pin = 8,         // ESP32-S3 I2C SDA
                .scl_pin = 9          // ESP32-S3 I2C SCL
            }
        }
    },
    .input = {
        .driver_type = INPUT_DRIVER_NONE,
        .params = {}
    },
    .resource_limits = {
        .max_stack_bytes = 4096,
        .max_heap_bytes = 8192,
        .max_cpu_percent = 50
    }
};

#define BASE_STATION_HARDWARE_PROFILE base_station_profile

#endif