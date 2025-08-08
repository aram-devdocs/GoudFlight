#include "ESPNowScreen.h"
#include "../../../lib/Core/Logger.h"
#include "../../../lib/HAL/Display/display_interface.h"
#include <stdio.h>

BaseStationESPNowScreen::BaseStationESPNowScreen(ESPNowManager* manager)
    : AppScreen("ESPNow")
    , espnow_manager(manager)
    , update_timer(0)
    , animation_frame(0) {
}

hal_status_t BaseStationESPNowScreen::onInitialize() {
    LOG_INFO("ESPNowScreen", "Initializing Base Station ESP-NOW screen");
    return AppScreen::onInitialize();
}

void BaseStationESPNowScreen::onEnter() {
    AppScreen::onEnter();
    LOG_INFO("ESPNowScreen", "Entering ESP-NOW test screen");
    update_timer = 0;
    animation_frame = 0;
    requestRedraw();  // Force immediate redraw when entering screen
}

void BaseStationESPNowScreen::onUpdate(uint32_t delta_ms) {
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
        LOG_INFO("ESPNowScreen", "State: %s", espnow_manager->getStateString());
        log_timer = 0;
    }
}

void BaseStationESPNowScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    // Force clear the display
    if (display->interface->clear) {
        display->interface->clear(display);
    }
    
    // Line 0: Show status
    drawConnectionStatus(display);
    
    // Line 1: Show stats or animation
    if (espnow_manager && espnow_manager->isPaired()) {
        drawStatistics(display);
    } else if (espnow_manager && espnow_manager->getState() == ESPNowManager::State::SEARCHING) {
        drawSearchingAnimation(display);
    }
    
    if (display->interface->refresh) {
        display->interface->refresh(display);
    }
}

void BaseStationESPNowScreen::drawHeader(display_instance_t* display) {
    if (!display->interface->set_text_cursor || !display->interface->write_text) return;
    
    display_point_t cursor = {0, 0};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "ESP-NOW", 1);
}

void BaseStationESPNowScreen::drawMacAddresses(display_instance_t* display) {
    if (!display->interface->set_text_cursor || !display->interface->write_text) return;
    
    char buffer[32];
    uint8_t mac[6];
    
    if (espnow_manager) {
        espnow_manager->getMacAddress(mac);
        display_point_t cursor = {0, 1};
        display->interface->set_text_cursor(display, &cursor);
        snprintf(buffer, sizeof(buffer), "%02X%02X%02X%02X", 
                 mac[2], mac[3], mac[4], mac[5]);
        display->interface->write_text(display, buffer, 1);
    }
}

void BaseStationESPNowScreen::drawConnectionStatus(display_instance_t* display) {
    if (!display->interface->set_text_cursor || !display->interface->write_text) return;
    
    // Line 0: Status
    display_point_t cursor = {0, 0};
    display->interface->set_text_cursor(display, &cursor);
    
    if (!espnow_manager) {
        display->interface->write_text(display, "NO MANAGER!", 1);
        return;
    }
    
    const char* status = "???";
    switch (espnow_manager->getState()) {
        case ESPNowManager::State::UNINITIALIZED:
            status = "ESP-NOW: INIT";
            break;
        case ESPNowManager::State::SEARCHING:
            status = "ESP-NOW: SEARCH";
            break;
        case ESPNowManager::State::PAIRING:
            status = "ESP-NOW: PAIRING";
            break;
        case ESPNowManager::State::PAIRED:
            status = "ESP-NOW: PAIRED!";
            break;
        case ESPNowManager::State::RECONNECTING:
            status = "ESP-NOW: RECONN";
            break;
        case ESPNowManager::State::ERROR:
            status = "ESP-NOW: ERROR";
            break;
    }
    
    display->interface->write_text(display, status, 1);
}

void BaseStationESPNowScreen::drawStatistics(display_instance_t* display) {
    if (!display->interface->set_text_cursor || !display->interface->write_text) return;
    if (!espnow_manager) return;
    
    const ESPNowManager::Stats& stats = espnow_manager->getStats();
    char buffer[17]; // 16 chars + null for LCD
    
    // Line 1: Compact stats
    display_point_t cursor = {0, 1};
    display->interface->set_text_cursor(display, &cursor);
    snprintf(buffer, sizeof(buffer), "P:%u/%u L:%ums",
             stats.ping_count, stats.pong_count, stats.latency_ms);
    display->interface->write_text(display, buffer, 1);
}

void BaseStationESPNowScreen::drawSearchingAnimation(display_instance_t* display) {
    if (!display->interface->set_text_cursor || !display->interface->write_text) return;
    
    // Line 1: Animation
    display_point_t cursor = {0, 1};
    display->interface->set_text_cursor(display, &cursor);
    
    const char* frames[] = { 
        "Scanning.      ",
        "Scanning..     ",
        "Scanning...    "
    };
    
    display->interface->write_text(display, frames[animation_frame % 3], 1);
}