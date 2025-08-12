// Simple Serial Test for ESP32 Base Station
// This test program sends data over UART2 to verify communication with RPi

#include <Arduino.h>
#include <HardwareSerial.h>

// UART2 pins - must match SerialInterface configuration
#define TX_PIN 17
#define RX_PIN 18
#define BAUDRATE 115200

HardwareSerial Serial2(2);
unsigned long lastSendTime = 0;
unsigned long messageCount = 0;

void setup() {
    // Initialize USB serial for debugging
    Serial.begin(115200);
    Serial.println("ESP32 Serial Test Starting...");
    
    // Initialize UART2 for RPi communication
    Serial2.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("UART2 initialized on pins:");
    Serial.printf("  TX: GPIO%d\n", TX_PIN);
    Serial.printf("  RX: GPIO%d\n", RX_PIN);
    Serial.printf("  Baudrate: %d\n", BAUDRATE);
    
    delay(1000);
    
    // Send initial test message
    Serial2.println("ESP32 Serial Test - Connection Established!");
    Serial2.println("{\"type\":\"STATUS\",\"status\":\"TEST_MODE\",\"timestamp\":0}");
    
    Serial.println("Sent initial message to RPi");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Send test message every 2 seconds
    if (currentTime - lastSendTime >= 2000) {
        lastSendTime = currentTime;
        messageCount++;
        
        // Send plain text message
        Serial2.printf("Test message #%lu at %lu ms\n", messageCount, currentTime);
        
        // Send JSON formatted message
        Serial2.printf("{\"type\":\"HEARTBEAT\",\"count\":%lu,\"timestamp\":%lu}\n", 
                      messageCount, currentTime);
        
        // Also send telemetry-style message
        if (messageCount % 3 == 0) {
            Serial2.printf("{\"type\":\"TELEMETRY\",\"data\":{\"uptime_ms\":%lu,\"free_heap\":%d,\"cpu_usage\":0.0,\"esp_now_connected\":false,\"remote_devices\":0,\"signal_strength\":-50,\"battery_voltage\":3.3}}\n",
                          currentTime, ESP.getFreeHeap());
        }
        
        // Echo to USB serial for debugging
        Serial.printf("Sent message #%lu\n", messageCount);
    }
    
    // Echo any received data from RPi
    while (Serial2.available()) {
        char c = Serial2.read();
        Serial.print("Received from RPi: ");
        Serial.println(c);
        
        // Echo back to RPi
        Serial2.print("Echo: ");
        Serial2.println(c);
    }
    
    // Also check USB serial for manual test commands
    while (Serial.available()) {
        char c = Serial.read();
        Serial.print("Sending to RPi: ");
        Serial.println(c);
        Serial2.print(c);
    }
}