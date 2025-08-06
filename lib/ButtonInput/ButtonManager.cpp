#include "ButtonManager.h"

ButtonManager::ButtonManager(ShiftRegisterInput* shiftRegister, uint8_t numButtons)
    : m_shiftRegister(shiftRegister),
      m_numButtons(numButtons > MAX_BUTTONS ? MAX_BUTTONS : numButtons),
      m_debounceMs(DEFAULT_DEBOUNCE_MS),
      m_longPressMs(DEFAULT_LONG_PRESS_MS),
      m_repeatDelayMs(DEFAULT_REPEAT_DELAY_MS),
      m_repeatRateMs(DEFAULT_REPEAT_RATE_MS),
      m_inverted(true),
      m_lastReadValue(0) {
    
    for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
        m_buttonStates[i] = {false, false, false, 0, 0, 0, false, EVENT_NONE};
    }
}

void ButtonManager::begin() {
    if (m_shiftRegister) {
        m_shiftRegister->begin();
    }
}

void ButtonManager::update() {
    if (!m_shiftRegister) return;
    
    uint32_t currentTime = millis();
    uint32_t rawValue = m_shiftRegister->readBits(m_numButtons);
    
    if (m_inverted) {
        rawValue = ~rawValue;
    }
    
    m_lastReadValue = rawValue;
    
    for (uint8_t i = 0; i < m_numButtons; i++) {
        bool currentState = (rawValue >> i) & 1;
        updateButton(i, currentState, currentTime);
    }
}

void ButtonManager::updateButton(uint8_t button, bool currentState, uint32_t currentTime) {
    if (button >= m_numButtons) return;
    
    ButtonState& state = m_buttonStates[button];
    state.current = currentState;
    
    processDebounce(state, currentState, currentTime);
    
    if (state.debounced != state.previous) {
        state.previous = state.debounced;
        
        if (state.debounced) {
            state.events |= EVENT_PRESSED;
            state.pressedTime = currentTime;
            state.longPressTriggered = false;
            state.lastRepeatTime = currentTime;
        } else {
            state.events |= EVENT_RELEASED;
            state.pressedTime = 0;
        }
    }
    
    processEvents(state, currentTime);
}

void ButtonManager::processDebounce(ButtonState& state, bool currentState, uint32_t currentTime) {
    if (currentState != state.debounced) {
        if ((currentTime - state.lastChangeTime) >= m_debounceMs) {
            state.debounced = currentState;
            state.lastChangeTime = currentTime;
        }
    } else {
        state.lastChangeTime = currentTime;
    }
}

void ButtonManager::processEvents(ButtonState& state, uint32_t currentTime) {
    if (state.debounced && state.pressedTime > 0) {
        uint32_t pressDuration = currentTime - state.pressedTime;
        
        if (!state.longPressTriggered && pressDuration >= m_longPressMs) {
            state.events |= EVENT_LONG_PRESS;
            state.longPressTriggered = true;
        }
        
        if (pressDuration >= m_repeatDelayMs) {
            uint32_t timeSinceLastRepeat = currentTime - state.lastRepeatTime;
            if (timeSinceLastRepeat >= m_repeatRateMs) {
                state.events |= EVENT_REPEAT;
                state.lastRepeatTime = currentTime;
            }
        }
    }
}

bool ButtonManager::isPressed(uint8_t button) const {
    if (button >= m_numButtons) return false;
    return m_buttonStates[button].debounced;
}

bool ButtonManager::isReleased(uint8_t button) const {
    return !isPressed(button);
}

bool ButtonManager::wasPressed(uint8_t button) {
    if (button >= m_numButtons) return false;
    bool result = (m_buttonStates[button].events & EVENT_PRESSED) != 0;
    m_buttonStates[button].events &= ~EVENT_PRESSED;
    return result;
}

bool ButtonManager::wasReleased(uint8_t button) {
    if (button >= m_numButtons) return false;
    bool result = (m_buttonStates[button].events & EVENT_RELEASED) != 0;
    m_buttonStates[button].events &= ~EVENT_RELEASED;
    return result;
}

bool ButtonManager::wasLongPressed(uint8_t button) {
    if (button >= m_numButtons) return false;
    bool result = (m_buttonStates[button].events & EVENT_LONG_PRESS) != 0;
    m_buttonStates[button].events &= ~EVENT_LONG_PRESS;
    return result;
}

bool ButtonManager::wasRepeated(uint8_t button) {
    if (button >= m_numButtons) return false;
    bool result = (m_buttonStates[button].events & EVENT_REPEAT) != 0;
    m_buttonStates[button].events &= ~EVENT_REPEAT;
    return result;
}

uint8_t ButtonManager::getEvents(uint8_t button) {
    if (button >= m_numButtons) return EVENT_NONE;
    uint8_t events = m_buttonStates[button].events;
    m_buttonStates[button].events = EVENT_NONE;
    return events;
}

void ButtonManager::clearEvents(uint8_t button) {
    if (button >= m_numButtons) return;
    m_buttonStates[button].events = EVENT_NONE;
}

void ButtonManager::clearAllEvents() {
    for (uint8_t i = 0; i < m_numButtons; i++) {
        m_buttonStates[i].events = EVENT_NONE;
    }
}

uint32_t ButtonManager::getPressedMask() const {
    uint32_t mask = 0;
    for (uint8_t i = 0; i < m_numButtons; i++) {
        if (m_buttonStates[i].debounced) {
            mask |= (1UL << i);
        }
    }
    return mask;
}

uint32_t ButtonManager::getChangedMask() const {
    uint32_t mask = 0;
    for (uint8_t i = 0; i < m_numButtons; i++) {
        if (m_buttonStates[i].debounced != m_buttonStates[i].previous) {
            mask |= (1UL << i);
        }
    }
    return mask;
}

const ButtonManager::ButtonState& ButtonManager::getButtonState(uint8_t button) const {
    static const ButtonState emptyState = {false, false, false, 0, 0, 0, false, EVENT_NONE};
    if (button >= m_numButtons) return emptyState;
    return m_buttonStates[button];
}