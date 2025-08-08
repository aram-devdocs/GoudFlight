#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "../HAL/Core/hal_types.h"
#include "../HAL/Input/input_interface.h"
#include "../Core/Constants.h"
#include <functional>
#include <vector>

class InputHandler {
public:
    using ButtonCallback = std::function<void(uint8_t channel, input_event_t event)>;
    using CombinationCallback = std::function<void()>;
    
    struct ButtonCombination {
        uint32_t mask;
        const char* name;
        CombinationCallback callback;
        uint32_t hold_time_ms;
    };
    
    InputHandler(input_instance_t* input);
    ~InputHandler() = default;
    
    hal_status_t init();
    hal_status_t update(uint32_t delta_ms);
    
    void registerCallback(uint8_t channel, input_event_t event, ButtonCallback callback);
    void registerCombination(const ButtonCombination& combination);
    void registerGlobalCallback(ButtonCallback callback);
    
    void enableDebounce(bool enable) { debounce_enabled = enable; }
    void setDebounceDelay(uint32_t delay_ms) { debounce_delay = delay_ms; }
    
    void enableLongPress(bool enable) { long_press_enabled = enable; }
    void setLongPressTime(uint32_t time_ms) { long_press_time = time_ms; }
    
    void enableDoubleClick(bool enable) { double_click_enabled = enable; }
    void setDoubleClickWindow(uint32_t window_ms) { double_click_window = window_ms; }
    
    bool isPressed(uint8_t channel) const;
    bool isAnyPressed() const;
    uint32_t getRawState() const;
    uint8_t getPressedCount() const;
    
    void printState() const;
    
private:
    struct ChannelState {
        bool current;
        bool previous;
        bool debounced;
        uint32_t press_time;
        uint32_t release_time;
        uint32_t last_click_time;
        bool long_press_fired;
        bool double_click_pending;
    };
    
    input_instance_t* input;
    std::vector<ChannelState> channel_states;
    std::vector<std::pair<uint8_t, std::pair<input_event_t, ButtonCallback>>> callbacks;
    std::vector<ButtonCombination> combinations;
    ButtonCallback global_callback;
    
    bool debounce_enabled;
    uint32_t debounce_delay;
    bool long_press_enabled;
    uint32_t long_press_time;
    bool double_click_enabled;
    uint32_t double_click_window;
    
    uint32_t last_raw_state;
    uint32_t current_combination;
    uint32_t combination_start_time;
    
    void processChannel(uint8_t channel, uint32_t current_time);
    void processCombinations(uint32_t current_time);
    void fireEvent(uint8_t channel, input_event_t event);
};

class ButtonManager {
public:
    enum class ButtonId : uint8_t {
        UP = 0,
        DOWN = 1,
        LEFT = 2,
        RIGHT = 3,
        SELECT = 4,
        BACK = 5,
        MENU = 6,
        ACTION = 7
    };
    
    ButtonManager(InputHandler* handler);
    ~ButtonManager() = default;
    
    void mapButton(ButtonId id, uint8_t channel);
    void setButtonAction(ButtonId id, input_event_t event, std::function<void()> action);
    
    void defineCombo(const char* name, std::vector<ButtonId> buttons, std::function<void()> action);
    
    bool isButtonPressed(ButtonId id) const;
    const char* getButtonName(ButtonId id) const;
    
private:
    InputHandler* handler;
    std::vector<std::pair<ButtonId, uint8_t>> button_map;
    
    uint8_t getChannel(ButtonId id) const;
    uint32_t createMask(const std::vector<ButtonId>& buttons) const;
};

#endif