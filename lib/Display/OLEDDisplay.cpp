#include "OLEDDisplay.h"

OLEDDisplay::OLEDDisplay(uint8_t width, uint8_t height, uint8_t address, int8_t resetPin)
    : m_display(width, height, &Wire, resetPin),
      m_width(width),
      m_height(height),
      m_address(address),
      m_resetPin(resetPin),
      m_initialized(false) {
}

bool OLEDDisplay::begin() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_display.begin(SSD1306_SWITCHCAPVCC, m_address)) {
        return false;
    }
    
    m_initialized = true;
    clear();
    refresh();
    return true;
}

void OLEDDisplay::clear() {
    if (!m_initialized) return;
    m_display.clearDisplay();
}

void OLEDDisplay::refresh() {
    if (!m_initialized) return;
    m_display.display();
}

void OLEDDisplay::setTextSize(uint8_t size) {
    if (!m_initialized) return;
    m_display.setTextSize(size);
}

void OLEDDisplay::setTextColor(uint16_t color) {
    if (!m_initialized) return;
    m_display.setTextColor(color);
}

void OLEDDisplay::setCursor(int16_t x, int16_t y) {
    if (!m_initialized) return;
    m_display.setCursor(x, y);
}

void OLEDDisplay::print(const char* text) {
    if (!m_initialized) return;
    m_display.print(text);
}

void OLEDDisplay::print(const __FlashStringHelper* text) {
    if (!m_initialized) return;
    m_display.print(text);
}

void OLEDDisplay::print(int value, int base) {
    if (!m_initialized) return;
    m_display.print(value, base);
}

void OLEDDisplay::println(const char* text) {
    if (!m_initialized) return;
    m_display.println(text);
}

void OLEDDisplay::println(const __FlashStringHelper* text) {
    if (!m_initialized) return;
    m_display.println(text);
}

void OLEDDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!m_initialized) return;
    m_display.drawRect(x, y, w, h, color);
}

void OLEDDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!m_initialized) return;
    m_display.fillRect(x, y, w, h, color);
}

void OLEDDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (!m_initialized) return;
    m_display.drawPixel(x, y, color);
}

void OLEDDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (!m_initialized) return;
    m_display.drawLine(x0, y0, x1, y1, color);
}

void OLEDDisplay::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (!m_initialized) return;
    m_display.drawCircle(x0, y0, r, color);
}

void OLEDDisplay::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (!m_initialized) return;
    m_display.fillCircle(x0, y0, r, color);
}