#ifndef ESPNOW_UTILS_H
#define ESPNOW_UTILS_H

#include <Arduino.h>

class ESPNowUtils {
public:
    static bool parseMacAddress(const char* str, uint8_t* mac) {
        if (!str || !mac) return false;
        
        int values[6];
        int count = sscanf(str, "%x:%x:%x:%x:%x:%x",
                          &values[0], &values[1], &values[2],
                          &values[3], &values[4], &values[5]);
        
        if (count != 6) return false;
        
        for (int i = 0; i < 6; i++) {
            if (values[i] < 0 || values[i] > 255) return false;
            mac[i] = (uint8_t)values[i];
        }
        
        return true;
    }
};

#endif