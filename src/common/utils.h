#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <Arduino.h>

class Utils {
public:
    static void printBoardInfo() {
        Serial.println("======================");
        Serial.print("Board Type: ");
        #ifdef BOARD_TYPE
            #if BOARD_TYPE == CONTROLLER
                Serial.println("CONTROLLER");
            #elif BOARD_TYPE == BASE_STATION
                Serial.println("BASE_STATION");
            #elif BOARD_TYPE == FLIGHT_CONTROLLER
                Serial.println("FLIGHT_CONTROLLER");
            #else
                Serial.println("UNKNOWN");
            #endif
        #else
            Serial.println("NOT DEFINED");
        #endif
        Serial.print("ESP32 Chip ID: ");
        Serial.println(ESP.getChipId(), HEX);
        Serial.print("Free Heap: ");
        Serial.println(ESP.getFreeHeap());
        Serial.println("======================");
    }
    
    static void blinkLED(int pin, int times, int delayMs) {
        for(int i = 0; i < times; i++) {
            digitalWrite(pin, HIGH);
            delay(delayMs);
            digitalWrite(pin, LOW);
            delay(delayMs);
        }
    }
};

#endif