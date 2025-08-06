#include "handheld_bsp.h"
#include "../../Config/handheld_config.h"
#include "../Display/Drivers/ssd1306_driver.h"
#include "../Input/Drivers/shift_register_74hc165.h"
#include "../Core/hal_errors.h"
#include <Arduino.h>

static char last_error_message[128] = {0};

static hal_status_t create_display_instance(handheld_hardware_t* const hardware,
                                           const hardware_profile_t* const profile) {
    if (profile->display.driver_type == DISPLAY_DRIVER_SSD1306) {
        ssd1306_config_t config = {
            .width = profile->display.params.ssd1306.width,
            .height = profile->display.params.ssd1306.height,
            .i2c_address = profile->display.params.ssd1306.i2c_address,
            .reset_pin = profile->display.params.ssd1306.reset_pin,
            .sda_pin = profile->display.params.ssd1306.sda_pin,
            .scl_pin = profile->display.params.ssd1306.scl_pin
        };
        
        hardware->display = ssd1306_create_instance(&config);
        if (!hardware->display) {
            strcpy(last_error_message, "Failed to create SSD1306 display instance");
            return HAL_HARDWARE_ERROR;
        }
    } else {
        strcpy(last_error_message, "Unsupported display driver type");
        return HAL_NOT_SUPPORTED;
    }
    
    return HAL_OK;
}

static hal_status_t create_input_instance(handheld_hardware_t* const hardware,
                                         const hardware_profile_t* const profile) {
    if (profile->input.driver_type == INPUT_DRIVER_SHIFT_REGISTER) {
        shift_register_74hc165_config_t config = {
            .load_pin = profile->input.params.shift_register.load_pin,
            .clock_pin = profile->input.params.shift_register.clock_pin,
            .data_pin = profile->input.params.shift_register.data_pin,
            .num_chips = profile->input.params.shift_register.num_registers,
            .clock_delay_us = profile->input.params.shift_register.clock_delay_us,
            .load_delay_us = profile->input.params.shift_register.load_delay_us
        };
        
        hardware->input = shift_register_74hc165_create_instance(&config);
        if (!hardware->input) {
            strcpy(last_error_message, "Failed to create 74HC165 input instance");
            return HAL_HARDWARE_ERROR;
        }
    } else {
        strcpy(last_error_message, "Unsupported input driver type");
        return HAL_NOT_SUPPORTED;
    }
    
    return HAL_OK;
}

hal_status_t handheld_bsp_init(handheld_hardware_t* const hardware) {
    HAL_CHECK_NULL(hardware);
    
    const hardware_profile_t* profile = &HANDHELD_HARDWARE_PROFILE;
    
    if (profile->header.magic_number != HAL_CONFIG_MAGIC) {
        strcpy(last_error_message, "Invalid hardware profile magic number");
        return HAL_INVALID_PARAM;
    }
    
    if (profile->header.board_type != HAL_BOARD_HANDHELD) {
        strcpy(last_error_message, "Hardware profile not for handheld board");
        return HAL_INVALID_PARAM;
    }
    
    hal_status_t status;
    
    status = create_display_instance(hardware, profile);
    if (status != HAL_OK) {
        return status;
    }
    
    status = create_input_instance(hardware, profile);
    if (status != HAL_OK) {
        if (hardware->display) {
            ssd1306_destroy_instance(hardware->display);
            hardware->display = nullptr;
        }
        return status;
    }
    
    if (hardware->display && hardware->display->interface) {
        status = hardware->display->interface->init(hardware->display);
        if (status != HAL_OK) {
            strcpy(last_error_message, "Failed to initialize display");
            handheld_bsp_deinit(hardware);
            return status;
        }
    }
    
    if (hardware->input && hardware->input->interface) {
        input_config_t input_config = {
            .debounce_ms = 50,
            .long_press_ms = 1000,
            .repeat_delay_ms = 500,
            .repeat_rate_ms = 100,
            .double_click_ms = 300,
            .active_low = true
        };
        
        status = hardware->input->interface->init(hardware->input, &input_config);
        if (status != HAL_OK) {
            strcpy(last_error_message, "Failed to initialize input");
            handheld_bsp_deinit(hardware);
            return status;
        }
    }
    
    Serial.print(F("BSP: Initialized "));
    Serial.println(profile->board_name);
    Serial.print(F("BSP: Hardware profile ID: 0x"));
    Serial.println(profile->profile_id, HEX);
    
    return HAL_OK;
}

