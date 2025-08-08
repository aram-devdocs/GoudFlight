#include "system_info.h"

void SystemInfo::printSystemInfo(const char* device_name) {
    Serial.println(F("===================================="));
    Serial.print(F("Device: "));
    Serial.println(device_name);
    Serial.println(F("===================================="));
    
    char mac_address[18];
    getMacAddress(mac_address, sizeof(mac_address));
    Serial.print(F("MAC Address: "));
    Serial.println(mac_address);
    
    #ifdef ESP32
    char chip_id[17];
    getChipId(chip_id, sizeof(chip_id));
    Serial.print(F("Chip ID: "));
    Serial.println(chip_id);
    
    Serial.print(F("Chip Model: "));
    Serial.println(getChipModel());
    
    Serial.print(F("Chip Revision: "));
    Serial.println(getChipRevision());
    
    Serial.print(F("CPU Frequency: "));
    Serial.print(getCpuFrequencyMHz());
    Serial.println(F(" MHz"));
    
    Serial.print(F("Flash Size: "));
    Serial.print(getFlashSize() / (1024 * 1024));
    Serial.println(F(" MB"));
    
    Serial.print(F("SDK Version: "));
    Serial.println(getSdkVersion());
    #endif
    
    Serial.print(F("Free Heap: "));
    Serial.print(getFreeHeap());
    Serial.println(F(" bytes"));
    
    #ifdef ESP32
    Serial.print(F("Total Heap: "));
    Serial.print(getTotalHeap());
    Serial.println(F(" bytes"));
    #endif
    
    Serial.println(F("===================================="));
}

void SystemInfo::getMacAddress(char* buffer, size_t buffer_size) {
    if (buffer == nullptr || buffer_size < 18) {
        return;
    }
    
    #ifdef ESP32
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(buffer, buffer_size, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    #elif defined(ESP8266)
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(buffer, buffer_size, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    #else
    strncpy(buffer, "Not Available", buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    #endif
}

void SystemInfo::getChipId(char* buffer, size_t buffer_size) {
    if (buffer == nullptr || buffer_size < 17) {
        return;
    }
    
    #ifdef ESP32
    uint64_t chipid = ESP.getEfuseMac();
    snprintf(buffer, buffer_size, "%04X%08X",
             (uint16_t)(chipid >> 32), (uint32_t)chipid);
    #elif defined(ESP8266)
    snprintf(buffer, buffer_size, "%08X", ESP.getChipId());
    #else
    strncpy(buffer, "Not Available", buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    #endif
}

uint32_t SystemInfo::getFreeHeap() {
    #if defined(ESP32) || defined(ESP8266)
    return ESP.getFreeHeap();
    #else
    return 0;
    #endif
}

uint32_t SystemInfo::getTotalHeap() {
    #ifdef ESP32
    return ESP.getHeapSize();
    #elif defined(ESP8266)
    return 81920;  // ESP8266 typical heap size
    #else
    return 0;
    #endif
}

float SystemInfo::getCpuFrequencyMHz() {
    #ifdef ESP32
    return ESP.getCpuFreqMHz();
    #elif defined(ESP8266)
    return ESP.getCpuFreqMHz();
    #else
    return 0;
    #endif
}

const char* SystemInfo::getChipModel() {
    #ifdef ESP32
    return ESP.getChipModel();
    #elif defined(ESP8266)
    return "ESP8266";
    #else
    return "Unknown";
    #endif
}

uint8_t SystemInfo::getChipRevision() {
    #ifdef ESP32
    return ESP.getChipRevision();
    #elif defined(ESP8266)
    return 0;
    #else
    return 0;
    #endif
}

uint32_t SystemInfo::getFlashSize() {
    #if defined(ESP32) || defined(ESP8266)
    return ESP.getFlashChipSize();
    #else
    return 0;
    #endif
}

const char* SystemInfo::getSdkVersion() {
    #ifdef ESP32
    return ESP.getSdkVersion();
    #elif defined(ESP8266)
    return ESP.getSdkVersion();
    #else
    return "Unknown";
    #endif
}