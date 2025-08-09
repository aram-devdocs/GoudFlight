#ifndef ESPNOW_CONFIG_H
#define ESPNOW_CONFIG_H

#include <Arduino.h>

namespace ESPNowConfig {
    static constexpr uint8_t CHANNEL = 1;
    static constexpr uint8_t ENCRYPT = 0;
    
    static constexpr uint32_t SEARCH_INTERVAL_MS = 1000;
    static constexpr uint32_t PAIRING_TIMEOUT_MS = 10000;
    static constexpr uint32_t PING_INTERVAL_MS = 1000;
    static constexpr uint32_t CONNECTION_TIMEOUT_MS = 5000;
    
    static constexpr uint8_t MAX_RETRIES = 3;
    static constexpr uint8_t MESSAGE_MAGIC = 0xAB;
    
    enum MessageType : uint8_t {
        MSG_ANNOUNCE = 0x01,
        MSG_PAIR_REQUEST = 0x02,
        MSG_PAIR_RESPONSE = 0x03,
        MSG_PING = 0x04,
        MSG_PONG = 0x05,
        MSG_DISCONNECT = 0x06,
        MSG_SCREEN_SYNC = 0x07,
        MSG_BUTTON_DATA = 0x08,
        MSG_INPUT_EVENT = 0x09
    };
    
    enum DeviceRole : uint8_t {
        ROLE_HANDHELD = 0x10,
        ROLE_BASE_STATION = 0x20
    };
}

#endif