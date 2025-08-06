#include "DisplayManager.h"

DisplayManager::DisplayManager(OLEDDisplay* display) 
    : m_display(display) {
}

bool DisplayManager::begin() {
    if (!m_display) {
        return false;
    }
    return m_display->begin();
}

void DisplayManager::clear() {
    if (m_display) {
        m_display->clear();
    }
}

void DisplayManager::refresh() {
    if (m_display) {
        m_display->refresh();
    }
}

void DisplayManager::drawText(const char* text, int16_t x, int16_t y, uint8_t size) {
    if (!m_display || !text) return;
    
    m_display->setTextSize(size);
    m_display->setTextColor(SSD1306_WHITE);
    m_display->setCursor(x, y);
    m_display->print(text);
}

void DisplayManager::drawText(const __FlashStringHelper* text, int16_t x, int16_t y, uint8_t size) {
    if (!m_display || !text) return;
    
    m_display->setTextSize(size);
    m_display->setTextColor(SSD1306_WHITE);
    m_display->setCursor(x, y);
    m_display->print(text);
}

void DisplayManager::drawTextCentered(const char* text, int16_t y, uint8_t size) {
    if (!m_display || !text) return;
    
    int16_t textWidth = getTextWidth(text, size);
    int16_t x = (m_display->getWidth() - textWidth) / 2;
    drawText(text, x, y, size);
}

void DisplayManager::drawTextCentered(const __FlashStringHelper* text, int16_t y, uint8_t size) {
    if (!m_display || !text) return;
    
    char buffer[64];
    strcpy_P(buffer, (const char*)text);
    drawTextCentered(buffer, y, size);
}

void DisplayManager::drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t percentage) {
    if (!m_display) return;
    
    if (percentage > 100) percentage = 100;
    
    m_display->drawRect(x, y, width, height, SSD1306_WHITE);
    
    int16_t fillWidth = (width - 2) * percentage / 100;
    if (fillWidth > 0) {
        m_display->fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void DisplayManager::drawBattery(int16_t x, int16_t y, uint8_t percentage) {
    if (!m_display) return;
    
    const int16_t width = 20;
    const int16_t height = 10;
    const int16_t tipWidth = 2;
    
    m_display->drawRect(x, y, width, height, SSD1306_WHITE);
    m_display->fillRect(x + width, y + 3, tipWidth, 4, SSD1306_WHITE);
    
    if (percentage > 100) percentage = 100;
    int16_t fillWidth = (width - 2) * percentage / 100;
    if (fillWidth > 0) {
        m_display->fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void DisplayManager::drawSignalStrength(int16_t x, int16_t y, uint8_t strength) {
    if (!m_display) return;
    
    const int16_t maxBars = 4;
    const int16_t barWidth = 3;
    const int16_t barSpacing = 2;
    const int16_t maxHeight = 12;
    
    if (strength > 100) strength = 100;
    int16_t bars = (strength * maxBars) / 100;
    
    for (int i = 0; i < maxBars; i++) {
        int16_t barHeight = ((i + 1) * maxHeight) / maxBars;
        int16_t barX = x + i * (barWidth + barSpacing);
        int16_t barY = y + maxHeight - barHeight;
        
        if (i < bars) {
            m_display->fillRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
        } else {
            m_display->drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
        }
    }
}

void DisplayManager::drawTitle(const char* title) {
    if (!m_display || !title) return;
    
    drawTextCentered(title, 0, 1);
    m_display->drawLine(0, 10, m_display->getWidth() - 1, 10, SSD1306_WHITE);
}

void DisplayManager::drawTitle(const __FlashStringHelper* title) {
    if (!m_display || !title) return;
    
    drawTextCentered(title, 0, 1);
    m_display->drawLine(0, 10, m_display->getWidth() - 1, 10, SSD1306_WHITE);
}

void DisplayManager::drawStatus(const char* status) {
    if (!m_display || !status) return;
    
    int16_t y = m_display->getHeight() - 8;
    m_display->drawLine(0, y - 2, m_display->getWidth() - 1, y - 2, SSD1306_WHITE);
    drawText(status, 0, y, 1);
}

void DisplayManager::drawStatus(const __FlashStringHelper* status) {
    if (!m_display || !status) return;
    
    int16_t y = m_display->getHeight() - 8;
    m_display->drawLine(0, y - 2, m_display->getWidth() - 1, y - 2, SSD1306_WHITE);
    drawText(status, 0, y, 1);
}

void DisplayManager::drawBox(int16_t x, int16_t y, int16_t w, int16_t h, bool filled) {
    if (!m_display) return;
    
    if (filled) {
        m_display->fillRect(x, y, w, h, SSD1306_WHITE);
    } else {
        m_display->drawRect(x, y, w, h, SSD1306_WHITE);
    }
}

void DisplayManager::drawRoundedBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool filled) {
    if (!m_display) return;
    
    if (filled) {
        m_display->fillRect(x + r, y, w - 2 * r, h, SSD1306_WHITE);
        m_display->fillRect(x, y + r, w, h - 2 * r, SSD1306_WHITE);
        m_display->fillCircle(x + r, y + r, r, SSD1306_WHITE);
        m_display->fillCircle(x + w - r - 1, y + r, r, SSD1306_WHITE);
        m_display->fillCircle(x + r, y + h - r - 1, r, SSD1306_WHITE);
        m_display->fillCircle(x + w - r - 1, y + h - r - 1, r, SSD1306_WHITE);
    } else {
        m_display->drawLine(x + r, y, x + w - r, y, SSD1306_WHITE);
        m_display->drawLine(x + r, y + h - 1, x + w - r, y + h - 1, SSD1306_WHITE);
        m_display->drawLine(x, y + r, x, y + h - r, SSD1306_WHITE);
        m_display->drawLine(x + w - 1, y + r, x + w - 1, y + h - r, SSD1306_WHITE);
        
        m_display->drawCircle(x + r, y + r, r, SSD1306_WHITE);
        m_display->drawCircle(x + w - r - 1, y + r, r, SSD1306_WHITE);
        m_display->drawCircle(x + r, y + h - r - 1, r, SSD1306_WHITE);
        m_display->drawCircle(x + w - r - 1, y + h - r - 1, r, SSD1306_WHITE);
    }
}

int16_t DisplayManager::getTextWidth(const char* text, uint8_t size) const {
    if (!text) return 0;
    return strlen(text) * 6 * size;
}

int16_t DisplayManager::getTextHeight(uint8_t size) const {
    return 8 * size;
}