#include "ESPNowScreen.h"
#include "../../../lib/Core/Logger.h"
#include "../../../lib/Business/DisplayController.h"
#include <stdio.h>

HandheldESPNowScreen::HandheldESPNowScreen(ESPNowManager* manager)
    : AppScreen("ESPNow")
    , espnow_manager(manager)
    , update_timer(0)
    , animation_frame(0) {
}

hal_status_t HandheldESPNowScreen::onInitialize() {
    LOG_INFO("ESPNowScreen", "Initializing Handheld ESP-NOW screen");
    return AppScreen::onInitialize();
}

void HandheldESPNowScreen::onEnter() {
    AppScreen::onEnter();
    LOG_INFO("ESPNowScreen", "Entering ESP-NOW test screen");
    update_timer = 0;
    animation_frame = 0;
    requestRedraw();  // Force immediate redraw when entering screen
}

void HandheldESPNowScreen::onUpdate(uint32_t delta_ms) {
    update_timer += delta_ms;
    
    // Only request redraw every 500ms to reduce flicker
    if (update_timer >= 500) {
        animation_frame++;
        requestRedraw();
        update_timer = 0;
    }
    
    // Log state periodically
    static uint32_t log_timer = 0;
    log_timer += delta_ms;
    if (log_timer >= 2000 && espnow_manager) {
        LOG_INFO("ESPNowScreen", "Handheld State: %s", espnow_manager->getStateString());
        log_timer = 0;
    }
}

void HandheldESPNowScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display->interface->clear(display);
    
    // Draw header
    DisplayController::drawHeader(display, "ESP-NOW TEST");
    
    // Draw connection status
    if (espnow_manager) {
        const char* status = "?";
        switch (espnow_manager->getState()) {
            case ESPNowManager::State::SEARCHING:
                status = "SEARCHING";
                break;
            case ESPNowManager::State::PAIRING:
                status = "PAIRING";
                break;
            case ESPNowManager::State::PAIRED:
                status = "CONNECTED";
                break;
            case ESPNowManager::State::RECONNECTING:
                status = "RECONNECTING";
                break;
            case ESPNowManager::State::ERROR:
                status = "ERROR";
                break;
            default:
                status = "INIT";
                break;
        }
        DisplayController::drawCenteredText(display, 30, status, 2);
        
        // Draw stats if connected
        if (espnow_manager->getState() == ESPNowManager::State::PAIRED) {
            const ESPNowManager::Stats& stats = espnow_manager->getStats();
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "P:%u L:%ums", 
                     stats.ping_count + stats.pong_count, stats.latency_ms);
            DisplayController::drawFooter(display, buffer);
        }
    } else {
        DisplayController::drawCenteredText(display, 30, "NO MANAGER", 1);
    }
    
    display->interface->refresh(display);
}

// Helper draw methods removed - using DisplayController instead
void HandheldESPNowScreen::drawHeader(display_instance_t* display) {
    // Not used anymore
}

void HandheldESPNowScreen::drawMacAddress(display_instance_t* display) {
    // Not used anymore
}

void HandheldESPNowScreen::drawConnectionStatus(display_instance_t* display) {
    // Not used anymore
}

void HandheldESPNowScreen::drawStatistics(display_instance_t* display) {
    // Not used anymore
}

void HandheldESPNowScreen::drawSearchingAnimation(display_instance_t* display) {
    // Not used anymore
}

void HandheldESPNowScreen::drawPairingAnimation(display_instance_t* display) {
    // Not used anymore
}