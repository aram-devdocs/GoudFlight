#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../hardware/DisplayConfig.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    
public:
    DisplayManager();
    bool begin();
    void clear();
    void update();
    void showButtonStatus(uint8_t buttonData);
    void showStartupScreen();
    void showError(const char* message);
};

#endif