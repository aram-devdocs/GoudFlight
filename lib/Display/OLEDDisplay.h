#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OLEDDisplay {
public:
    static constexpr uint8_t DEFAULT_WIDTH = 128;
    static constexpr uint8_t DEFAULT_HEIGHT = 64;
    static constexpr uint8_t DEFAULT_ADDRESS = 0x3C;
    static constexpr int8_t DEFAULT_RESET_PIN = -1;
    
    OLEDDisplay(uint8_t width = DEFAULT_WIDTH, 
                uint8_t height = DEFAULT_HEIGHT,
                uint8_t address = DEFAULT_ADDRESS,
                int8_t resetPin = DEFAULT_RESET_PIN);
    
    bool begin();
    void clear();
    void refresh();
    
    void setTextSize(uint8_t size);
    void setTextColor(uint16_t color);
    void setCursor(int16_t x, int16_t y);
    void print(const char* text);
    void print(const __FlashStringHelper* text);
    void print(int value, int base = DEC);
    void println(const char* text);
    void println(const __FlashStringHelper* text);
    
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    
    uint8_t getWidth() const { return m_width; }
    uint8_t getHeight() const { return m_height; }
    bool isInitialized() const { return m_initialized; }
    
    Adafruit_SSD1306* getDisplay() { return &m_display; }
    
private:
    Adafruit_SSD1306 m_display;
    const uint8_t m_width;
    const uint8_t m_height;
    const uint8_t m_address;
    const int8_t m_resetPin;
    bool m_initialized;
};

#endif