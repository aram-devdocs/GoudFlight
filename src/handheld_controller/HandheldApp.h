#ifndef HANDHELD_APP_H
#define HANDHELD_APP_H

#include "../../lib/Core/AppFramework.h"
#include "../../lib/Core/StateManager.h"
#include "../../lib/Core/AppScreen.h"
#include "../../lib/Business/SystemMonitor.h"
#include "../../lib/Business/DisplayController.h"
#include "../../lib/Business/InputHandler.h"
#include "../../lib/HAL/Board/handheld_bsp.h"
#include "screens/StartupScreen.h"
#include "screens/ButtonTestScreen.h"
#include "screens/MenuScreen.h"
#include "screens/FlightControlScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/ESPNowScreen.h"
#include "../../lib/Communication/ESPNow/ESPNowManager.h"
#include "../../lib/Communication/ESPNow/ESPNowUtils.h"
// Simple screen sync - no complex managers

class HandheldApp : public AppFramework {
public:
    enum class AppState : uint16_t {
        INIT = 0,
        STARTUP_SCREEN,
        ESPNOW_TEST,
        BUTTON_TEST,
        MENU,
        FLIGHT_CONTROL,
        SETTINGS,
        ERROR
    };
    
    HandheldApp();
    ~HandheldApp() = default;
    
protected:
    hal_status_t onInitialize() override;
    hal_status_t onStart() override;
    hal_status_t onUpdate(uint32_t delta_ms) override;
    hal_status_t onShutdown() override;
    hal_status_t onError(hal_status_t error) override;
    
private:
    StateMachine<AppState> state_machine;
    SystemMonitor* system_monitor;
    InputHandler* input_handler;
    
    HandheldStartupScreen* startup_screen;
    HandheldButtonTestScreen* button_test_screen;
    HandheldMenuScreen* menu_screen;
    HandheldFlightControlScreen* flight_screen;
    HandheldSettingsScreen* settings_screen;
    HandheldESPNowScreen* espnow_screen;
    ESPNowManager* espnow_manager;
    // Simple sync
    void sendScreenSync(uint8_t screenType);
    void sendButtonData(uint8_t buttonStates);
    AppScreen* current_screen;
    
    handheld_hardware_t hardware;
    
    hal_status_t initHardware();
    hal_status_t initDisplay();
    hal_status_t initInput();
    hal_status_t initStates();
    hal_status_t initScreens();
    hal_status_t initESPNow();
    
    void switchToScreen(AppScreen* screen);
    void handleScreenInput();
    
    hal_status_t handleInitState(uint32_t delta_ms);
    hal_status_t handleStartupScreen(uint32_t delta_ms);
    hal_status_t handleESPNowTest(uint32_t delta_ms);
    hal_status_t handleButtonTest(uint32_t delta_ms);
    hal_status_t handleMenuState(uint32_t delta_ms);
    hal_status_t handleFlightControl(uint32_t delta_ms);
    hal_status_t handleSettings(uint32_t delta_ms);
    hal_status_t handleErrorState(uint32_t delta_ms);
    
    
    void processButtonEvents();
    void handleButtonPress(uint8_t channel, input_event_t event);
};

#endif