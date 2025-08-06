#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "ShiftRegisterInput.h"

class ButtonManager {
public:
    static constexpr uint8_t MAX_BUTTONS = 32;
    static constexpr uint32_t DEFAULT_DEBOUNCE_MS = 50;
    static constexpr uint32_t DEFAULT_LONG_PRESS_MS = 1000;
    static constexpr uint32_t DEFAULT_REPEAT_DELAY_MS = 500;
    static constexpr uint32_t DEFAULT_REPEAT_RATE_MS = 100;
    
    enum ButtonEvent {
        EVENT_NONE = 0,
        EVENT_PRESSED = 1,
        EVENT_RELEASED = 2,
        EVENT_LONG_PRESS = 4,
        EVENT_REPEAT = 8
    };
    
    struct ButtonState {
        bool current;
        bool previous;
        bool debounced;
        uint32_t lastChangeTime;
        uint32_t pressedTime;
        uint32_t lastRepeatTime;
        bool longPressTriggered;
        uint8_t events;
    };
    
    ButtonManager(ShiftRegisterInput* shiftRegister, uint8_t numButtons = 8);
    
    void begin();
    
    void update();
    
    bool isPressed(uint8_t button) const;
    bool isReleased(uint8_t button) const;
    bool wasPressed(uint8_t button);
    bool wasReleased(uint8_t button);
    bool wasLongPressed(uint8_t button);
    bool wasRepeated(uint8_t button);
    
    uint8_t getEvents(uint8_t button);
    void clearEvents(uint8_t button);
    void clearAllEvents();
    
    uint32_t getPressedMask() const;
    uint32_t getChangedMask() const;
    
    void setDebounceTime(uint32_t ms) { m_debounceMs = ms; }
    void setLongPressTime(uint32_t ms) { m_longPressMs = ms; }
    void setRepeatDelay(uint32_t ms) { m_repeatDelayMs = ms; }
    void setRepeatRate(uint32_t ms) { m_repeatRateMs = ms; }
    
    void setInverted(bool inverted) { m_inverted = inverted; }
    bool isInverted() const { return m_inverted; }
    
    uint8_t getNumButtons() const { return m_numButtons; }
    
    const ButtonState& getButtonState(uint8_t button) const;
    
private:
    ShiftRegisterInput* m_shiftRegister;
    const uint8_t m_numButtons;
    ButtonState m_buttonStates[MAX_BUTTONS];
    
    uint32_t m_debounceMs;
    uint32_t m_longPressMs;
    uint32_t m_repeatDelayMs;
    uint32_t m_repeatRateMs;
    
    bool m_inverted;
    uint32_t m_lastReadValue;
    
    void updateButton(uint8_t button, bool currentState, uint32_t currentTime);
    void processDebounce(ButtonState& state, bool currentState, uint32_t currentTime);
    void processEvents(ButtonState& state, uint32_t currentTime);
};

#endif