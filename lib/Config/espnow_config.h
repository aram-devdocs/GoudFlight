#ifndef ESPNOW_CONFIG_H_GLOBAL
#define ESPNOW_CONFIG_H_GLOBAL

#include <Arduino.h>

namespace ESPNowGlobalConfig {
    // MAC addresses for ESP-NOW pairing
    // These are loaded from .env file at build time
    // Create a .env file from .env.template and set your MAC addresses
    
    #ifndef BASE_STATION_MAC_ARRAY
        #error "BASE_STATION_MAC_ADDRESS not defined! Please create a .env file with MAC addresses. See .env.template for format."
    #endif
    
    #ifndef HANDHELD_MAC_ARRAY
        #error "HANDHELD_MAC_ADDRESS not defined! Please create a .env file with MAC addresses. See .env.template for format."
    #endif
    
    // Base station MAC address (from .env file)
    static constexpr uint8_t BASE_STATION_MAC[6] = BASE_STATION_MAC_ARRAY;
    
    // Handheld controller MAC address (from .env file)
    static constexpr uint8_t HANDHELD_MAC[6] = HANDHELD_MAC_ARRAY;
    
    // Broadcast MAC for discovery
    static constexpr uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    // Security settings
    static constexpr bool ENABLE_ENCRYPTION = false;  // Set to true for production
    static constexpr uint8_t PMK_KEY[16] = {0};  // Primary master key for encryption
    static constexpr uint8_t LMK_KEY[16] = {0};  // Local master key for encryption
    
    // Connection settings
    static constexpr uint32_t MAX_RETRY_COUNT = 5;
    static constexpr uint32_t RETRY_DELAY_MS = 1000;
    static constexpr uint32_t MESSAGE_QUEUE_SIZE = 32;
    
    // Validation settings
    static constexpr bool ENABLE_CRC_CHECK = true;
    static constexpr bool ENABLE_SEQUENCE_CHECK = true;
    static constexpr uint32_t MAX_SEQUENCE_GAP = 100;
}

#endif