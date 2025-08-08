#include "StartupScreen.h"

StartupScreen::StartupScreen() 
    : AppScreen("Startup")
    , elapsed_time(0) {
}

void StartupScreen::onUpdate(uint32_t delta_ms) {
    elapsed_time += delta_ms;
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