#include "FlightControlScreen.h"
#include "../../../lib/Business/DisplayController.h"
#include "../../../lib/Core/Logger.h"
#include <Arduino.h>

HandheldFlightControlScreen::HandheldFlightControlScreen()
    : AppScreen("FlightControl")
    , current_mode(FlightMode::DISARMED)
    , armed_time(0)
    , connection_active(false)
    , battery_percent(100) {
}

void HandheldFlightControlScreen::onEnter() {
    AppScreen::onEnter();
    current_mode = FlightMode::DISARMED;
    armed_time = 0;
    LOG_INFO("Flight", "Flight control screen entered");
}

void HandheldFlightControlScreen::onUpdate(uint32_t delta_ms) {
    if (current_mode != FlightMode::DISARMED) {
        armed_time += delta_ms;
        if (armed_time % 1000 < delta_ms) {
            requestRedraw();
        }
    }
}

void HandheldFlightControlScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display->interface->clear(display);
    drawStatus(display);
    drawControls(display);
    drawTelemetry(display);
    display->interface->refresh(display);
}

void HandheldFlightControlScreen::onButtonPress(uint8_t button_id) {
    switch (button_id) {
        case 0:
            if (current_mode == FlightMode::DISARMED) {
                current_mode = FlightMode::ARMED;
                armed_time = 0;
                LOG_INFO("Flight", "Armed");
            } else {
                current_mode = FlightMode::DISARMED;
                LOG_INFO("Flight", "Disarmed");
            }
            requestRedraw();
            break;
            
        case 1:
            if (current_mode == FlightMode::ARMED) {
                current_mode = FlightMode::MANUAL;
                LOG_INFO("Flight", "Manual mode");
                requestRedraw();
            }
            break;
            
        case 2:
            if (current_mode == FlightMode::ARMED || current_mode == FlightMode::MANUAL) {
                current_mode = FlightMode::STABILIZE;
                LOG_INFO("Flight", "Stabilize mode");
                requestRedraw();
            }
            break;
    }
}

void HandheldFlightControlScreen::drawStatus(display_instance_t* display) {
    const char* mode_str = "DISARMED";
    switch (current_mode) {
        case FlightMode::ARMED: mode_str = "ARMED"; break;
        case FlightMode::MANUAL: mode_str = "MANUAL"; break;
        case FlightMode::STABILIZE: mode_str = "STABLE"; break;
        case FlightMode::AUTO: mode_str = "AUTO"; break;
        default: break;
    }
    
    DisplayController::drawHeader(display, mode_str);
}

void HandheldFlightControlScreen::drawControls(display_instance_t* display) {
    DisplayController::drawCenteredText(display, 25, 
        current_mode == FlightMode::DISARMED ? "Press 1 to ARM" : "Press 1 to DISARM", 1);
}

void HandheldFlightControlScreen::drawTelemetry(display_instance_t* display) {
    char telemetry_str[32];
    snprintf(telemetry_str, sizeof(telemetry_str), "Batt: %d%%", battery_percent);
    DisplayController::drawFooter(display, telemetry_str);
}