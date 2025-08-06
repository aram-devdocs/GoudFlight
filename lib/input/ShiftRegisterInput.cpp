#include "ShiftRegisterInput.h"

ShiftRegisterInput::ShiftRegisterInput(uint8_t load, uint8_t clk, uint8_t data) 
    : loadPin(load), clkPin(clk), dataPin(data), lastState(0) {
}

void ShiftRegisterInput::begin() {
    pinMode(loadPin, OUTPUT);
    pinMode(clkPin, OUTPUT);
    pinMode(dataPin, INPUT);
    
    digitalWrite(clkPin, LOW);
    digitalWrite(loadPin, HIGH);
}

uint8_t ShiftRegisterInput::read() {
    uint8_t data = 0;
    
    digitalWrite(loadPin, LOW);
    delayMicroseconds(5);
    digitalWrite(loadPin, HIGH);
    
    for (int i = 0; i < 8; i++) {
        data = data << 1;
        if (digitalRead(dataPin)) {
            data = data | 1;
        }
        digitalWrite(clkPin, HIGH);
        delayMicroseconds(5);
        digitalWrite(clkPin, LOW);
        delayMicroseconds(5);
    }
    
    lastState = data;
    return data;
}

bool ShiftRegisterInput::hasChanged() {
    uint8_t currentState = read();
    return currentState != lastState;
}

bool ShiftRegisterInput::isButtonPressed(uint8_t buttonIndex) {
    if (buttonIndex >= 8) return false;
    return (lastState >> buttonIndex) & 1;
}