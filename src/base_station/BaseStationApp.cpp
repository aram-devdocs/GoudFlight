#include "BaseStationApp.h"
#include "../../lib/Core/Logger.h"
#include "../../lib/Core/Constants.h"
#include "../../lib/Business/DataFormatter.h"
#include "../../lib/SystemInfo/system_info.h"

BaseStationApp* BaseStationApp::instance = nullptr;

BaseStationApp::BaseStationApp()
    : AppFramework("BaseStation", HAL_BOARD_BASE_STATION)
    , state_machine("BaseStationSM")
    , system_monitor(nullptr)
    , display_controller(nullptr)
    , lcd_display(nullptr)
    , startup_screen(nullptr)
    , counter_screen(nullptr)
    , espnow_screen(nullptr)
    , espnow_manager(nullptr)
    , remote_screen_type(0)
    , remote_button_states(0)
    , is_synced(false)
    , current_screen(nullptr) {
    instance = this;
}

hal_status_t BaseStationApp::onInitialize() {
    hal_status_t status = initHardware();
    if (status != HAL_OK) return status;
    
    system_monitor = new SystemMonitor("BaseStation", LED_BUILTIN);
    if (!system_monitor) return HAL_ERROR;
    
    status = system_monitor->init();
    if (status != HAL_OK) return status;
    
    // Initialize ESP-NOW before display so screens can be created
    status = initESPNow();
    if (status != HAL_OK) {
        LOG_WARNING("BaseStation", "ESP-NOW init failed, continuing without it");
    } else {
        // Register simple callbacks for screen sync
        espnow_manager->setScreenSyncCallback([](const ESPNowMessage* msg) {
            if (instance) {
                instance->handleScreenSync(msg->getScreenId());
            }
        });
        
        espnow_manager->setButtonDataCallback([](const ESPNowMessage* msg) {
            if (instance && msg->type == ESPNowConfig::MSG_BUTTON_DATA) {
                // data[0] contains the actual button states
                instance->handleButtonData(msg->data[0]);
            }
        });
    }
    
    status = initDisplay();
    if (status != HAL_OK) return status;
    
    status = initScreens();
    if (status != HAL_OK) return status;
    
    status = initStates();
    if (status != HAL_OK) return status;
    
    return state_machine.transitionTo(AppState::STARTUP_SCREEN);
}

hal_status_t BaseStationApp::initHardware() {
    hal_status_t result = base_station_bsp_init(&base_station_profile.header);
    if (result != HAL_OK) {
        LOG_ERROR("BaseStation", "Failed to initialize BSP: %d", result);
        return result;
    }
    
    LOG_INFO("BaseStation", "Hardware initialized successfully");
    return HAL_OK;
}

hal_status_t BaseStationApp::initDisplay() {
    lcd_display = base_station_bsp_get_display();
    if (!lcd_display || !lcd_display->interface) {
        LOG_WARNING("BaseStation", "No display available");
        return HAL_OK;
    }
    
    startup_screen = new StartupScreen();
    counter_screen = new CounterScreen();
    
    if (!startup_screen || !counter_screen) {
        return HAL_ERROR;
    }
    
    if (espnow_manager) {
        espnow_screen = new BaseStationESPNowScreen(espnow_manager);
        if (espnow_screen) {
            espnow_screen->onInitialize();
        }
    }
    
    hal_status_t status = startup_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    status = counter_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    LOG_INFO("BaseStation", "Display initialized");
    return HAL_OK;
}

hal_status_t BaseStationApp::initStates() {
    state_machine.registerState(AppState::INIT, "Init",
        [this](uint32_t dt) { return handleInitState(dt); });
    
    state_machine.registerState(AppState::STARTUP_SCREEN, "Startup",
        [this](uint32_t dt) { return handleStartupScreen(dt); });
    
    state_machine.registerState(AppState::ESPNOW_TEST, "ESPNow",
        [this](uint32_t dt) { return handleESPNowTest(dt); });
    
    state_machine.registerState(AppState::IDLE, "Idle",
        [this](uint32_t dt) { return handleIdleState(dt); });
    
    state_machine.registerState(AppState::MONITORING, "Monitoring",
        [this](uint32_t dt) { return handleMonitoringState(dt); });
    
    state_machine.registerState(AppState::ERROR, "Error",
        [this](uint32_t dt) { return handleErrorState(dt); });
    
    return HAL_OK;
}

hal_status_t BaseStationApp::onStart() {
    LOG_INFO("BaseStation", "Application started");
    SystemInfo::printSystemInfo("Base Station");
    return HAL_OK;
}

