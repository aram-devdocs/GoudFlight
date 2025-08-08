#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <Arduino.h>

#ifdef ESP32
    #include <WiFi.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
#endif

class SystemInfo {
public:
    static void printSystemInfo(const char* device_name);
    static void getMacAddress(char* buffer, size_t buffer_size);
    static void getChipId(char* buffer, size_t buffer_size);
    static uint32_t getFreeHeap();
    static uint32_t getTotalHeap();
    static float getCpuFrequencyMHz();
    static const char* getChipModel();
    static uint8_t getChipRevision();
    static uint32_t getFlashSize();
    static const char* getSdkVersion();
};

#endif