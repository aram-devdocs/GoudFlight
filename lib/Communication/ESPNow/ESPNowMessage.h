#ifndef ESPNOW_MESSAGE_H
#define ESPNOW_MESSAGE_H

#include <Arduino.h>
#include <string.h>
#include "ESPNowConfig.h"

// Simple CRC16 implementation for message validation
static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

struct ESPNowMessage {
    uint8_t magic;
    uint8_t type;
    uint8_t role;
    uint32_t sequence;
    uint32_t timestamp;
    uint8_t data[32];
    uint16_t crc;  // CRC16 of all preceding fields
    
    ESPNowMessage() : magic(ESPNowConfig::MESSAGE_MAGIC), type(0), role(0), sequence(0), timestamp(0), crc(0) {
        memset(data, 0, sizeof(data));
    }
    
    bool isValid() const {
        // Check magic number
        if (magic != ESPNowConfig::MESSAGE_MAGIC) {
            return false;
        }
        
        // Validate CRC
        uint16_t calculated_crc = calculateCRC();
        return crc == calculated_crc;
    }
    
    uint16_t calculateCRC() const {
        // Calculate CRC of all fields except the CRC field itself
        size_t crc_offset = offsetof(ESPNowMessage, crc);
        return crc16(reinterpret_cast<const uint8_t*>(this), crc_offset);
    }
    
    void updateCRC() {
        crc = calculateCRC();
    }
    
    void setPingData(uint32_t counter) {
        type = ESPNowConfig::MSG_PING;
        memcpy(data, &counter, sizeof(counter));
        updateCRC();
    }
    
    void setPongData(uint32_t counter) {
        type = ESPNowConfig::MSG_PONG;
        memcpy(data, &counter, sizeof(counter));
        updateCRC();
    }
    
    uint32_t getPingPongCounter() const {
        uint32_t counter = 0;
        memcpy(&counter, data, sizeof(counter));
        return counter;
    }
    
    // Screen sync methods
    void setScreenSync(uint8_t screenId, const char* screenName) {
        type = ESPNowConfig::MSG_SCREEN_SYNC;
        data[0] = screenId;
        uint8_t nameLen = screenName ? strlen(screenName) : 0;
        if (nameLen > 30) nameLen = 30;  // Limit name length
        data[1] = nameLen;
        if (nameLen > 0) {
            memcpy(&data[2], screenName, nameLen);
        }
        updateCRC();
    }
    
    uint8_t getScreenId() const {
        return data[0];
    }
    
    void getScreenName(char* buffer, size_t bufferSize) const {
        uint8_t nameLen = data[1];
        if (nameLen > 30) nameLen = 30;
        size_t copyLen = (nameLen < bufferSize - 1) ? nameLen : bufferSize - 1;
        memcpy(buffer, &data[2], copyLen);
        buffer[copyLen] = '\0';
    }
    
    // Button data methods
    void setButtonData(uint8_t buttonStates) {
        type = ESPNowConfig::MSG_BUTTON_DATA;
        data[0] = buttonStates;
        uint32_t timestamp = millis();
        memcpy(&data[1], &timestamp, sizeof(timestamp));
        updateCRC();
    }
    
    uint8_t getButtonStates() const {
        return data[0];
    }
    
    uint32_t getButtonTimestamp() const {
        uint32_t timestamp = 0;
        memcpy(&timestamp, &data[1], sizeof(timestamp));
        return timestamp;
    }
    
    // Input event methods
    void setInputEvent(uint8_t eventType, uint8_t buttonId, uint16_t eventData) {
        type = ESPNowConfig::MSG_INPUT_EVENT;
        data[0] = eventType;
        data[1] = buttonId;
        memcpy(&data[2], &eventData, sizeof(eventData));
        updateCRC();
    }
    
    uint8_t getInputEventType() const {
        return data[0];
    }
    
    uint8_t getInputButtonId() const {
        return data[1];
    }
    
    uint16_t getInputEventData() const {
        uint16_t eventData = 0;
        memcpy(&eventData, &data[2], sizeof(eventData));
        return eventData;
    }
} __attribute__((packed));

#endif