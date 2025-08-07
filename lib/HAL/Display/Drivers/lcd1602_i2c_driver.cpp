#include "lcd1602_i2c_driver.h"
#include "../../Core/hal_errors.h"
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

typedef struct {
    uint8_t i2c_address;
    uint8_t sda_pin;
    uint8_t scl_pin;
    uint8_t columns;
    uint8_t rows;
} lcd1602_config_t;

typedef struct {
    LiquidCrystal_I2C* lcd;
    lcd1602_config_t config;
    uint8_t cursor_x;
    uint8_t cursor_y;
} lcd1602_driver_data_t;

static hal_status_t lcd1602_init(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    if (instance->initialized) {
        return HAL_OK;
    }
    
    Wire.setPins(data->config.sda_pin, data->config.scl_pin);
    Wire.begin();
    Wire.setClock(100000);  // Set I2C clock to 100kHz for better compatibility
    
    data->lcd = new LiquidCrystal_I2C(
        data->config.i2c_address,
        data->config.columns,
        data->config.rows
    );
    
    if (!data->lcd) {
        return HAL_HARDWARE_ERROR;
    }
    
    // Try both init methods for compatibility
    data->lcd->init();
    data->lcd->begin(data->config.columns, data->config.rows);
    delay(100);  // Allow LCD to fully initialize
    
    // Ensure backlight is on
    data->lcd->backlight();
    data->lcd->setBacklight(255);  // Some libraries use this method
    
    // Ensure display is on
    data->lcd->display();
    data->lcd->noCursor();  // Hide cursor
    data->lcd->noBlink();   // No blinking cursor
    
    // Clear and reset
    data->lcd->clear();
    data->lcd->home();
    data->cursor_x = 0;
    data->cursor_y = 0;
    instance->initialized = true;
    
    return HAL_OK;
}

static hal_status_t lcd1602_deinit(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    
    if (!instance->initialized) {
        return HAL_OK;
    }
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    if (data->lcd) {
        data->lcd->noBacklight();
        data->lcd->clear();
        delete data->lcd;
        data->lcd = nullptr;
    }
    
    instance->initialized = false;
    return HAL_OK;
}

static hal_status_t lcd1602_clear(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    HAL_CHECK_INITIALIZED(instance);
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    if (!data->lcd) {
        return HAL_NOT_INITIALIZED;
    }
    
    data->lcd->clear();
    data->cursor_x = 0;
    data->cursor_y = 0;
    
    return HAL_OK;
}

static hal_status_t lcd1602_refresh(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    return HAL_OK;
}

static hal_status_t lcd1602_set_pixel(display_instance_t* const instance,
                                      const display_point_t* const point,
                                      const uint32_t color) {
    (void)instance;
    (void)point;
    (void)color;
    return HAL_NOT_SUPPORTED;
}

static hal_status_t lcd1602_draw_rect(display_instance_t* const instance,
                                      const display_rect_t* const rect,
                                      const uint32_t color,
                                      const bool filled) {
    (void)instance;
    (void)rect;
    (void)color;
    (void)filled;
    return HAL_NOT_SUPPORTED;
}

static hal_status_t lcd1602_draw_line(display_instance_t* const instance,
                                      const display_point_t* const start,
                                      const display_point_t* const end,
                                      const uint32_t color) {
    (void)instance;
    (void)start;
    (void)end;
    (void)color;
    return HAL_NOT_SUPPORTED;
}

static hal_status_t lcd1602_draw_circle(display_instance_t* const instance,
                                        const display_point_t* const center,
                                        const uint16_t radius,
                                        const uint32_t color,
                                        const bool filled) {
    (void)instance;
    (void)center;
    (void)radius;
    (void)color;
    (void)filled;
    return HAL_NOT_SUPPORTED;
}

static hal_status_t lcd1602_set_text_cursor(display_instance_t* const instance,
                                            const display_point_t* const point) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    HAL_CHECK_NULL(point);
    HAL_CHECK_INITIALIZED(instance);
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    if (!data->lcd) {
        return HAL_NOT_INITIALIZED;
    }
    
    if (point->x >= data->config.columns || point->y >= data->config.rows) {
        return HAL_INVALID_PARAM;
    }
    
    data->lcd->setCursor(point->x, point->y);
    data->cursor_x = point->x;
    data->cursor_y = point->y;
    
    return HAL_OK;
}