hal_status_t BaseStationApp::onUpdate(uint32_t delta_ms) {
    if (system_monitor) {
        system_monitor->update(delta_ms);
    }
    
    if (espnow_manager) {
        espnow_manager->update(delta_ms);
    }
    
    if (current_screen && current_screen->isActive()) {
        current_screen->onUpdate(delta_ms);
        
        if (current_screen->needsRedraw() && lcd_display && lcd_display->interface) {
            lcd_display->interface->clear(lcd_display);
            current_screen->onDraw(lcd_display);
            current_screen->clearRedrawFlag();
            lcd_display->interface->refresh(lcd_display);
        }
    }
    
    return state_machine.update(delta_ms);
}

hal_status_t BaseStationApp::onShutdown() {
    LOG_INFO("BaseStation", "Shutting down");
    
    if (startup_screen) {
        delete startup_screen;
        startup_screen = nullptr;
    }
    
    if (counter_screen) {
        delete counter_screen;
        counter_screen = nullptr;
    }
    
    if (espnow_screen) {
        delete espnow_screen;
        espnow_screen = nullptr;
    }
    
    // Cleanup removed - no more remote screens
    
    if (espnow_manager) {
        espnow_manager->shutdown();
        delete espnow_manager;
        espnow_manager = nullptr;
    }
    
    if (display_controller) {
        delete display_controller;
        display_controller = nullptr;
    }
    
    if (system_monitor) {
        delete system_monitor;
        system_monitor = nullptr;
    }
    
    base_station_bsp_deinit();
    return HAL_OK;
}

hal_status_t BaseStationApp::onError(hal_status_t error) {
    LOG_ERROR("BaseStation", "Application error: %s", DataFormatter::errorToString(error));
    return state_machine.transitionTo(AppState::ERROR);
}

hal_status_t BaseStationApp::handleInitState(uint32_t delta_ms) {
    return state_machine.transitionTo(AppState::STARTUP_SCREEN);
}

hal_status_t BaseStationApp::handleStartupScreen(uint32_t delta_ms) {
    // First, update the startup screen's timer even if not the current screen
    if (startup_screen) {
        startup_screen->onUpdate(delta_ms);
    }
    
    // Switch to startup screen if needed
    if (current_screen != startup_screen) {
        switchToScreen(startup_screen);
    }
    
    // Check if startup is complete
    if (startup_screen && startup_screen->isComplete()) {
        LOG_INFO("BaseStation", "Startup complete, transitioning to ESP-NOW test");
        return state_machine.transitionTo(AppState::ESPNOW_TEST);
    }
    
    return HAL_OK;
}

hal_status_t BaseStationApp::handleIdleState(uint32_t delta_ms) {
    if (current_screen != counter_screen) {
        switchToScreen(counter_screen);
    }
    
    return HAL_OK;
}

hal_status_t BaseStationApp::handleMonitoringState(uint32_t delta_ms) {
    return HAL_OK;
}

hal_status_t BaseStationApp::handleErrorState(uint32_t delta_ms) {
    if (lcd_display && lcd_display->interface) {
        lcd_display->interface->clear(lcd_display);
        display_point_t cursor = {0, 0};
        lcd_display->interface->set_text_cursor(lcd_display, &cursor);
        lcd_display->interface->write_text(lcd_display, "ERROR", 2);
        lcd_display->interface->refresh(lcd_display);
    }
    
    return HAL_OK;
}

void BaseStationApp::switchToScreen(AppScreen* screen) {
    if (!screen) return;
    
    // Don't switch if already on this screen
    if (current_screen == screen) return;
    
    LOG_DEBUG("BaseStation", "Switching from %s to %s", 
              current_screen ? current_screen->getName() : "null",
              screen->getName());
    
    if (current_screen) {
        current_screen->onExit();
    }
    
    // Clear display before switching
    if (lcd_display && lcd_display->interface) {
        lcd_display->interface->clear(lcd_display);
        // Small delay to ensure clear completes
        delay(5);
    }
    
    current_screen = screen;
    current_screen->onEnter();
    
    // Force immediate redraw of new screen
    if (lcd_display && lcd_display->interface && current_screen) {
        current_screen->onDraw(lcd_display);
        current_screen->clearRedrawFlag();
        lcd_display->interface->refresh(lcd_display);
    }
}

hal_status_t BaseStationApp::initESPNow() {
    const char* handheld_mac_str = "30:ED:A0:A8:B5:70";
    uint8_t handheld_mac[6];
    
    if (!ESPNowUtils::parseMacAddress(handheld_mac_str, handheld_mac)) {
        LOG_ERROR("BaseStation", "Failed to parse handheld MAC address");
        return HAL_ERROR;
    }
    
    espnow_manager = new ESPNowManager(ESPNowConfig::ROLE_BASE_STATION, handheld_mac);
    if (!espnow_manager) {
        LOG_ERROR("BaseStation", "Failed to create ESP-NOW manager");
        return HAL_ERROR;
    }
    
    hal_status_t status = espnow_manager->init();
    if (status != HAL_OK) {
        LOG_ERROR("BaseStation", "Failed to initialize ESP-NOW");
        delete espnow_manager;
        espnow_manager = nullptr;
        return status;
    }
    
    LOG_INFO("BaseStation", "ESP-NOW initialized");
    return HAL_OK;
}

