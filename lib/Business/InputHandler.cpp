#include "InputHandler.h"
#include "../Core/Logger.h"

InputHandler::InputHandler(input_instance_t* input)
    : input(input)
    , global_callback(nullptr)
    , debounce_enabled(true)
    , debounce_delay(Constants::Hardware::DEBOUNCE_DELAY_MS)
    , long_press_enabled(true)
    , long_press_time(Constants::Hardware::LONG_PRESS_DURATION_MS)
    , double_click_enabled(true)
    , double_click_window(Constants::Hardware::DOUBLE_CLICK_WINDOW_MS)
    , last_raw_state(0)
    , current_combination(0)
    , combination_start_time(0) {
}

hal_status_t InputHandler::init() {
    if (!input || !input->interface) {
        LOG_ERROR("InputHandler", "Invalid input instance");
        return HAL_INVALID_PARAM;
    }
    
    uint8_t num_channels = input->interface->get_channel_count(input);
    channel_states.resize(num_channels);
    
    for (auto& state : channel_states) {
        memset(&state, 0, sizeof(state));
    }
    
    LOG_INFO("InputHandler", "Input handler initialized with %u channels", num_channels);
    return HAL_OK;
}

hal_status_t InputHandler::update(uint32_t delta_ms) {
    if (!input || !input->interface) {
        return HAL_ERROR;
    }
    
    input->interface->update(input);
    
    uint32_t current_time = millis();
    
    for (uint8_t i = 0; i < channel_states.size(); i++) {
        processChannel(i, current_time);
    }
    
    processCombinations(current_time);
    
    input->interface->read_raw(input, &last_raw_state);
    
    return HAL_OK;
}

void InputHandler::processChannel(uint8_t channel, uint32_t current_time) {
    auto& state = channel_states[channel];
    
    bool raw_state = false;
    input->interface->read_channel(input, channel, &raw_state);
    
    state.previous = state.current;
    state.current = raw_state;
    
    if (debounce_enabled) {
        if (state.current != state.debounced) {
            if (current_time - state.press_time > debounce_delay) {
                state.debounced = state.current;
            } else {
                return;
            }
        }
    } else {
        state.debounced = state.current;
    }
    
    if (state.debounced && !state.previous) {
        state.press_time = current_time;
        state.long_press_fired = false;
        fireEvent(channel, INPUT_EVENT_PRESSED);
        
        if (double_click_enabled) {
            if (current_time - state.last_click_time < double_click_window) {
                fireEvent(channel, INPUT_EVENT_DOUBLE_CLICK);
                state.double_click_pending = false;
                state.last_click_time = 0;
            } else {
                state.double_click_pending = true;
                state.last_click_time = current_time;
            }
        }
    }
    
    if (!state.debounced && state.previous) {
        state.release_time = current_time;
        fireEvent(channel, INPUT_EVENT_RELEASED);
    }
    
    if (state.debounced && long_press_enabled && !state.long_press_fired) {
        if (current_time - state.press_time >= long_press_time) {
            state.long_press_fired = true;
            fireEvent(channel, INPUT_EVENT_LONG_PRESS);
        }
    }
    
    if (double_click_enabled && state.double_click_pending) {
        if (current_time - state.last_click_time >= double_click_window) {
            state.double_click_pending = false;
        }
    }
}

void InputHandler::processCombinations(uint32_t current_time) {
    uint32_t current_mask = 0;
    for (uint8_t i = 0; i < channel_states.size(); i++) {
        if (channel_states[i].debounced) {
            current_mask |= (1 << i);
        }
    }
    
    for (const auto& combo : combinations) {
        if ((current_mask & combo.mask) == combo.mask) {
            if (current_combination != combo.mask) {
                current_combination = combo.mask;
                combination_start_time = current_time;
                LOG_DEBUG("InputHandler", "Combination detected: %s", combo.name);
            } else if (combo.hold_time_ms > 0) {
                if (current_time - combination_start_time >= combo.hold_time_ms) {
                    if (combo.callback) {
                        combo.callback();
                    }
                    current_combination = 0;
                }
            } else {
                if (combo.callback) {
                    combo.callback();
                }
                current_combination = 0;
            }
            return;
        }
    }
    
    current_combination = 0;
}

