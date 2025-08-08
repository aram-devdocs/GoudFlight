#include <Arduino.h>
#include "../../lib/HAL/Board/handheld_bsp.h"
#include "../../lib/HAL/Core/hal_errors.h"
#include "../../lib/SystemInfo/system_info.h"

static handheld_hardware_t hardware = {nullptr, nullptr};

static void draw_test_screen() {
    if (!hardware.display || !hardware.display->interface) {
        return;
    }
    
    const display_interface_t* display = hardware.display->interface;
    
    display->clear(hardware.display);
    
    display_point_t cursor = {30, 0};
    display->set_text_cursor(hardware.display, &cursor);
    display->write_text(hardware.display, "Button Test", 1);
    
    display_point_t line_start = {0, 10};
    display_point_t line_end = {127, 10};
    display->draw_line(hardware.display, &line_start, &line_end, DISPLAY_COLOR_WHITE);
    
    if (hardware.input && hardware.input->interface) {
        uint8_t num_buttons = hardware.input->interface->get_channel_count(hardware.input);
        if (num_buttons > 8) num_buttons = 8;
        
        for (uint8_t i = 0; i < num_buttons; i++) {
            bool pressed = false;
            hardware.input->interface->read_channel(hardware.input, i, &pressed);
            
            display_rect_t button_rect = {
                .x = static_cast<int16_t>(10 + i * 14),
                .y = 20,
                .width = 12,
                .height = 12
            };
            
            display->draw_rect(hardware.display, &button_rect, DISPLAY_COLOR_WHITE, pressed);
            
            cursor.x = button_rect.x + 3;
            cursor.y = button_rect.y + 16;
            display->set_text_cursor(hardware.display, &cursor);
            
            char num_str[2] = {static_cast<char>('1' + i), '\0'};
            display->write_text(hardware.display, num_str, 1);
        }
        
        uint32_t raw_state = 0;
        hardware.input->interface->read_raw(hardware.input, &raw_state);
        uint8_t inverted = ~(raw_state & 0xFF);
        
        cursor.x = 5;
        cursor.y = 50;
        display->set_text_cursor(hardware.display, &cursor);
        display->write_text(hardware.display, "Data: 0x", 1);
        
        char hex_str[3];
        sprintf(hex_str, "%02X", inverted);
        display->write_text(hardware.display, hex_str, 1);
    }
    
    display->refresh(hardware.display);
}

static void process_input_events() {
    if (!hardware.input || !hardware.input->interface) {
        return;
    }
    
    input_event_data_t events[8];
    uint8_t num_events = 0;
    
    hardware.input->interface->get_events(hardware.input, events, 8, &num_events);
    
    for (uint8_t i = 0; i < num_events; i++) {
        if (events[i].event & INPUT_EVENT_PRESSED) {
            Serial.print(F("Button "));
            Serial.print(events[i].channel + 1);
            Serial.println(F(" pressed"));
        }
        if (events[i].event & INPUT_EVENT_RELEASED) {
            Serial.print(F("Button "));
            Serial.print(events[i].channel + 1);
            Serial.println(F(" released"));
        }
        if (events[i].event & INPUT_EVENT_LONG_PRESS) {
            Serial.print(F("Button "));
            Serial.print(events[i].channel + 1);
            Serial.println(F(" long press"));
        }
        if (events[i].event & INPUT_EVENT_DOUBLE_CLICK) {
            Serial.print(F("Button "));
            Serial.print(events[i].channel + 1);
            Serial.println(F(" double click"));
        }
    }
}

static void show_startup_screen() {
    if (!hardware.display || !hardware.display->interface) {
        return;
    }
    
    const display_interface_t* display = hardware.display->interface;
    
    display->clear(hardware.display);
    
    display_point_t cursor = {20, 10};
    display->set_text_cursor(hardware.display, &cursor);
    display->write_text(hardware.display, "GoudFlight", 2);
    
    cursor.x = 25;
    cursor.y = 35;
    display->set_text_cursor(hardware.display, &cursor);
    display->write_text(hardware.display, "HAL v1.0", 1);
    
    cursor.x = 15;
    cursor.y = 50;
    display->set_text_cursor(hardware.display, &cursor);
    display->write_text(hardware.display, "Initializing...", 1);
    
    display->refresh(hardware.display);
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {
        ; // Wait for serial port to connect or timeout after 5 seconds
    }
    delay(1000); // Give serial monitor time to fully initialize
    Serial.println(F("=== GoudFlight Handheld Controller ==="));
    Serial.println(F("Starting with HAL Architecture..."));
    
    SystemInfo::printSystemInfo("Handheld Controller");
    
    hal_status_t status = handheld_bsp_init(&hardware);
    if (status != HAL_OK) {
        Serial.print(F("BSP initialization failed: "));
        Serial.println(hal_get_error_string(status));
        
        char error_msg[128];
        handheld_get_last_error(error_msg, sizeof(error_msg));
        Serial.print(F("Details: "));
        Serial.println(error_msg);
        
        while(1) {
            delay(1000);
        }
    }
    
    handheld_log_system_info();
    
    show_startup_screen();
    delay(2000);
    
    Serial.println(F("Setup complete!"));
}

void loop() {
    static uint32_t last_update = 0;
    static uint32_t last_debug = 0;
    uint32_t current_time = millis();
    
    if (hardware.input && hardware.input->interface) {
        hardware.input->interface->update(hardware.input);
        process_input_events();
    }
    
    if (current_time - last_update >= 50) {
        draw_test_screen();
        last_update = current_time;
    }
    
    if (current_time - last_debug >= 1000) {
        if (hardware.input && hardware.input->interface) {
            uint32_t raw_state = 0;
            hardware.input->interface->read_raw(hardware.input, &raw_state);
            Serial.print(F("Buttons: 0x"));
            Serial.println(~raw_state & 0xFF, HEX);
        }
        last_debug = current_time;
    }
    
    delay(10);
}