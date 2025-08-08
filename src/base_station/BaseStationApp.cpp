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
    , counter(0) {
}

hal_status_t BaseStationApp::onInitialize() {
    hal_status_t status = initHardware();
    if (status != HAL_OK) return status;
    
    status = initDisplay();
    if (status != HAL_OK) return status;
    
    system_monitor = new SystemMonitor("BaseStation", LED_BUILTIN);
    if (!system_monitor) return HAL_ERROR;
    
    status = system_monitor->init();
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
    
    display_controller = new DisplayController(lcd_display);
    if (!display_controller) return HAL_ERROR;
    
    hal_status_t status = display_controller->init();
    if (status != HAL_OK) return status;
    
    DisplayController::Screen startup_screen = {
        .name = "Startup",
        .onDraw = [this](display_instance_t* d) { drawStartupScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = 1000
    };
    
    DisplayController::Screen status_screen = {
        .name = "Status",
        .onDraw = [this](display_instance_t* d) { drawStatusScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = Constants::Timing::DISPLAY_UPDATE_INTERVAL_MS
    };
    
    display_controller->registerScreen(0, startup_screen);
    display_controller->registerScreen(1, status_screen);
    
    LOG_INFO("BaseStation", "Display initialized");
    return HAL_OK;
}

hal_status_t BaseStationApp::initStates() {
    state_machine.registerState(AppState::INIT, "Init",
        [this](uint32_t dt) { return handleInitState(dt); });
    
    state_machine.registerState(AppState::STARTUP_SCREEN, "Startup",
        [this](uint32_t dt) { return handleStartupScreen(dt); });
    
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
    
    if (display_controller) {
        display_controller->update(delta_ms);
    }
    
    return state_machine.update(delta_ms);
}

hal_status_t BaseStationApp::onShutdown() {
    LOG_INFO("BaseStation", "Shutting down");
    
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
    if (display_controller && display_controller->getCurrentScreen() != 0) {
        display_controller->switchToScreen(0);
    }
    
    if (state_machine.getStateTime() >= Constants::System::STARTUP_SCREEN_DURATION_MS) {
        return state_machine.transitionTo(AppState::IDLE);
    }
    
    return HAL_OK;
}

hal_status_t BaseStationApp::handleIdleState(uint32_t delta_ms) {
    if (display_controller && display_controller->getCurrentScreen() != 1) {
        display_controller->switchToScreen(1);
    }
    
    static uint32_t last_counter_update = 0;
    uint32_t current_time = millis();
    
    if (current_time - last_counter_update >= 1000) {
        counter++;
        updateStatusDisplay();
        last_counter_update = current_time;
    }
    
    return HAL_OK;
}

hal_status_t BaseStationApp::handleMonitoringState(uint32_t delta_ms) {
    return HAL_OK;
}

hal_status_t BaseStationApp::handleErrorState(uint32_t delta_ms) {
    if (display_controller) {
        display_controller->clear();
        display_controller->drawText(0, 0, "ERROR", 2);
        display_controller->refresh();
    }
    
    return HAL_OK;
}

void BaseStationApp::drawStartupScreen(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display_point_t cursor = {0, 0};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "Base Station", 1);
    
    cursor.y = 1;
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "Initializing...", 1);
}

void BaseStationApp::drawStatusScreen(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display_point_t cursor = {0, 0};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "Base Station OK", 1);
    
    char time_str[32];
    DataFormatter::formatUptime(time_str, sizeof(time_str), counter);
    
    cursor.y = 1;
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, time_str, 1);
}

void BaseStationApp::updateStatusDisplay() {
    if (!lcd_display || !lcd_display->interface) return;
    
    char status_str[64];
    snprintf(status_str, sizeof(status_str), "Time: %u s", (unsigned)counter);
    
    display_point_t cursor = {0, 1};
    lcd_display->interface->set_text_cursor(lcd_display, &cursor);
    lcd_display->interface->write_text(lcd_display, status_str, 1);
}