void InputHandler::fireEvent(uint8_t channel, input_event_t event) {
    for (const auto& cb : callbacks) {
        if (cb.first == channel && cb.second.first == event) {
            if (cb.second.second) {
                cb.second.second(channel, event);
            }
        }
    }
    
    if (global_callback) {
        global_callback(channel, event);
    }
}

void InputHandler::registerCallback(uint8_t channel, input_event_t event, ButtonCallback callback) {
    callbacks.push_back({channel, {event, callback}});
}

void InputHandler::registerCombination(const ButtonCombination& combination) {
    combinations.push_back(combination);
    LOG_DEBUG("InputHandler", "Registered combination: %s (0x%X)", combination.name, combination.mask);
}

void InputHandler::registerGlobalCallback(ButtonCallback callback) {
    global_callback = callback;
}

bool InputHandler::isPressed(uint8_t channel) const {
    if (channel >= channel_states.size()) {
        return false;
    }
    return channel_states[channel].debounced;
}

bool InputHandler::isAnyPressed() const {
    for (const auto& state : channel_states) {
        if (state.debounced) {
            return true;
        }
    }
    return false;
}

uint32_t InputHandler::getRawState() const {
    return last_raw_state;
}

uint8_t InputHandler::getPressedCount() const {
    uint8_t count = 0;
    for (const auto& state : channel_states) {
        if (state.debounced) {
            count++;
        }
    }
    return count;
}

void InputHandler::printState() const {
    LOG_DEBUG("InputHandler", "Button state: 0x%02X, Pressed: %u", 
              getRawState() & 0xFF, getPressedCount());
}

ButtonManager::ButtonManager(InputHandler* handler)
    : handler(handler) {
}

void ButtonManager::mapButton(ButtonId id, uint8_t channel) {
    button_map.push_back({id, channel});
}

void ButtonManager::setButtonAction(ButtonId id, input_event_t event, std::function<void()> action) {
    uint8_t channel = getChannel(id);
    if (channel != 0xFF) {
        handler->registerCallback(channel, event, 
            [action](uint8_t ch, input_event_t ev) { action(); });
    }
}

void ButtonManager::defineCombo(const char* name, std::vector<ButtonId> buttons, std::function<void()> action) {
    InputHandler::ButtonCombination combo;
    combo.name = name;
    combo.mask = createMask(buttons);
    combo.callback = action;
    combo.hold_time_ms = 0;
    handler->registerCombination(combo);
}

bool ButtonManager::isButtonPressed(ButtonId id) const {
    uint8_t channel = getChannel(id);
    if (channel != 0xFF) {
        return handler->isPressed(channel);
    }
    return false;
}

const char* ButtonManager::getButtonName(ButtonId id) const {
    switch (id) {
        case ButtonId::UP:     return "UP";
        case ButtonId::DOWN:   return "DOWN";
        case ButtonId::LEFT:   return "LEFT";
        case ButtonId::RIGHT:  return "RIGHT";
        case ButtonId::SELECT: return "SELECT";
        case ButtonId::BACK:   return "BACK";
        case ButtonId::MENU:   return "MENU";
        case ButtonId::ACTION: return "ACTION";
        default:               return "UNKNOWN";
    }
}

uint8_t ButtonManager::getChannel(ButtonId id) const {
    for (const auto& mapping : button_map) {
        if (mapping.first == id) {
            return mapping.second;
        }
    }
    return 0xFF;
}

uint32_t ButtonManager::createMask(const std::vector<ButtonId>& buttons) const {
    uint32_t mask = 0;
    for (ButtonId id : buttons) {
        uint8_t channel = getChannel(id);
        if (channel != 0xFF) {
            mask |= (1 << channel);
        }
    }
    return mask;
}