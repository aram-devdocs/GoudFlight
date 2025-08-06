#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include "../display_interface.h"

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t i2c_address;
    int8_t reset_pin;
    uint8_t sda_pin;
    uint8_t scl_pin;
} ssd1306_config_t;

display_instance_t* ssd1306_create_instance(const void* const config);
void ssd1306_destroy_instance(display_instance_t* instance);

extern const hal_resource_constraints_t ssd1306_constraints;

#endif