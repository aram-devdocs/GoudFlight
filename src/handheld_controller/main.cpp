#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define PISO_LOAD_PIN 25
#define PISO_CLK_PIN 26
#define PISO_DATA_PIN 27

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t readShiftRegister() {
    uint8_t data = 0;
    
    digitalWrite(PISO_LOAD_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(PISO_LOAD_PIN, HIGH);
    
    for (int i = 0; i < 8; i++) {
        data = data << 1;
        if (digitalRead(PISO_DATA_PIN)) {
            data = data | 1;
        }
        digitalWrite(PISO_CLK_PIN, HIGH);
        delayMicroseconds(5);
        digitalWrite(PISO_CLK_PIN, LOW);
        delayMicroseconds(5);
    }
    
    return data;
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting Hello World Test...");
    
    // Setup shift register pins
    pinMode(PISO_LOAD_PIN, OUTPUT);
    pinMode(PISO_CLK_PIN, OUTPUT);
    pinMode(PISO_DATA_PIN, INPUT);
    
    digitalWrite(PISO_CLK_PIN, LOW);
    digitalWrite(PISO_LOAD_PIN, HIGH);
    
    // Initialize display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    
    Serial.println("Display initialized!");
    
    // Clear and show hello world
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println(F("Hello"));
    display.setCursor(10, 30);
    display.println(F("World!"));
    display.display();
    
    delay(2000);
    Serial.println("Setup complete!");
}

void loop() {
    uint8_t buttonData = readShiftRegister();
    
    display.clearDisplay();
    
    // Title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(30, 0);
    display.println(F("Button Test"));
    
    // Draw 8 boxes for buttons
    for (int i = 0; i < 8; i++) {
        int x = 10 + (i * 14);
        int y = 20;
        
        if ((buttonData >> i) & 1) {
            // Empty box for unpressed (HIGH)
            display.drawRect(x, y, 12, 12, SSD1306_WHITE);
        } else {
            // Filled box for pressed (LOW)
            display.fillRect(x, y, 12, 12, SSD1306_WHITE);
        }
        
        // Button number
        display.setCursor(x + 3, y + 16);
        display.print(i + 1);
    }
    
    // Show hex value
    display.setCursor(5, 50);
    display.print(F("Data: 0x"));
    if (buttonData < 16) display.print(F("0"));
    display.print(buttonData, HEX);
    
    display.display();
    
    // Debug to serial
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
        Serial.print("Buttons: 0x");
        Serial.println(buttonData, HEX);
        lastPrint = millis();
    }
    
    delay(50);
}