#include <Arduino.h>
#include <OLEDDisplay.h>
#include <DisplayManager.h>
#include <ShiftRegisterInput.h>
#include <ButtonManager.h>
#include <ButtonTestScreen.h>

#define PISO_LOAD_PIN 25
#define PISO_CLK_PIN 26
#define PISO_DATA_PIN 27

OLEDDisplay oledDisplay;
DisplayManager displayManager(&oledDisplay);
ShiftRegisterInput shiftRegister(PISO_LOAD_PIN, PISO_CLK_PIN, PISO_DATA_PIN);
ButtonManager buttonManager(&shiftRegister);
ButtonTestScreen buttonTestScreen(&displayManager, &buttonManager);

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting Handheld Controller..."));
    
    buttonManager.begin();
    
    if (!displayManager.begin()) {
        Serial.println(F("Display initialization failed"));
        for(;;);
    }
    
    Serial.println(F("Display initialized!"));
    
    displayManager.clear();
    displayManager.drawTextCentered(F("Hello"), 10, 2);
    displayManager.drawTextCentered(F("World!"), 30, 2);
    displayManager.refresh();
    
    delay(2000);
    
    buttonTestScreen.onEnter();
    
    Serial.println(F("Setup complete!"));
}

void loop() {
    buttonTestScreen.update();
    delay(50);
}