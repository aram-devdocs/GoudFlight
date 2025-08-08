#include "StartupScreen.h"
#include "../../../lib/Core/Logger.h"

StartupScreen::StartupScreen() 
    : AppScreen("Startup")
    , elapsed_time(0) {
}

void StartupScreen::onUpdate(uint32_t delta_ms) {
    elapsed_time += delta_ms;
    
    // Log progress
    static uint32_t last_log = 0;
    last_log += delta_ms;
    if (last_log >= 1000) {
        LOG_INFO("StartupScreen", "Progress: %u/%u ms", elapsed_time, Constants::System::STARTUP_SCREEN_DURATION_MS);
        last_log = 0;
    }
    
    // Request redraw periodically to ensure display updates
    static uint32_t last_redraw = 0;
    last_redraw += delta_ms;
    if (last_redraw >= 500) {  // Redraw every 500ms
        requestRedraw();
        last_redraw = 0;
    }
}

void StartupScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display_point_t cursor = {0, 0};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "Base Station", 1);
    
    cursor.y = 1;
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "Initializing...", 1);
}