#ifndef ESPNOW_MESSAGE_H
#define ESPNOW_MESSAGE_H

#include <Arduino.h>
#include "ESPNowConfig.h"

struct ESPNowMessage {
    uint8_t magic;
    uint8_t type;
    uint8_t role;
    uint32_t sequence;
    uint32_t timestamp;
    uint8_t data[32];
    
    ESPNowMessage() : magic(ESPNowConfig::MESSAGE_MAGIC), type(0), role(0), sequence(0), timestamp(0) {
        memset(data, 0, sizeof(data));
    }
    
    bool isValid() const {
        return magic == ESPNowConfig::MESSAGE_MAGIC;
    }
    
    void setPingData(uint32_t counter) {
        type = ESPNowConfig::MSG_PING;
        memcpy(data, &counter, sizeof(counter));
    }
    
    void setPongData(uint32_t counter) {
        type = ESPNowConfig::MSG_PONG;
        memcpy(data, &counter, sizeof(counter));
    }
    
    uint32_t getPingPongCounter() const {
        uint32_t counter = 0;
        memcpy(&counter, data, sizeof(counter));
        return counter;
    }
} __attribute__((packed));

#endif