static hal_status_t lcd1602_write_text(display_instance_t* const instance,
                                       const char* const text,
                                       const uint8_t font_size) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    HAL_CHECK_NULL(text);
    HAL_CHECK_INITIALIZED(instance);
    
    (void)font_size;
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    if (!data->lcd) {
        return HAL_NOT_INITIALIZED;
    }
    
    data->lcd->print(text);
    
    size_t len = strlen(text);
    data->cursor_x += len;
    
    while (data->cursor_x >= data->config.columns) {
        data->cursor_x -= data->config.columns;
        data->cursor_y++;
        if (data->cursor_y >= data->config.rows) {
            data->cursor_y = 0;
        }
    }
    
    return HAL_OK;
}

static hal_status_t lcd1602_get_info(const display_instance_t* const instance,
                                     display_info_t* const info) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    HAL_CHECK_NULL(info);
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    info->width = data->config.columns;
    info->height = data->config.rows;
    info->color_depth = 1;
    info->capabilities = DISPLAY_CAP_MONOCHROME | DISPLAY_CAP_BACKLIGHT;
    
    return HAL_OK;
}

static hal_status_t lcd1602_set_brightness(display_instance_t* const instance,
                                           const uint8_t brightness) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    HAL_CHECK_INITIALIZED(instance);
    
    lcd1602_driver_data_t* data = (lcd1602_driver_data_t*)instance->driver_data;
    
    if (!data->lcd) {
        return HAL_NOT_INITIALIZED;
    }
    
    if (brightness > 0) {
        data->lcd->backlight();
    } else {
        data->lcd->noBacklight();
    }
    
    return HAL_OK;
}

static hal_status_t lcd1602_set_contrast(display_instance_t* const instance,
                                         const uint8_t contrast) {
    (void)instance;
    (void)contrast;
    return HAL_NOT_SUPPORTED;
}

static const display_interface_t g_lcd1602_interface = {
    .init = lcd1602_init,
    .deinit = lcd1602_deinit,
    .clear = lcd1602_clear,
    .refresh = lcd1602_refresh,
    .set_pixel = lcd1602_set_pixel,
    .draw_rect = lcd1602_draw_rect,
    .draw_line = lcd1602_draw_line,
    .draw_circle = lcd1602_draw_circle,
    .set_text_cursor = lcd1602_set_text_cursor,
    .write_text = lcd1602_write_text,
    .get_info = lcd1602_get_info,
    .set_brightness = lcd1602_set_brightness,
    .set_contrast = lcd1602_set_contrast
};

static lcd1602_driver_data_t g_lcd1602_driver_data = {0};
static display_instance_t g_lcd1602_instance = {0};

static const hal_resource_constraints_t g_lcd1602_constraints = {
    .max_stack_bytes = 512,
    .max_heap_bytes = 256,
    .max_cpu_percent = 5
};

display_instance_t* lcd1602_i2c_create_instance(uint8_t i2c_address,
                                                uint8_t sda_pin,
                                                uint8_t scl_pin,
                                                uint8_t columns,
                                                uint8_t rows) {
    g_lcd1602_driver_data.config.i2c_address = i2c_address;
    g_lcd1602_driver_data.config.sda_pin = sda_pin;
    g_lcd1602_driver_data.config.scl_pin = scl_pin;
    g_lcd1602_driver_data.config.columns = columns;
    g_lcd1602_driver_data.config.rows = rows;
    g_lcd1602_driver_data.lcd = nullptr;
    g_lcd1602_driver_data.cursor_x = 0;
    g_lcd1602_driver_data.cursor_y = 0;
    
    g_lcd1602_instance.interface = &g_lcd1602_interface;
    g_lcd1602_instance.driver_data = &g_lcd1602_driver_data;
    g_lcd1602_instance.constraints = &g_lcd1602_constraints;
    g_lcd1602_instance.initialized = false;
    
    return &g_lcd1602_instance;
}