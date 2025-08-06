#include "shift_register_74hc165.h"
#include "../../Core/hal_errors.h"
#include <Arduino.h>

typedef struct {
    bool current_state;
    bool previous_state;
    bool debounced_state;
    uint32_t last_change_time;
    uint32_t press_start_time;
    uint32_t last_repeat_time;
    uint32_t last_release_time;
    input_event_t pending_events;
    bool long_press_triggered;
} button_state_t;

typedef struct {
    shift_register_74hc165_config_t hw_config;
    input_config_t input_config;
    button_state_t button_states[MAX_INPUT_CHANNELS];
    uint32_t last_raw_value;
    uint8_t total_buttons;
    uint32_t last_update_time;
} shift_register_driver_data_t;

static hal_status_t shift_register_init(input_instance_t* const instance,
                                       const input_config_t* const config);
static hal_status_t shift_register_deinit(input_instance_t* const instance);
static hal_status_t shift_register_update(input_instance_t* const instance);
static hal_status_t shift_register_read_raw(const input_instance_t* const instance,
                                           uint32_t* const raw_state);
static hal_status_t shift_register_read_channel(const input_instance_t* const instance,
                                               const uint8_t channel,
                                               bool* const state);
static hal_status_t shift_register_get_events(input_instance_t* const instance,
                                             input_event_data_t* const events,
                                             const uint8_t max_events,
                                             uint8_t* const num_events);
static hal_status_t shift_register_clear_events(input_instance_t* const instance);
static uint8_t shift_register_get_channel_count(const input_instance_t* const instance);

static const input_interface_t shift_register_interface = {
    .init = shift_register_init,
    .deinit = shift_register_deinit,
    .update = shift_register_update,
    .read_raw = shift_register_read_raw,
    .read_channel = shift_register_read_channel,
    .get_events = shift_register_get_events,
    .clear_events = shift_register_clear_events,
    .get_channel_count = shift_register_get_channel_count
};

const hal_resource_constraints_t shift_register_74hc165_constraints = {
    .max_stack_bytes = 256,
    .max_heap_bytes = 512,
    .max_cpu_percent = 10
};

static input_instance_t shift_register_instance = {
    .interface = &shift_register_interface,
    .driver_data = nullptr,
    .constraints = &shift_register_74hc165_constraints,
    .initialized = false
};

static shift_register_driver_data_t shift_register_data;

input_instance_t* shift_register_74hc165_create_instance(const void* const config) {
    if (config == nullptr) {
        return nullptr;
    }
    
    const shift_register_74hc165_config_t* sr_config = (const shift_register_74hc165_config_t*)config;
    
    if (sr_config->num_chips > MAX_74HC165_CHIPS) {
        return nullptr;
    }
    
    shift_register_data.hw_config = *sr_config;
    shift_register_data.total_buttons = sr_config->num_chips * BITS_PER_74HC165;
    shift_register_data.last_raw_value = 0;
    shift_register_data.last_update_time = 0;
    
    for (uint8_t i = 0; i < MAX_INPUT_CHANNELS; i++) {
        shift_register_data.button_states[i] = {
            false, false, false, 0, 0, 0, 0,
            INPUT_EVENT_NONE, false
        };
    }
    
    shift_register_instance.driver_data = &shift_register_data;
    
    return &shift_register_instance;
}

void shift_register_74hc165_destroy_instance(input_instance_t* instance) {
    if (instance && instance->initialized) {
        shift_register_deinit(instance);
    }
}

static uint32_t read_shift_register_raw(const shift_register_driver_data_t* const data) {
    uint32_t value = 0;
    
    digitalWrite(data->hw_config.load_pin, LOW);
    if (data->hw_config.load_delay_us > 0) {
        delayMicroseconds(data->hw_config.load_delay_us);
    }
    digitalWrite(data->hw_config.load_pin, HIGH);
    
    for (uint8_t i = 0; i < data->total_buttons; i++) {
        value = value << 1;
        if (digitalRead(data->hw_config.data_pin)) {
            value = value | 1;
        }
        
        digitalWrite(data->hw_config.clock_pin, HIGH);
        if (data->hw_config.clock_delay_us > 0) {
            delayMicroseconds(data->hw_config.clock_delay_us);
        }
        digitalWrite(data->hw_config.clock_pin, LOW);
        if (data->hw_config.clock_delay_us > 0) {
            delayMicroseconds(data->hw_config.clock_delay_us);
        }
    }
    
    return value;
}

