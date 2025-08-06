#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "OLEDDisplay.h"

class DisplayManager {
public:
    enum TextAlignment {
        ALIGN_LEFT,
        ALIGN_CENTER,
        ALIGN_RIGHT
    };
    
    DisplayManager(OLEDDisplay* display);
    
    bool begin();
    void clear();
    void refresh();
    
    void drawText(const char* text, int16_t x, int16_t y, uint8_t size = 1);
    void drawText(const __FlashStringHelper* text, int16_t x, int16_t y, uint8_t size = 1);
    void drawTextCentered(const char* text, int16_t y, uint8_t size = 1);
    void drawTextCentered(const __FlashStringHelper* text, int16_t y, uint8_t size = 1);
    
    void drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t percentage);
    void drawBattery(int16_t x, int16_t y, uint8_t percentage);
    void drawSignalStrength(int16_t x, int16_t y, uint8_t strength);
    
    void drawTitle(const char* title);
    void drawTitle(const __FlashStringHelper* title);
    void drawStatus(const char* status);
    void drawStatus(const __FlashStringHelper* status);
    
    void drawBox(int16_t x, int16_t y, int16_t w, int16_t h, bool filled = false);
    void drawRoundedBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool filled = false);
    
    OLEDDisplay* getDisplay() { return m_display; }
    const OLEDDisplay* getDisplay() const { return m_display; }
    
private:
    OLEDDisplay* m_display;
    
    int16_t getTextWidth(const char* text, uint8_t size) const;
    int16_t getTextHeight(uint8_t size) const;
};

#endif