hal_status_t handheld_bsp_deinit(handheld_hardware_t* const hardware) {
    HAL_CHECK_NULL(hardware);
    
    if (hardware->display && hardware->display->interface && hardware->display->initialized) {
        hardware->display->interface->deinit(hardware->display);
    }
    
    if (hardware->input && hardware->input->interface && hardware->input->initialized) {
        hardware->input->interface->deinit(hardware->input);
    }
    
    if (hardware->display) {
        ssd1306_destroy_instance(hardware->display);
        hardware->display = nullptr;
    }
    
    if (hardware->input) {
        shift_register_74hc165_destroy_instance(hardware->input);
        hardware->input = nullptr;
    }
    
    return HAL_OK;
}

hal_status_t handheld_bsp_reset(handheld_hardware_t* const hardware) {
    HAL_CHECK_NULL(hardware);
    
    hal_status_t status = handheld_bsp_deinit(hardware);
    if (status != HAL_OK) {
        return status;
    }
    
    delay(100);
    
    return handheld_bsp_init(hardware);
}

const display_instance_t* handheld_get_display(const handheld_hardware_t* const hw) {
    if (!hw) {
        return nullptr;
    }
    return hw->display;
}

const input_instance_t* handheld_get_input(const handheld_hardware_t* const hw) {
    if (!hw) {
        return nullptr;
    }
    return hw->input;
}

hal_status_t handheld_get_last_error(char* const error_msg, const uint8_t max_len) {
    HAL_CHECK_NULL(error_msg);
    
    strncpy(error_msg, last_error_message, max_len - 1);
    error_msg[max_len - 1] = '\0';
    
    return HAL_OK;
}

void handheld_log_system_info(void) {
    const hardware_profile_t* profile = &HANDHELD_HARDWARE_PROFILE;
    
    Serial.println(F("=== Handheld BSP System Info ==="));
    Serial.print(F("Board: "));
    Serial.println(profile->board_name);
    Serial.print(F("Profile ID: 0x"));
    Serial.println(profile->profile_id, HEX);
    Serial.print(F("Version: 0x"));
    Serial.println(profile->header.version, HEX);
    
    Serial.println(F("Display:"));
    Serial.print(F("  Driver: "));
    Serial.println(profile->display.driver_type == DISPLAY_DRIVER_SSD1306 ? "SSD1306" : "Unknown");
    Serial.print(F("  Resolution: "));
    Serial.print(profile->display.params.ssd1306.width);
    Serial.print(F("x"));
    Serial.println(profile->display.params.ssd1306.height);
    
    Serial.println(F("Input:"));
    Serial.print(F("  Driver: "));
    Serial.println(profile->input.driver_type == INPUT_DRIVER_SHIFT_REGISTER ? "74HC165" : "Unknown");
    Serial.print(F("  Channels: "));
    Serial.println(profile->input.params.shift_register.num_registers * 8);
    
    Serial.println(F("Resource Limits:"));
    Serial.print(F("  Max Stack: "));
    Serial.print(profile->resource_limits.max_stack_bytes);
    Serial.println(F(" bytes"));
    Serial.print(F("  Max Heap: "));
    Serial.print(profile->resource_limits.max_heap_bytes);
    Serial.println(F(" bytes"));
    Serial.print(F("  Max CPU: "));
    Serial.print(profile->resource_limits.max_cpu_percent);
    Serial.println(F("%"));
    
    Serial.println(F("========================="));
}