#ifndef APP_FRAMEWORK_H
#define APP_FRAMEWORK_H

#include <Arduino.h>
#include "../HAL/Core/hal_types.h"

enum class AppState {
    UNINITIALIZED,
    INITIALIZING,
    IDLE,
    ACTIVE,
    ERROR,
    SHUTDOWN
};

class AppFramework {
public:
    AppFramework(const char* app_name, hal_board_type_t board_type);
    virtual ~AppFramework() = default;
    
    hal_status_t run();
    
    AppState getState() const { return current_state; }
    const char* getAppName() const { return app_name; }
    uint32_t getUptime() const { return millis() - start_time; }
    
protected:
    virtual hal_status_t onInitialize() = 0;
    virtual hal_status_t onStart() { return HAL_OK; }
    virtual hal_status_t onUpdate(uint32_t delta_ms) = 0;
    virtual hal_status_t onShutdown() { return HAL_OK; }
    virtual hal_status_t onError(hal_status_t error) { return error; }
    
    void setState(AppState new_state);
    hal_status_t transition(AppState new_state);
    
    const char* app_name;
    hal_board_type_t board_type;
    AppState current_state;
    AppState previous_state;
    
private:
    uint32_t start_time;
    uint32_t last_update_time;
    bool initialized;
    
    hal_status_t initialize();
    hal_status_t update();
    hal_status_t shutdown();
};

#endif