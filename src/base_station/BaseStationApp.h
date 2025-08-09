#ifndef BASE_STATION_APP_H
#define BASE_STATION_APP_H

#include "../../lib/Core/AppFramework.h"
#include "../../lib/Core/StateManager.h"
#include "../../lib/Core/AppScreen.h"
#include "../../lib/Business/SystemMonitor.h"
#include "../../lib/Business/DisplayController.h"
#include "../../lib/HAL/Board/base_station_bsp.h"
#include "../../lib/Config/base_station_config.h"
#include "screens/StartupScreen.h"
#include "screens/CounterScreen.h"
#include "screens/ESPNowScreen.h"
#include "../../lib/Communication/ESPNow/ESPNowManager.h"
#include "../../lib/Communication/ESPNow/ESPNowUtils.h"

class BaseStationApp : public AppFramework {
public:
    enum class AppState : uint16_t {
        INIT = 0,
        STARTUP_SCREEN,
        ESPNOW_TEST,
        IDLE,
        MONITORING,
        ERROR
    };
    
    BaseStationApp();
    ~BaseStationApp() = default;
    
protected:
    hal_status_t onInitialize() override;
    hal_status_t onStart() override;
    hal_status_t onUpdate(uint32_t delta_ms) override;
    hal_status_t onShutdown() override;
    hal_status_t onError(hal_status_t error) override;
    
private:
    StateMachine<AppState> state_machine;
    SystemMonitor* system_monitor;
    DisplayController* display_controller;
    display_instance_t* lcd_display;
    
    StartupScreen* startup_screen;
    CounterScreen* counter_screen;
    BaseStationESPNowScreen* espnow_screen;
    ESPNowManager* espnow_manager;
    
    // Simple sync state
    uint8_t remote_screen_type;
    uint8_t remote_button_states;
    bool is_synced;
    AppScreen* current_screen;
    
    hal_status_t initHardware();
    hal_status_t initDisplay();
    hal_status_t initStates();
    
    hal_status_t handleInitState(uint32_t delta_ms);
    hal_status_t handleStartupScreen(uint32_t delta_ms);
    hal_status_t handleESPNowTest(uint32_t delta_ms);
    hal_status_t handleIdleState(uint32_t delta_ms);
    hal_status_t handleMonitoringState(uint32_t delta_ms);
    hal_status_t handleErrorState(uint32_t delta_ms);
    
    hal_status_t initESPNow();
    hal_status_t initScreens();
    
    void switchToScreen(AppScreen* screen);
    void updateSyncDisplay();
    void handleScreenSync(uint8_t screenType);
    void handleButtonData(uint8_t buttonStates);
    
    static BaseStationApp* instance;  // For static callbacks
};

#endif