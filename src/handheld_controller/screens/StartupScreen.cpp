#include "StartupScreen.h"
#include "../../../lib/Business/DisplayController.h"

HandheldStartupScreen::HandheldStartupScreen()
    : AppScreen("Startup")
    , elapsed_time(0) {
}

void HandheldStartupScreen::onUpdate(uint32_t delta_ms) {
    elapsed_time += delta_ms;
}

void HandheldStartupScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    DisplayController::drawHeader(display, "GoudFlight");
    DisplayController::drawCenteredText(display, 30, "Handheld", 2);
    
    char version_str[32];
    snprintf(version_str, sizeof(version_str), "v%s", Constants::System::VERSION);
    DisplayController::drawFooter(display, version_str);
}