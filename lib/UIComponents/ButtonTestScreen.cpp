#include "ButtonTestScreen.h"

ButtonTestScreen::ButtonTestScreen(DisplayManager* displayManager, ButtonManager* buttonManager)
    : Screen(displayManager, buttonManager),
      m_lastButtonState(0),
      m_lastSerialPrintTime(0),
      m_serialIntervalMs(1000),
      m_debugSerial(true) {
}

void ButtonTestScreen::draw() {
    if (!m_displayManager) return;
    
    m_displayManager->clear();
    
    drawTitle();
    drawButtonBoxes();
    drawButtonNumbers();
    drawHexValue();
    
    m_displayManager->refresh();
}

void ButtonTestScreen::handleInput() {
    if (!m_buttonManager) return;
    
    Screen::handleInput();
    
    uint32_t currentButtonState = m_buttonManager->getPressedMask();
    
    if (currentButtonState != m_lastButtonState) {
        m_lastButtonState = currentButtonState;
        requestRedraw();
    }
    
    if (m_debugSerial) {
        printSerialDebug();
    }
}

void ButtonTestScreen::drawTitle() {
    m_displayManager->drawText(F("Button Test"), 30, 0, 1);
}

void ButtonTestScreen::drawButtonBoxes() {
    if (!m_buttonManager) return;
    
    uint8_t numButtons = m_buttonManager->getNumButtons();
    if (numButtons > MAX_DISPLAY_BUTTONS) {
        numButtons = MAX_DISPLAY_BUTTONS;
    }
    
    for (uint8_t i = 0; i < numButtons; i++) {
        bool pressed = m_buttonManager->isPressed(i);
        drawButtonBox(i, pressed);
    }
}

void ButtonTestScreen::drawButtonBox(uint8_t buttonIndex, bool pressed) {
    int16_t x = BUTTON_BOX_X_START + (buttonIndex * BUTTON_BOX_SPACING);
    int16_t y = BUTTON_BOX_Y;
    
    m_displayManager->drawBox(x, y, BUTTON_BOX_SIZE, BUTTON_BOX_SIZE, pressed);
}

void ButtonTestScreen::drawButtonNumbers() {
    if (!m_buttonManager) return;
    
    uint8_t numButtons = m_buttonManager->getNumButtons();
    if (numButtons > MAX_DISPLAY_BUTTONS) {
        numButtons = MAX_DISPLAY_BUTTONS;
    }
    
    for (uint8_t i = 0; i < numButtons; i++) {
        int16_t x = BUTTON_BOX_X_START + (i * BUTTON_BOX_SPACING) + 3;
        int16_t y = BUTTON_BOX_Y + BUTTON_BOX_SIZE + 4;
        
        m_displayManager->getDisplay()->setCursor(x, y);
        m_displayManager->getDisplay()->setTextSize(1);
        m_displayManager->getDisplay()->setTextColor(SSD1306_WHITE);
        m_displayManager->getDisplay()->print(i + 1);
    }
}

void ButtonTestScreen::drawHexValue() {
    if (!m_buttonManager) return;
    
    uint32_t buttonMask = m_buttonManager->getPressedMask();
    uint8_t buttonData = ~static_cast<uint8_t>(buttonMask & 0xFF);
    
    m_displayManager->getDisplay()->setCursor(5, 50);
    m_displayManager->getDisplay()->setTextSize(1);
    m_displayManager->getDisplay()->setTextColor(SSD1306_WHITE);
    m_displayManager->getDisplay()->print(F("Data: 0x"));
    
    if (buttonData < 16) {
        m_displayManager->getDisplay()->print(F("0"));
    }
    m_displayManager->getDisplay()->print(buttonData, HEX);
}

void ButtonTestScreen::printSerialDebug() {
    uint32_t currentTime = millis();
    
    if (currentTime - m_lastSerialPrintTime >= m_serialIntervalMs) {
        uint32_t buttonMask = m_buttonManager->getPressedMask();
        uint8_t buttonData = ~static_cast<uint8_t>(buttonMask & 0xFF);
        
        Serial.print(F("Buttons: 0x"));
        Serial.println(buttonData, HEX);
        
        m_lastSerialPrintTime = currentTime;
    }
}