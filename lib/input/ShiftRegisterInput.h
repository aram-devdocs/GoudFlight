#ifndef SHIFT_REGISTER_INPUT_H
#define SHIFT_REGISTER_INPUT_H

#include <Arduino.h>
#include "../hardware/PinConfig.h"

class ShiftRegisterInput {
private:
    uint8_t loadPin;
    uint8_t clkPin;
    uint8_t dataPin;
    uint8_t lastState;
    
public:
    ShiftRegisterInput(uint8_t load, uint8_t clk, uint8_t data);
    void begin();
    uint8_t read();
    bool hasChanged();
    bool isButtonPressed(uint8_t buttonIndex);
};

#endif