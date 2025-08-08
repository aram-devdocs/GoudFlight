#include "SettingsScreen.h"
#include "../../../lib/Business/DisplayController.h"

HandheldSettingsScreen::HandheldSettingsScreen()
    : AppScreen("Settings")
    , current_setting(Setting::BRIGHTNESS)
    , brightness_level(75)
    , volume_level(50)
    , wifi_enabled(false)
    , bluetooth_enabled(false) {
}

void HandheldSettingsScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display->interface->clear(display);
    DisplayController::drawHeader(display, "Settings");
    drawSettingsList(display);
    drawCurrentValue(display);
    DisplayController::drawFooter(display, "1:Up 2:Down 3:Adj 4:Back");
    display->interface->refresh(display);
}

void HandheldSettingsScreen::onButtonPress(uint8_t button_id) {
    switch (button_id) {
        case 0:
            if ((uint8_t)current_setting > 0) {
                current_setting = (Setting)((uint8_t)current_setting - 1);
                requestRedraw();
            }
            break;
            
        case 1:
            if ((uint8_t)current_setting < (uint8_t)Setting::COUNT - 1) {
                current_setting = (Setting)((uint8_t)current_setting + 1);
                requestRedraw();
            }
            break;
            
        case 2:
            adjustSetting(1);
            requestRedraw();
            break;
            
        case 3:
            adjustSetting(-1);
            requestRedraw();
            break;
    }
}

void HandheldSettingsScreen::drawSettingsList(display_instance_t* display) {
    const char* settings[] = {"Brightness", "Volume", "WiFi", "Bluetooth"};
    
    for (uint8_t i = 0; i < (uint8_t)Setting::COUNT; i++) {
        display_point_t cursor = {5, (int16_t)(15 + i * 10)};
        display->interface->set_text_cursor(display, &cursor);
        
        if (i == (uint8_t)current_setting) {
            display->interface->write_text(display, ">", 1);
        }
        
        cursor.x = 15;
        display->interface->set_text_cursor(display, &cursor);
        display->interface->write_text(display, settings[i], 1);
    }
}

void HandheldSettingsScreen::drawCurrentValue(display_instance_t* display) {
    char value_str[16];
    display_point_t cursor = {80, (int16_t)(15 + (uint8_t)current_setting * 10)};
    
    switch (current_setting) {
        case Setting::BRIGHTNESS:
            snprintf(value_str, sizeof(value_str), "%d%%", brightness_level);
            break;
        case Setting::VOLUME:
            snprintf(value_str, sizeof(value_str), "%d%%", volume_level);
            break;
        case Setting::WIFI:
            snprintf(value_str, sizeof(value_str), "%s", wifi_enabled ? "ON" : "OFF");
            break;
        case Setting::BLUETOOTH:
            snprintf(value_str, sizeof(value_str), "%s", bluetooth_enabled ? "ON" : "OFF");
            break;
        default:
            return;
    }
    
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, value_str, 1);
}

void HandheldSettingsScreen::adjustSetting(int8_t direction) {
    switch (current_setting) {
        case Setting::BRIGHTNESS:
            if (direction > 0 && brightness_level < 100) brightness_level += 5;
            if (direction < 0 && brightness_level > 0) brightness_level -= 5;
            break;
            
        case Setting::VOLUME:
            if (direction > 0 && volume_level < 100) volume_level += 5;
            if (direction < 0 && volume_level > 0) volume_level -= 5;
            break;
            
        case Setting::WIFI:
            wifi_enabled = !wifi_enabled;
            break;
            
        case Setting::BLUETOOTH:
            bluetooth_enabled = !bluetooth_enabled;
            break;
            
        default:
            break;
    }
}