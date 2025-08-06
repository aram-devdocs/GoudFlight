#ifndef SHIFT_REGISTER_INPUT_H
#define SHIFT_REGISTER_INPUT_H

#include <Arduino.h>

class ShiftRegisterInput {
public:
    static constexpr uint8_t DEFAULT_NUM_BITS = 8;
    static constexpr uint32_t DEFAULT_CLOCK_DELAY_US = 5;
    static constexpr uint32_t DEFAULT_LOAD_DELAY_US = 5;
    
    ShiftRegisterInput(uint8_t loadPin, uint8_t clockPin, uint8_t dataPin, 
                      uint8_t numBits = DEFAULT_NUM_BITS);
    
    void begin();
    
    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    
    uint32_t readBits(uint8_t numBits);
    
    bool readBit(uint8_t bitIndex);
    
    void setClockDelay(uint32_t delayUs) { m_clockDelayUs = delayUs; }
    void setLoadDelay(uint32_t delayUs) { m_loadDelayUs = delayUs; }
    
    uint8_t getNumBits() const { return m_numBits; }
    
private:
    const uint8_t m_loadPin;
    const uint8_t m_clockPin;
    const uint8_t m_dataPin;
    const uint8_t m_numBits;
    
    uint32_t m_clockDelayUs;
    uint32_t m_loadDelayUs;
    uint32_t m_lastReadValue;
    
    void pulseLoad();
    void pulseClock();
    uint32_t shiftInBits();
};

#endif