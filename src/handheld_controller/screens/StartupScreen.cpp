#include "StartupScreen.h"
#include "../../../lib/Business/DisplayController.h"
#include "../../../lib/Core/Logger.h"

HandheldStartupScreen::HandheldStartupScreen()
    : AppScreen("Startup")
    , elapsed_time(0) {
}

void HandheldStartupScreen::onUpdate(uint32_t delta_ms) {
    elapsed_time += delta_ms;
    
    // Log progress and force redraw
    static uint32_t last_log = 0;
    last_log += delta_ms;
    if (last_log >= 1000) {
        LOG_INFO("StartupScreen", "Handheld progress: %u/%u ms", elapsed_time, Constants::System::STARTUP_SCREEN_DURATION_MS);
        requestRedraw();
        last_log = 0;
    }
}

void HandheldStartupScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    DisplayController::drawHeader(display, "GoudFlight");
    DisplayController::drawCenteredText(display, 30, "Handheld", 2);
    
    char version_str[32];
    snprintf(version_str, sizeof(version_str), "v%s", Constants::System::VERSION);
    DisplayController::drawFooter(display, version_str);
}