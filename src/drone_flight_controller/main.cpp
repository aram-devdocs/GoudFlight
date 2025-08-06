#include <Arduino.h>

#define LED_BUILTIN 2

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("============================");
    Serial.println("Drone Flight Controller");
    Serial.println("Hello World!");
    Serial.println("============================");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Drone FC: Alive");
    delay(1000);
    
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}