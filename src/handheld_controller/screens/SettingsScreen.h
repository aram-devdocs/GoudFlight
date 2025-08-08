#ifndef HANDHELD_SETTINGS_SCREEN_H
#define HANDHELD_SETTINGS_SCREEN_H

#include "../../../lib/Core/AppScreen.h"

class HandheldSettingsScreen : public AppScreen {
public:
    HandheldSettingsScreen();
    
    void onDraw(display_instance_t* display) override;
    void onButtonPress(uint8_t button_id) override;
    
private:
    enum class Setting {
        BRIGHTNESS,
        VOLUME,
        WIFI,
        BLUETOOTH,
        COUNT
    };
    
    Setting current_setting;
    uint8_t brightness_level;
    uint8_t volume_level;
    bool wifi_enabled;
    bool bluetooth_enabled;
    
    void drawSettingsList(display_instance_t* display);
    void drawCurrentValue(display_instance_t* display);
    void adjustSetting(int8_t direction);
};

#endif