static void process_button_state(button_state_t* const state,
                                const bool current_raw,
                                const input_config_t* const config,
                                const uint32_t current_time) {
    state->current_state = config->active_low ? !current_raw : current_raw;
    
    if (state->current_state != state->debounced_state) {
        if ((current_time - state->last_change_time) >= config->debounce_ms) {
            state->debounced_state = state->current_state;
            state->last_change_time = current_time;
            
            if (state->debounced_state != state->previous_state) {
                state->previous_state = state->debounced_state;
                
                if (state->debounced_state) {
                    state->pending_events = (input_event_t)(state->pending_events | INPUT_EVENT_PRESSED);
                    state->press_start_time = current_time;
                    state->long_press_triggered = false;
                    state->last_repeat_time = current_time;
                    
                    if (config->double_click_ms > 0 && 
                        (current_time - state->last_release_time) < config->double_click_ms) {
                        state->pending_events = (input_event_t)(state->pending_events | INPUT_EVENT_DOUBLE_CLICK);
                    }
                } else {
                    state->pending_events = (input_event_t)(state->pending_events | INPUT_EVENT_RELEASED);
                    state->last_release_time = current_time;
                    state->press_start_time = 0;
                }
            }
        }
    } else {
        state->last_change_time = current_time;
    }
    
    if (state->debounced_state && state->press_start_time > 0) {
        uint32_t press_duration = current_time - state->press_start_time;
        
        if (!state->long_press_triggered && press_duration >= config->long_press_ms) {
            state->pending_events = (input_event_t)(state->pending_events | INPUT_EVENT_LONG_PRESS);
            state->long_press_triggered = true;
        }
        
        if (press_duration >= config->repeat_delay_ms) {
            if ((current_time - state->last_repeat_time) >= config->repeat_rate_ms) {
                state->pending_events = (input_event_t)(state->pending_events | INPUT_EVENT_REPEAT);
                state->last_repeat_time = current_time;
            }
        }
    }
}

static hal_status_t shift_register_init(input_instance_t* const instance,
                                       const input_config_t* const config) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(instance->driver_data);
    HAL_CHECK_NULL(config);
    
    if (instance->initialized) {
        return HAL_OK;
    }
    
    shift_register_driver_data_t* data = (shift_register_driver_data_t*)instance->driver_data;
    
    data->input_config = *config;
    
    pinMode(data->hw_config.load_pin, OUTPUT);
    pinMode(data->hw_config.clock_pin, OUTPUT);
    pinMode(data->hw_config.data_pin, INPUT);
    
    digitalWrite(data->hw_config.clock_pin, LOW);
    digitalWrite(data->hw_config.load_pin, HIGH);
    
    instance->initialized = true;
    return HAL_OK;
}

static hal_status_t shift_register_deinit(input_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    instance->initialized = false;
    return HAL_OK;
}

static hal_status_t shift_register_update(input_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    shift_register_driver_data_t* data = (shift_register_driver_data_t*)instance->driver_data;
    uint32_t current_time = millis();
    
    uint32_t raw_value = read_shift_register_raw(data);
    data->last_raw_value = raw_value;
    data->last_update_time = current_time;
    
    for (uint8_t i = 0; i < data->total_buttons; i++) {
        bool current_raw = (raw_value >> i) & 1;
        process_button_state(&data->button_states[i], current_raw, 
                           &data->input_config, current_time);
    }
    
    return HAL_OK;
}

static hal_status_t shift_register_read_raw(const input_instance_t* const instance,
                                           uint32_t* const raw_state) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(raw_state);
    HAL_CHECK_INITIALIZED(instance);
    
    const shift_register_driver_data_t* data = (const shift_register_driver_data_t*)instance->driver_data;
    *raw_state = data->last_raw_value;
    
    return HAL_OK;
}

static hal_status_t shift_register_read_channel(const input_instance_t* const instance,
                                               const uint8_t channel,
                                               bool* const state) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(state);
    HAL_CHECK_INITIALIZED(instance);
    
    const shift_register_driver_data_t* data = (const shift_register_driver_data_t*)instance->driver_data;
    
    if (channel >= data->total_buttons) {
        return HAL_INVALID_PARAM;
    }
    
    *state = data->button_states[channel].debounced_state;
    return HAL_OK;
}

static hal_status_t shift_register_get_events(input_instance_t* const instance,
                                             input_event_data_t* const events,
                                             const uint8_t max_events,
                                             uint8_t* const num_events) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_NULL(events);
    HAL_CHECK_NULL(num_events);
    HAL_CHECK_INITIALIZED(instance);
    
    shift_register_driver_data_t* data = (shift_register_driver_data_t*)instance->driver_data;
    *num_events = 0;
    
    for (uint8_t i = 0; i < data->total_buttons && *num_events < max_events; i++) {
        if (data->button_states[i].pending_events != INPUT_EVENT_NONE) {
            events[*num_events].channel = i;
            events[*num_events].event = data->button_states[i].pending_events;
            events[*num_events].timestamp = data->last_update_time;
            events[*num_events].duration = data->button_states[i].press_start_time > 0 ?
                                          data->last_update_time - data->button_states[i].press_start_time : 0;
            (*num_events)++;
            
            data->button_states[i].pending_events = INPUT_EVENT_NONE;
        }
    }
    
    return HAL_OK;
}

static hal_status_t shift_register_clear_events(input_instance_t* const instance) {
    HAL_CHECK_NULL(instance);
    HAL_CHECK_INITIALIZED(instance);
    
    shift_register_driver_data_t* data = (shift_register_driver_data_t*)instance->driver_data;
    
    for (uint8_t i = 0; i < data->total_buttons; i++) {
        data->button_states[i].pending_events = INPUT_EVENT_NONE;
    }
    
    return HAL_OK;
}

static uint8_t shift_register_get_channel_count(const input_instance_t* const instance) {
    if (!instance || !instance->initialized || !instance->driver_data) {
        return 0;
    }
    
    const shift_register_driver_data_t* data = (const shift_register_driver_data_t*)instance->driver_data;
    return data->total_buttons;
}