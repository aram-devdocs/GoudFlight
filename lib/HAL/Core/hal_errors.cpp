#include "hal_errors.h"
#include <Arduino.h>

static hal_error_info_t last_error = {HAL_OK, nullptr, nullptr, 0, 0};

void hal_log_error(hal_status_t status, const char* file, uint32_t line) {
    last_error.code = status;
    last_error.file = file;
    last_error.line = line;
    last_error.timestamp = millis();
    last_error.message = hal_get_error_string(status);
    
    #ifdef DEBUG
    Serial.print(F("HAL Error: "));
    Serial.print(last_error.message);
    Serial.print(F(" at "));
    Serial.print(file);
    Serial.print(F(":"));
    Serial.println(line);
    #endif
}

const char* hal_get_error_string(hal_status_t status) {
    switch(status) {
        case HAL_OK:
            return "OK";
        case HAL_ERROR:
            return "Generic error";
        case HAL_BUSY:
            return "Resource busy";
        case HAL_TIMEOUT:
            return "Operation timeout";
        case HAL_INVALID_PARAM:
            return "Invalid parameter";
        case HAL_NOT_SUPPORTED:
            return "Not supported";
        case HAL_HARDWARE_ERROR:
            return "Hardware error";
        case HAL_NOT_INITIALIZED:
            return "Not initialized";
        default:
            return "Unknown error";
    }
}

hal_status_t hal_get_last_error(hal_error_info_t* error_info) {
    if (!error_info) {
        return HAL_INVALID_PARAM;
    }
    
    *error_info = last_error;
    return HAL_OK;
}

void hal_clear_errors(void) {
    last_error.code = HAL_OK;
    last_error.message = nullptr;
    last_error.file = nullptr;
    last_error.line = 0;
    last_error.timestamp = 0;
}