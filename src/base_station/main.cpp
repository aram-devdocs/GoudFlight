#include <Arduino.h>

#define LED_BUILTIN 2

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("============================");
    Serial.println("Base Station");
    Serial.println("Hello World!");
    Serial.println("============================");
}

void loop() {
    static unsigned long counter = 0;
    
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
    Serial.print("Base Station: Running for ");
    Serial.print(++counter);
    Serial.println(" seconds");
    
    delay(1000);
}