#include "BaseStationApp.h"
#include "../../lib/Core/Logger.h"
#include "../../lib/Core/Constants.h"
#include "../../lib/Business/DataFormatter.h"
#include "../../lib/SystemInfo/system_info.h"

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
    , current_screen(nullptr) {
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
    }
    
    status = initDisplay();
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
    if (current_screen != startup_screen) {
        switchToScreen(startup_screen);
    }
    
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
    
    if (current_screen && current_screen != screen) {
        current_screen->onExit();
    }
    
    current_screen = screen;
    current_screen->onEnter();
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
    if (espnow_screen && current_screen != espnow_screen) {
        LOG_INFO("BaseStation", "Switching to ESP-NOW screen");
        switchToScreen(espnow_screen);
    }
    
    return HAL_OK;
}