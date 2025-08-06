#ifndef HANDHELD_CONFIG_H
#define HANDHELD_CONFIG_H

#include "hardware_profiles.h"

static const hardware_profile_t handheld_v1_profile = {
    .header = {
        .magic_number = HAL_CONFIG_MAGIC,
        .version = HAL_CONFIG_VERSION,
        .board_type = HAL_BOARD_HANDHELD
    },
    .profile_id = HW_PROFILE_HANDHELD_V1,
    .board_name = "Handheld Controller V1",
    .display = {
        .driver_type = DISPLAY_DRIVER_SSD1306,
        .params = {
            .ssd1306 = {
                .width = 128,
                .height = 64,
                .i2c_address = 0x3C,
                .reset_pin = -1,
                .sda_pin = 21,
                .scl_pin = 22
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

static const hardware_profile_t handheld_v2_profile = {
    .header = {
        .magic_number = HAL_CONFIG_MAGIC,
        .version = HAL_CONFIG_VERSION,
        .board_type = HAL_BOARD_HANDHELD
    },
    .profile_id = HW_PROFILE_HANDHELD_V2,
    .board_name = "Handheld Controller V2",
    .display = {
        .driver_type = DISPLAY_DRIVER_SSD1306,
        .params = {
            .ssd1306 = {
                .width = 128,
                .height = 64,
                .i2c_address = 0x3D,
                .reset_pin = 4,
                .sda_pin = 21,
                .scl_pin = 22
            }
        }
    },
    .input = {
        .driver_type = INPUT_DRIVER_SHIFT_REGISTER,
        .params = {
            .shift_register = {
                .load_pin = 32,
                .clock_pin = 33,
                .data_pin = 34,
                .num_registers = 2,
                .clock_delay_us = 3,
                .load_delay_us = 3
            }
        }
    },
    .resource_limits = {
        .max_stack_bytes = 4096,
        .max_heap_bytes = 16384,
        .max_cpu_percent = 75
    }
};

#if defined(HARDWARE_VERSION_V2)
    #define HANDHELD_HARDWARE_PROFILE handheld_v2_profile
#else
    #define HANDHELD_HARDWARE_PROFILE handheld_v1_profile
#endif

#endif