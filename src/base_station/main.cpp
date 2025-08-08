#include <Arduino.h>
#include "../../lib/HAL/Board/base_station_bsp.h"
#include "../../lib/Config/base_station_config.h"
#include "../../lib/HAL/Display/display_interface.h"
#include "../../lib/HAL/Core/hal_errors.h"
#include "../../lib/SystemInfo/system_info.h"

static display_instance_t* lcd_display = nullptr;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {
        ; // Wait for serial port to connect or timeout after 5 seconds
    }
    delay(1000); // Give serial monitor time to fully initialize
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("============================");
    Serial.println("Base Station Starting...");
    Serial.println("============================");
    
    SystemInfo::printSystemInfo("Base Station");
    
    hal_status_t result = base_station_bsp_init(&base_station_profile.header);
    if (result != HAL_OK) {
        Serial.print("Failed to initialize BSP: ");
        Serial.println(result);
        return;
    }
    
    lcd_display = base_station_bsp_get_display();
    if (!lcd_display || !lcd_display->interface) {
        Serial.println("Warning: No display available");
        return;
    }
    
    lcd_display->interface->clear(lcd_display);
    
    // Set maximum brightness
    lcd_display->interface->set_brightness(lcd_display, 255);
    
    display_point_t cursor = {0, 0};
    lcd_display->interface->set_text_cursor(lcd_display, &cursor);
    lcd_display->interface->write_text(lcd_display, "Hello World!", 1);
    
    cursor.y = 1;
    lcd_display->interface->set_text_cursor(lcd_display, &cursor);
    lcd_display->interface->write_text(lcd_display, "Base Station OK", 1);
    
    Serial.println("LCD initialized - displaying Hello World!");
}

void loop() {
    static unsigned long counter = 0;
    static unsigned long last_update = 0;
    
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
    unsigned long current_time = millis();
    if (current_time - last_update >= 1000) {
        counter++;
        last_update = current_time;
        
        Serial.print("Base Station: Running for ");
        Serial.print(counter);
        Serial.println(" seconds");
        
        if (lcd_display && lcd_display->interface) {
            char time_str[16];
            snprintf(time_str, sizeof(time_str), "Time: %lu s", counter);
            
            display_point_t cursor = {0, 1};
            lcd_display->interface->set_text_cursor(lcd_display, &cursor);
            lcd_display->interface->write_text(lcd_display, time_str, 1);
        }
    }
    
    delay(100);
}