hal_status_t BaseStationApp::handleESPNowTest(uint32_t delta_ms) {
    // When first paired, disable normal screen updates
    if (is_synced && espnow_manager && espnow_manager->isPaired()) {
        // Disable normal screen system when synced
        if (current_screen != nullptr) {
            current_screen = nullptr;
            LOG_INFO("BaseStation", "Sync mode active - disabled screen system");
        }
        // Display updates only happen when handleScreenSync or handleButtonData are called
    } else if (espnow_screen && current_screen != espnow_screen) {
        // Show ESP-NOW screen when not synced
        LOG_INFO("BaseStation", "Showing ESP-NOW screen");
        switchToScreen(espnow_screen);
        is_synced = false; // Reset sync state if disconnected
    }
    
    return HAL_OK;
}

hal_status_t BaseStationApp::initScreens() {
    // No remote screens needed anymore
    return HAL_OK;
}

void BaseStationApp::handleScreenSync(uint8_t screenType) {
    // Only update if screen actually changed
    if (remote_screen_type != screenType || !is_synced) {
        LOG_INFO("BaseStation", "Screen sync: type=%d", screenType);
        remote_screen_type = screenType;
        
        // First sync needs extra clear
        if (!is_synced) {
            is_synced = true;
            // Double clear on first sync to ensure clean display
            if (lcd_display && lcd_display->interface) {
                lcd_display->interface->clear(lcd_display);
                delay(20);
            }
        }
        
        updateSyncDisplay();
    }
}

void BaseStationApp::handleButtonData(uint8_t buttonStates) {
    // Only update if button states actually changed
    if (remote_button_states != buttonStates) {
        remote_button_states = buttonStates;
        // Only update display if we're showing button test
        if (remote_screen_type == 3) { // BUTTON_TEST = 3
            updateSyncDisplay();
        }
    }
}

void BaseStationApp::updateSyncDisplay() {
    if (!lcd_display || !lcd_display->interface) return;
    if (!is_synced) return;
    
    LOG_DEBUG("BaseStation", "Updating sync display for screen %d", remote_screen_type);
    
    // Simple, direct LCD update - no complex screen switching
    lcd_display->interface->clear(lcd_display);
    delay(10); // Give LCD time to process clear
    
    // Line 0: Show what screen handheld is on
    display_point_t cursor = {0, 0};
    lcd_display->interface->set_text_cursor(lcd_display, &cursor);
    
    const char* screen_names[] = {
        "None", "Starting", "Menu", "Button Test", 
        "Flight", "Settings", "ESP-NOW", "System"
    };
    
    char buffer[17];
    if (remote_screen_type < 8) {
        snprintf(buffer, sizeof(buffer), "HH: %s", screen_names[remote_screen_type]);
    } else {
        snprintf(buffer, sizeof(buffer), "HH: Screen %d", remote_screen_type);
    }
    lcd_display->interface->write_text(lcd_display, buffer, 1);
    
    // Line 1: Show button states if on button test, otherwise show connected
    cursor.y = 1;
    lcd_display->interface->set_text_cursor(lcd_display, &cursor);
    
    if (remote_screen_type == 3) { // BUTTON_TEST
        // Show button states as binary (0/1 for each button)
        buffer[0] = '0' + ((remote_button_states >> 0) & 1);
        buffer[1] = ' ';
        buffer[2] = '0' + ((remote_button_states >> 1) & 1);
        buffer[3] = ' ';
        buffer[4] = '0' + ((remote_button_states >> 2) & 1);
        buffer[5] = ' ';
        buffer[6] = '0' + ((remote_button_states >> 3) & 1);
        buffer[7] = ' ';
        buffer[8] = '0' + ((remote_button_states >> 4) & 1);
        buffer[9] = ' ';
        buffer[10] = '0' + ((remote_button_states >> 5) & 1);
        buffer[11] = ' ';
        buffer[12] = '0' + ((remote_button_states >> 6) & 1);
        buffer[13] = ' ';
        buffer[14] = '0' + ((remote_button_states >> 7) & 1);
        buffer[15] = '\0';
        lcd_display->interface->write_text(lcd_display, buffer, 1);
    } else {
        lcd_display->interface->write_text(lcd_display, "Connected", 1);
    }
    
    lcd_display->interface->refresh(lcd_display);
}