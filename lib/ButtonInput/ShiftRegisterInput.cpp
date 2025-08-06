#include "ShiftRegisterInput.h"

ShiftRegisterInput::ShiftRegisterInput(uint8_t loadPin, uint8_t clockPin, 
                                       uint8_t dataPin, uint8_t numBits)
    : m_loadPin(loadPin),
      m_clockPin(clockPin),
      m_dataPin(dataPin),
      m_numBits(numBits),
      m_clockDelayUs(DEFAULT_CLOCK_DELAY_US),
      m_loadDelayUs(DEFAULT_LOAD_DELAY_US),
      m_lastReadValue(0) {
}

void ShiftRegisterInput::begin() {
    pinMode(m_loadPin, OUTPUT);
    pinMode(m_clockPin, OUTPUT);
    pinMode(m_dataPin, INPUT);
    
    digitalWrite(m_clockPin, LOW);
    digitalWrite(m_loadPin, HIGH);
}

uint8_t ShiftRegisterInput::read8() {
    if (m_numBits > 8) {
        return static_cast<uint8_t>(readBits(8));
    }
    return static_cast<uint8_t>(readBits(m_numBits));
}

uint16_t ShiftRegisterInput::read16() {
    if (m_numBits > 16) {
        return static_cast<uint16_t>(readBits(16));
    }
    return static_cast<uint16_t>(readBits(m_numBits));
}

uint32_t ShiftRegisterInput::read32() {
    return readBits(m_numBits);
}

uint32_t ShiftRegisterInput::readBits(uint8_t numBits) {
    if (numBits > 32) {
        numBits = 32;
    }
    if (numBits > m_numBits) {
        numBits = m_numBits;
    }
    
    m_lastReadValue = shiftInBits();
    
    if (numBits < 32) {
        uint32_t mask = (1UL << numBits) - 1;
        return m_lastReadValue & mask;
    }
    
    return m_lastReadValue;
}

bool ShiftRegisterInput::readBit(uint8_t bitIndex) {
    if (bitIndex >= m_numBits) {
        return false;
    }
    
    uint32_t value = readBits(m_numBits);
    return (value >> bitIndex) & 1;
}

void ShiftRegisterInput::pulseLoad() {
    digitalWrite(m_loadPin, LOW);
    if (m_loadDelayUs > 0) {
        delayMicroseconds(m_loadDelayUs);
    }
    digitalWrite(m_loadPin, HIGH);
}

void ShiftRegisterInput::pulseClock() {
    digitalWrite(m_clockPin, HIGH);
    if (m_clockDelayUs > 0) {
        delayMicroseconds(m_clockDelayUs);
    }
    digitalWrite(m_clockPin, LOW);
    if (m_clockDelayUs > 0) {
        delayMicroseconds(m_clockDelayUs);
    }
}

uint32_t ShiftRegisterInput::shiftInBits() {
    uint32_t data = 0;
    
    pulseLoad();
    
    for (uint8_t i = 0; i < m_numBits; i++) {
        data = data << 1;
        if (digitalRead(m_dataPin)) {
            data = data | 1;
        }
        pulseClock();
    }
    
    return data;
}