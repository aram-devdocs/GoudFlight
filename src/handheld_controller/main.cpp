#include <Arduino.h>
#include <Wire.h>
#include <DisplayManager.h>
#include <ShiftRegisterInput.h>
#include <PinConfig.h>

DisplayManager display;
ShiftRegisterInput buttonInput(PISO_LOAD_PIN, PISO_CLK_PIN, PISO_DATA_PIN);

void setup() {
    Serial.begin(115200);
    Serial.println("Handheld Controller Starting...");
    
    buttonInput.begin();
    
    if (!display.begin()) {
        Serial.println(F("Display initialization failed"));
        for(;;);
    }
    
    display.showStartupScreen();
    delay(2000);
    display.clear();
}

void loop() {
    uint8_t buttonData = buttonInput.read();
    
    display.showButtonStatus(buttonData);
    
    if (buttonInput.hasChanged()) {
        Serial.print("Button state changed: 0x");
        Serial.println(buttonData, HEX);
        
        for (int i = 0; i < 8; i++) {
            if (buttonInput.isButtonPressed(i)) {
                Serial.print("Button ");
                Serial.print(i + 1);
                Serial.println(" is pressed");
            }
        }
    }
    
    delay(50);
}