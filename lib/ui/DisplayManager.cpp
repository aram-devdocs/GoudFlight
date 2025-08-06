#include "DisplayManager.h"

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

bool DisplayManager::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        return false;
    }
    clear();
    return true;
}

void DisplayManager::clear() {
    display.clearDisplay();
    display.display();
}

void DisplayManager::update() {
    display.display();
}

void DisplayManager::showButtonStatus(uint8_t buttonData) {
    display.clearDisplay();
    
    int squareSize = 12;
    int spacing = 4;
    int totalWidth = (squareSize * 8) + (spacing * 7);
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    int startY = 20;
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 5);
    display.println(F("Button Status"));
    
    for (int i = 0; i < 8; i++) {
        int x = startX + (i * (squareSize + spacing));
        
        if ((buttonData >> i) & 1) {
            display.drawRect(x, startY, squareSize, squareSize, SSD1306_WHITE);
        } else {
            display.fillRect(x, startY, squareSize, squareSize, SSD1306_WHITE);
        }
        
        display.setCursor(x + 3, startY + squareSize + 5);
        display.print(i + 1);
    }
    
    display.setCursor(5, 50);
    display.print(F("Data: 0x"));
    if (buttonData < 16) display.print(F("0"));
    display.print(buttonData, HEX);
    display.print(F(" ("));
    for (int i = 7; i >= 0; i--) {
        display.print((buttonData >> i) & 1);
    }
    display.print(F(")"));
    
    update();
}

void DisplayManager::showStartupScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println(F("Handheld"));
    display.setCursor(10, 30);
    display.println(F("Controller"));
    display.setTextSize(1);
    display.setCursor(30, 50);
    display.println(F("Initializing..."));
    update();
}

void DisplayManager::showError(const char* message) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("ERROR:"));
    display.println(message);
    update();
}