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
    DisplayController::drawHeader(display, "ESP-NOW");
    
    // Draw connection status
    if (espnow_manager) {
        const char* status = "?";
        ESPNowManager::State state = espnow_manager->getState();
        
        switch (state) {
            case ESPNowManager::State::UNINITIALIZED:
                status = "NOT CONNECTED";
                break;
            case ESPNowManager::State::SEARCHING:
                status = "SEARCHING...";
                break;
            case ESPNowManager::State::PAIRING:
                status = "PAIRING...";
                break;
            case ESPNowManager::State::PAIRED:
                status = "CONNECTED";
                break;
            case ESPNowManager::State::RECONNECTING:
                status = "RECONNECTING...";
                break;
            case ESPNowManager::State::ERROR:
                status = "ERROR";
                break;
        }
        DisplayController::drawCenteredText(display, 25, status, 1);
        
        // Draw button hints based on state
        char hint[64];
        if (state == ESPNowManager::State::UNINITIALIZED) {
            snprintf(hint, sizeof(hint), "[2] Connect");
        } else if (state == ESPNowManager::State::PAIRED) {
            const ESPNowManager::Stats& stats = espnow_manager->getStats();
            snprintf(hint, sizeof(hint), "P:%u L:%ums", 
                     stats.ping_count + stats.pong_count, stats.latency_ms);
            DisplayController::drawCenteredText(display, 40, hint, 1);
            snprintf(hint, sizeof(hint), "[3] Disconnect");
        } else if (state == ESPNowManager::State::SEARCHING || 
                   state == ESPNowManager::State::PAIRING ||
                   state == ESPNowManager::State::RECONNECTING) {
            snprintf(hint, sizeof(hint), "[3] Cancel");
        }
        
        // Draw the hint at the bottom
        DisplayController::drawFooter(display, hint);
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

void HandheldESPNowScreen::onButtonPress(uint8_t button_id) {
    if (!espnow_manager) return;
    
    ESPNowManager::State state = espnow_manager->getState();
    
    switch (button_id) {
        case 2:  // Button 2 - Connect
            if (state == ESPNowManager::State::UNINITIALIZED) {
                LOG_INFO("ESPNowScreen", "User pressed Connect");
                espnow_manager->startConnection();
                requestRedraw();
            }
            break;
            
        case 3:  // Button 3 - Disconnect/Cancel
            if (state == ESPNowManager::State::PAIRED) {
                LOG_INFO("ESPNowScreen", "User pressed Disconnect");
                espnow_manager->disconnect();
                espnow_manager->stopConnection();  // Go back to UNINITIALIZED
                requestRedraw();
            } else if (state == ESPNowManager::State::SEARCHING || 
                       state == ESPNowManager::State::PAIRING ||
                       state == ESPNowManager::State::RECONNECTING) {
                LOG_INFO("ESPNowScreen", "User pressed Cancel");
                espnow_manager->stopConnection();
                requestRedraw();
            }
            break;
            
        case 7:  // Button 7 - Back (handled by HandheldApp)
            // Do nothing here, let the app handle navigation
            break;
            
        default:
            break;
    }
}