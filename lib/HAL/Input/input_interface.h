#ifndef INPUT_INTERFACE_H
#define INPUT_INTERFACE_H

#include "../Core/hal_types.h"

#define MAX_INPUT_CHANNELS 32U

typedef enum {
    INPUT_EVENT_NONE = 0x00,
    INPUT_EVENT_PRESSED = 0x01,
    INPUT_EVENT_RELEASED = 0x02,
    INPUT_EVENT_LONG_PRESS = 0x04,
    INPUT_EVENT_REPEAT = 0x08,
    INPUT_EVENT_DOUBLE_CLICK = 0x10
} input_event_t;

typedef struct {
    uint8_t channel;
    input_event_t event;
    uint32_t timestamp;
    uint32_t duration;
} input_event_data_t;

typedef struct {
    uint32_t debounce_ms;
    uint32_t long_press_ms;
    uint32_t repeat_delay_ms;
    uint32_t repeat_rate_ms;
    uint32_t double_click_ms;
    bool active_low;
} input_config_t;

typedef struct input_instance_s input_instance_t;

typedef struct {
    hal_status_t (*init)(input_instance_t* const instance, 
                        const input_config_t* const config);
    hal_status_t (*deinit)(input_instance_t* const instance);
    hal_status_t (*update)(input_instance_t* const instance);
    hal_status_t (*read_raw)(const input_instance_t* const instance, 
                            uint32_t* const raw_state);
    hal_status_t (*read_channel)(const input_instance_t* const instance,
                                const uint8_t channel,
                                bool* const state);
    hal_status_t (*get_events)(input_instance_t* const instance, 
                              input_event_data_t* const events, 
                              const uint8_t max_events, 
                              uint8_t* const num_events);
    hal_status_t (*clear_events)(input_instance_t* const instance);
    uint8_t (*get_channel_count)(const input_instance_t* const instance);
} input_interface_t;

struct input_instance_s {
    const input_interface_t* interface;
    void* driver_data;
    const hal_resource_constraints_t* constraints;
    bool initialized;
};

typedef input_instance_t* (*input_factory_func_t)(const void* const config);

#endif