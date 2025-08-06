#ifndef SHIFT_REGISTER_74HC165_H
#define SHIFT_REGISTER_74HC165_H

#include "../input_interface.h"

#define MAX_74HC165_CHIPS 4U
#define BITS_PER_74HC165 8U

typedef struct {
    uint8_t load_pin;
    uint8_t clock_pin;
    uint8_t data_pin;
    uint8_t num_chips;
    uint32_t clock_delay_us;
    uint32_t load_delay_us;
} shift_register_74hc165_config_t;

input_instance_t* shift_register_74hc165_create_instance(const void* const config);
void shift_register_74hc165_destroy_instance(input_instance_t* instance);

extern const hal_resource_constraints_t shift_register_74hc165_constraints;

#endif