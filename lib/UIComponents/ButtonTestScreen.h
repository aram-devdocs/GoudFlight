#ifndef BUTTON_TEST_SCREEN_H
#define BUTTON_TEST_SCREEN_H

#include "Screen.h"

class ButtonTestScreen : public Screen {
public:
    static constexpr uint8_t MAX_DISPLAY_BUTTONS = 8;
    static constexpr uint8_t BUTTON_BOX_SIZE = 12;
    static constexpr uint8_t BUTTON_BOX_SPACING = 14;
    static constexpr uint8_t BUTTON_BOX_Y = 20;
    static constexpr uint8_t BUTTON_BOX_X_START = 10;
    
    ButtonTestScreen(DisplayManager* displayManager, ButtonManager* buttonManager);
    
    void draw() override;
    void handleInput() override;
    
    void setDebugSerial(bool enable) { m_debugSerial = enable; }
    void setSerialInterval(uint32_t ms) { m_serialIntervalMs = ms; }
    
private:
    uint32_t m_lastButtonState;
    uint32_t m_lastSerialPrintTime;
    uint32_t m_serialIntervalMs;
    bool m_debugSerial;
    
    void drawTitle();
    void drawButtonBoxes();
    void drawButtonBox(uint8_t buttonIndex, bool pressed);
    void drawButtonNumbers();
    void drawHexValue();
    void printSerialDebug();
};

#endif