#ifndef HARDWARE_PROFILES_H
#define HARDWARE_PROFILES_H

#include "../HAL/Core/hal_types.h"

typedef enum {
    HW_PROFILE_HANDHELD_V1 = 0x1001,
    HW_PROFILE_HANDHELD_V2 = 0x1002,
    HW_PROFILE_DRONE_V1 = 0x2001,
    HW_PROFILE_BASE_V1 = 0x3001
} hardware_profile_id_t;

typedef enum {
    DISPLAY_DRIVER_SSD1306 = 0,
    DISPLAY_DRIVER_ST7735 = 1,
    DISPLAY_DRIVER_ILI9341 = 2,
    DISPLAY_DRIVER_NONE = 0xFF
} display_driver_type_t;

typedef enum {
    INPUT_DRIVER_SHIFT_REGISTER = 0,
    INPUT_DRIVER_GPIO_MATRIX = 1,
    INPUT_DRIVER_I2C_EXPANDER = 2,
    INPUT_DRIVER_NONE = 0xFF
} input_driver_type_t;

typedef struct {
    display_driver_type_t driver_type;
    union {
        struct {
            uint8_t width;
            uint8_t height;
            uint8_t i2c_address;
            int8_t reset_pin;
            uint8_t sda_pin;
            uint8_t scl_pin;
        } ssd1306;
        struct {
            uint8_t width;
            uint8_t height;
            uint8_t cs_pin;
            uint8_t dc_pin;
            uint8_t rst_pin;
            uint8_t bl_pin;
        } st7735;
    } params;
} display_hw_config_t;

typedef struct {
    input_driver_type_t driver_type;
    union {
        struct {
            uint8_t load_pin;
            uint8_t clock_pin;
            uint8_t data_pin;
            uint8_t num_registers;
            uint32_t clock_delay_us;
            uint32_t load_delay_us;
        } shift_register;
        struct {
            uint8_t rows;
            uint8_t cols;
            const uint8_t* row_pins;
            const uint8_t* col_pins;
        } gpio_matrix;
    } params;
} input_hw_config_t;

typedef struct {
    hal_config_header_t header;
    hardware_profile_id_t profile_id;
    const char* board_name;
    display_hw_config_t display;
    input_hw_config_t input;
    hal_resource_constraints_t resource_limits;
} hardware_profile_t;

#endif