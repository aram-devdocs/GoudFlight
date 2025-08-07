#include "base_station_bsp.h"
#include "../Display/Drivers/lcd1602_i2c_driver.h"
#include "../Core/hal_errors.h"
#include <Arduino.h>

static display_instance_t* g_display = nullptr;

hal_status_t base_station_bsp_init(const hal_config_header_t* config) {
    HAL_CHECK_NULL(config);
    
    if (config->magic_number != HAL_CONFIG_MAGIC) {
        Serial.println("Error: Invalid configuration magic number");
        return HAL_INVALID_PARAM;
    }
    
    if (config->version != HAL_CONFIG_VERSION) {
        Serial.println("Error: Configuration version mismatch");
        return HAL_INVALID_PARAM;
    }
    
    if (config->board_type != HAL_BOARD_BASE_STATION) {
        Serial.println("Error: Invalid board type for base station BSP");
        return HAL_INVALID_PARAM;
    }
    
    const hardware_profile_t* profile = (const hardware_profile_t*)config;
    
    if (profile->display.driver_type == DISPLAY_DRIVER_LCD1602_I2C) {
        g_display = lcd1602_i2c_create_instance(
            profile->display.params.lcd1602.i2c_address,
            profile->display.params.lcd1602.sda_pin,
            profile->display.params.lcd1602.scl_pin,
            profile->display.params.lcd1602.columns,
            profile->display.params.lcd1602.rows
        );
        
        if (!g_display) {
            Serial.println("Error: Failed to create LCD1602 display instance");
            return HAL_HARDWARE_ERROR;
        }
        
        hal_status_t err = g_display->interface->init(g_display);
        if (err != HAL_OK) {
            Serial.println("Error: Failed to initialize LCD1602 display");
            g_display = nullptr;
            return err;
        }
    } else if (profile->display.driver_type != DISPLAY_DRIVER_NONE) {
        Serial.println("Warning: Unsupported display driver for base station");
    }
    
    Serial.println("Base Station BSP initialized successfully");
    return HAL_OK;
}

hal_status_t base_station_bsp_deinit(void) {
    if (g_display) {
        g_display->interface->deinit(g_display);
        g_display = nullptr;
    }
    
    return HAL_OK;
}

display_instance_t* base_station_bsp_get_display(void) {
    return g_display;
}