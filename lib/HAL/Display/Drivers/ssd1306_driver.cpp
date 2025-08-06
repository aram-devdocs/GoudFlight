#include "ssd1306_driver.h"
#include "../../Core/hal_errors.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

typedef struct {
    Adafruit_SSD1306* adafruit_display;
    ssd1306_config_t config;
    bool needs_refresh;
    display_point_t cursor;
} ssd1306_driver_data_t;

static hal_status_t ssd1306_init(display_instance_t* const instance);
static hal_status_t ssd1306_deinit(display_instance_t* const instance);
static hal_status_t ssd1306_clear(display_instance_t* const instance);
static hal_status_t ssd1306_refresh(display_instance_t* const instance);
static hal_status_t ssd1306_set_pixel(display_instance_t* const instance,
                                     const display_point_t* const point,
                                     const uint32_t color);
static hal_status_t ssd1306_draw_rect(display_instance_t* const instance,
                                     const display_rect_t* const rect,
                                     const uint32_t color,
                                     const bool filled);
static hal_status_t ssd1306_draw_line(display_instance_t* const instance,
                                     const display_point_t* const start,
                                     const display_point_t* const end,
                                     const uint32_t color);
static hal_status_t ssd1306_draw_circle(display_instance_t* const instance,
                                       const display_point_t* const center,
                                       const uint16_t radius,
                                       const uint32_t color,
                                       const bool filled);
static hal_status_t ssd1306_set_text_cursor(display_instance_t* const instance,
                                           const display_point_t* const point);
static hal_status_t ssd1306_write_text(display_instance_t* const instance,
                                      const char* const text,
                                      const uint8_t font_size);
static hal_status_t ssd1306_get_info(const display_instance_t* const instance,
                                    display_info_t* const info);
static hal_status_t ssd1306_set_brightness(display_instance_t* const instance,
                                          const uint8_t brightness);
static hal_status_t ssd1306_set_contrast(display_instance_t* const instance,
                                        const uint8_t contrast);

static const display_interface_t ssd1306_interface = {
    .init = ssd1306_init,
    .deinit = ssd1306_deinit,
    .clear = ssd1306_clear,
    .refresh = ssd1306_refresh,
    .set_pixel = ssd1306_set_pixel,
    .draw_rect = ssd1306_draw_rect,
    .draw_line = ssd1306_draw_line,
    .draw_circle = ssd1306_draw_circle,
    .set_text_cursor = ssd1306_set_text_cursor,
    .write_text = ssd1306_write_text,
    .get_info = ssd1306_get_info,
    .set_brightness = ssd1306_set_brightness,
    .set_contrast = ssd1306_set_contrast
};

const hal_resource_constraints_t ssd1306_constraints = {
    .max_stack_bytes = 512,
    .max_heap_bytes = 1024,
    .max_cpu_percent = 20
};

static display_instance_t ssd1306_instance = {
    .interface = &ssd1306_interface,
    .driver_data = nullptr,
    .constraints = &ssd1306_constraints,
    .initialized = false
};

static ssd1306_driver_data_t ssd1306_data;

display_instance_t* ssd1306_create_instance(const void* const config) {
    if (config == nullptr) {
        return nullptr;
    }
    
    const ssd1306_config_t* ssd_config = (const ssd1306_config_t*)config;
    
    ssd1306_data.config = *ssd_config;
    ssd1306_data.adafruit_display = nullptr;
    ssd1306_data.needs_refresh = false;
    ssd1306_data.cursor.x = 0;
    ssd1306_data.cursor.y = 0;
    
    ssd1306_instance.driver_data = &ssd1306_data;
    
    return &ssd1306_instance;
}

void ssd1306_destroy_instance(display_instance_t* instance) {
    if (instance && instance->initialized) {
        ssd1306_deinit(instance);
    }
}

static uint16_t ssd1306_color_to_mono(const uint32_t color) {
    if (color == DISPLAY_COLOR_BLACK) {
        return SSD1306_BLACK;
    } else if (color == DISPLAY_COLOR_INVERT) {
        return SSD1306_INVERSE;
    } else {
        return SSD1306_WHITE;
    }
}

