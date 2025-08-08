#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

namespace Constants {
    
    namespace Serial {
        constexpr uint32_t BAUD_RATE = 115200;
        constexpr uint32_t INIT_TIMEOUT_MS = 5000;
        constexpr uint32_t INIT_DELAY_MS = 1000;
    }
    
    namespace Timing {
        constexpr uint32_t HEARTBEAT_INTERVAL_MS = 1000;
        constexpr uint32_t DISPLAY_UPDATE_INTERVAL_MS = 50;
        constexpr uint32_t INPUT_POLL_INTERVAL_MS = 10;
        constexpr uint32_t DEBUG_PRINT_INTERVAL_MS = 1000;
        constexpr uint32_t WATCHDOG_TIMEOUT_MS = 10000;
        constexpr uint32_t STATE_TRANSITION_DELAY_MS = 100;
    }
    
    namespace Hardware {
        constexpr uint8_t DEFAULT_LED_PIN = 2;
        constexpr uint8_t LED_BUILTIN_ESP32 = 2;
        constexpr uint8_t LED_BUILTIN_ESP32S3 = 48;
        
        constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
        constexpr uint32_t LONG_PRESS_DURATION_MS = 1000;
        constexpr uint32_t DOUBLE_CLICK_WINDOW_MS = 500;
    }
    
    namespace Display {
        constexpr uint8_t DEFAULT_BRIGHTNESS = 255;
        constexpr uint8_t MIN_BRIGHTNESS = 0;
        constexpr uint8_t MAX_BRIGHTNESS = 255;
        constexpr uint8_t TEXT_SIZE_SMALL = 1;
        constexpr uint8_t TEXT_SIZE_NORMAL = 1;
        constexpr uint8_t TEXT_SIZE_LARGE = 2;
    }
    
    namespace Memory {
        constexpr uint32_t MIN_FREE_HEAP_WARNING = 10240;
        constexpr uint32_t MIN_FREE_HEAP_CRITICAL = 4096;
        constexpr uint32_t STACK_SIZE_DEFAULT = 4096;
        constexpr uint32_t STACK_SIZE_LARGE = 8192;
    }
    
    namespace Communication {
        constexpr uint32_t MESSAGE_TIMEOUT_MS = 5000;
        constexpr uint32_t RETRY_DELAY_MS = 1000;
        constexpr uint8_t MAX_RETRIES = 3;
        constexpr uint16_t MAX_MESSAGE_SIZE = 256;
    }
    
    namespace System {
        constexpr const char* VERSION = "1.0.0";
        constexpr const char* BUILD_DATE = __DATE__;
        constexpr const char* BUILD_TIME = __TIME__;
        constexpr uint32_t STARTUP_SCREEN_DURATION_MS = 2000;
    }
}

#endif