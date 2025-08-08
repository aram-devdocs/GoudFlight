#ifndef HANDHELD_FLIGHT_CONTROL_SCREEN_H
#define HANDHELD_FLIGHT_CONTROL_SCREEN_H

#include "../../../lib/Core/AppScreen.h"

class HandheldFlightControlScreen : public AppScreen {
public:
    HandheldFlightControlScreen();
    
    void onEnter() override;
    void onDraw(display_instance_t* display) override;
    void onButtonPress(uint8_t button_id) override;
    void onUpdate(uint32_t delta_ms) override;
    
private:
    enum class FlightMode {
        DISARMED,
        ARMED,
        MANUAL,
        STABILIZE,
        AUTO
    };
    
    FlightMode current_mode;
    uint32_t armed_time;
    bool connection_active;
    uint8_t battery_percent;
    
    void drawStatus(display_instance_t* display);
    void drawControls(display_instance_t* display);
    void drawTelemetry(display_instance_t* display);
};

#endif