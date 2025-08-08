#include "AppFramework.h"
#include "Logger.h"

AppFramework::AppFramework(const char* app_name, hal_board_type_t board_type)
    : app_name(app_name)
    , board_type(board_type)
    , current_state(AppState::UNINITIALIZED)
    , previous_state(AppState::UNINITIALIZED)
    , start_time(0)
    , last_update_time(0)
    , initialized(false) {
}

hal_status_t AppFramework::run() {
    if (!initialized) {
        hal_status_t status = initialize();
        if (status != HAL_OK) {
            return status;
        }
    }
    
    return update();
}

hal_status_t AppFramework::initialize() {
    if (initialized) {
        return HAL_OK;
    }
    
    setState(AppState::INITIALIZING);
    LOG_INFO(app_name, "Initializing application");
    
    start_time = millis();
    last_update_time = start_time;
    
    hal_status_t status = onInitialize();
    if (status != HAL_OK) {
        LOG_ERROR(app_name, "Initialization failed: %d", status);
        setState(AppState::ERROR);
        return onError(status);
    }
    
    status = onStart();
    if (status != HAL_OK) {
        LOG_ERROR(app_name, "Start failed: %d", status);
        setState(AppState::ERROR);
        return onError(status);
    }
    
    initialized = true;
    setState(AppState::IDLE);
    LOG_INFO(app_name, "Application initialized successfully");
    
    return HAL_OK;
}

hal_status_t AppFramework::update() {
    uint32_t current_time = millis();
    uint32_t delta_ms = current_time - last_update_time;
    last_update_time = current_time;
    
    if (current_state == AppState::ERROR) {
        return HAL_ERROR;
    }
    
    hal_status_t status = onUpdate(delta_ms);
    if (status != HAL_OK) {
        LOG_ERROR(app_name, "Update failed: %d", status);
        setState(AppState::ERROR);
        return onError(status);
    }
    
    return HAL_OK;
}

hal_status_t AppFramework::shutdown() {
    LOG_INFO(app_name, "Shutting down application");
    setState(AppState::SHUTDOWN);
    
    hal_status_t status = onShutdown();
    if (status != HAL_OK) {
        LOG_ERROR(app_name, "Shutdown failed: %d", status);
    }
    
    initialized = false;
    return status;
}

void AppFramework::setState(AppState new_state) {
    if (current_state != new_state) {
        previous_state = current_state;
        current_state = new_state;
        LOG_DEBUG(app_name, "State transition: %d -> %d", 
                  static_cast<int>(previous_state), 
                  static_cast<int>(new_state));
    }
}

hal_status_t AppFramework::transition(AppState new_state) {
    setState(new_state);
    return HAL_OK;
}