static hal_status_t ssd1306_init(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    
    if (instance->initialized) {
        return HAL_OK;
    }
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    
    Wire.setPins(data->config.sda_pin, data->config.scl_pin);
    
    data->adafruit_display = new Adafruit_SSD1306(
        data->config.width,
        data->config.height,
        &Wire,
        data->config.reset_pin
    );
    
    if (!data->adafruit_display) {
        return HAL_HARDWARE_ERROR;
    }
    
    if (!data->adafruit_display->begin(SSD1306_SWITCHCAPVCC, data->config.i2c_address)) {
        delete data->adafruit_display;
        data->adafruit_display = nullptr;
        return HAL_HARDWARE_ERROR;
    }
    
    data->adafruit_display->clearDisplay();
    data->adafruit_display->display();
    
    instance->initialized = true;
    return HAL_OK;
}

static hal_status_t ssd1306_deinit(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    
    if (data->adafruit_display) {
        data->adafruit_display->clearDisplay();
        data->adafruit_display->display();
        delete data->adafruit_display;
        data->adafruit_display = nullptr;
    }
    
    instance->initialized = false;
    return HAL_OK;
}

static hal_status_t ssd1306_clear(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->clearDisplay();
    data->needs_refresh = true;
    
    return HAL_OK;
}

static hal_status_t ssd1306_refresh(display_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->display();
    data->needs_refresh = false;
    
    return HAL_OK;
}

static hal_status_t ssd1306_set_pixel(display_instance_t* const instance,
                                     const display_point_t* const point,
                                     const uint32_t color) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(point);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->drawPixel(point->x, point->y, ssd1306_color_to_mono(color));
    data->needs_refresh = true;
    
    return HAL_OK;
}

static hal_status_t ssd1306_draw_rect(display_instance_t* const instance,
                                     const display_rect_t* const rect,
                                     const uint32_t color,
                                     const bool filled) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(rect);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    uint16_t mono_color = ssd1306_color_to_mono(color);
    
    if (filled) {
        data->adafruit_display->fillRect(rect->x, rect->y, rect->width, rect->height, mono_color);
    } else {
        data->adafruit_display->drawRect(rect->x, rect->y, rect->width, rect->height, mono_color);
    }
    
    data->needs_refresh = true;
    return HAL_OK;
}

static hal_status_t ssd1306_draw_line(display_instance_t* const instance,
                                     const display_point_t* const start,
                                     const display_point_t* const end,
                                     const uint32_t color) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(start);
    HAL_CHECK_NULL(end);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->drawLine(start->x, start->y, end->x, end->y, ssd1306_color_to_mono(color));
    data->needs_refresh = true;
    
    return HAL_OK;
}

static hal_status_t ssd1306_draw_circle(display_instance_t* const instance,
                                       const display_point_t* const center,
                                       const uint16_t radius,
                                       const uint32_t color,
                                       const bool filled) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(center);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    uint16_t mono_color = ssd1306_color_to_mono(color);
    
    if (filled) {
        data->adafruit_display->fillCircle(center->x, center->y, radius, mono_color);
    } else {
        data->adafruit_display->drawCircle(center->x, center->y, radius, mono_color);
    }
    
    data->needs_refresh = true;
    return HAL_OK;
}

static hal_status_t ssd1306_set_text_cursor(display_instance_t* const instance,
                                           const display_point_t* const point) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(point);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->setCursor(point->x, point->y);
    data->cursor = *point;
    
    return HAL_OK;
}

static hal_status_t ssd1306_write_text(display_instance_t* const instance,
                                      const char* const text,
                                      const uint8_t font_size) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(text);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    
    data->adafruit_display->setTextSize(font_size);
    data->adafruit_display->setTextColor(SSD1306_WHITE);
    data->adafruit_display->print(text);
    data->needs_refresh = true;
    
    return HAL_OK;
}

static hal_status_t ssd1306_get_info(const display_instance_t* const instance,
                                    display_info_t* const info) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(info);
    HAL_CHECK_INITIALIZED(instance);
    
    const ssd1306_driver_data_t* data = (const ssd1306_driver_data_t*)instance->driver_data;
    
    info->width = data->config.width;
    info->height = data->config.height;
    info->color_depth = 1;
    info->capabilities = DISPLAY_CAP_MONOCHROME;
    
    return HAL_OK;
}

static hal_status_t ssd1306_set_brightness(display_instance_t* const instance,
                                          const uint8_t brightness) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->dim(brightness < 128);
    
    return HAL_OK;
}

static hal_status_t ssd1306_set_contrast(display_instance_t* const instance,
                                        const uint8_t contrast) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    ssd1306_driver_data_t* data = (ssd1306_driver_data_t*)instance->driver_data;
    data->adafruit_display->ssd1306_command(SSD1306_SETCONTRAST);
    data->adafruit_display->ssd1306_command(contrast);
    
    return HAL_OK;
}