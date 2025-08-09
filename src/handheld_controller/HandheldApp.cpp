#include "HandheldApp.h"
#include "../../lib/Core/Logger.h"
#include "../../lib/Core/Constants.h"
#include "../../lib/Business/DataFormatter.h"
#include "../../lib/SystemInfo/system_info.h"
#include "../../lib/HAL/Core/hal_errors.h"

HandheldApp::HandheldApp()
    : AppFramework("Handheld", HAL_BOARD_HANDHELD)
    , state_machine("HandheldSM")
    , system_monitor(nullptr)
    , input_handler(nullptr)
    , startup_screen(nullptr)
    , button_test_screen(nullptr)
    , menu_screen(nullptr)
    , flight_screen(nullptr)
    , settings_screen(nullptr)
    , espnow_screen(nullptr)
    , espnow_manager(nullptr)
    , current_screen(nullptr)
    , hardware({nullptr, nullptr}) {
}

hal_status_t HandheldApp::onInitialize() {
    hal_status_t status = initHardware();
    if (status != HAL_OK) return status;
    
    system_monitor = new SystemMonitor("Handheld", LED_BUILTIN);
    if (!system_monitor) return HAL_ERROR;
    status = system_monitor->init();
    if (status != HAL_OK) return status;
    
    status = initInput();
    if (status != HAL_OK) return status;
    
    // Initialize ESP-NOW before screens so ESP-NOW screen can be created
    status = initESPNow();
    if (status != HAL_OK) {
        LOG_WARNING("Handheld", "ESP-NOW init failed, continuing without it");
    }
    
    status = initScreens();
    if (status != HAL_OK) return status;
    
    status = initStates();
    if (status != HAL_OK) return status;
    
    return state_machine.transitionTo(AppState::STARTUP_SCREEN);
}

hal_status_t HandheldApp::initHardware() {
    hal_status_t status = handheld_bsp_init(&hardware);
    if (status != HAL_OK) {
        LOG_ERROR("Handheld", "BSP initialization failed: %s", hal_get_error_string(status));
        
        char error_msg[128];
        handheld_get_last_error(error_msg, sizeof(error_msg));
        LOG_ERROR("Handheld", "Details: %s", error_msg);
        return status;
    }
    
    handheld_log_system_info();
    LOG_INFO("Handheld", "Hardware initialized successfully");
    return HAL_OK;
}

hal_status_t HandheldApp::initScreens() {
    if (!hardware.display || !hardware.display->interface) {
        LOG_WARNING("Handheld", "No display available for screens");
        return HAL_OK;
    }
    
    startup_screen = new HandheldStartupScreen();
    button_test_screen = new HandheldButtonTestScreen();
    menu_screen = new HandheldMenuScreen();
    flight_screen = new HandheldFlightControlScreen();
    settings_screen = new HandheldSettingsScreen();
    
    if (!startup_screen || !button_test_screen || !menu_screen || 
        !flight_screen || !settings_screen) {
        return HAL_ERROR;
    }
    
    hal_status_t status = startup_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    status = button_test_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    status = menu_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    status = flight_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    status = settings_screen->onInitialize();
    if (status != HAL_OK) return status;
    
    if (espnow_manager) {
        espnow_screen = new HandheldESPNowScreen(espnow_manager);
        if (espnow_screen) {
            espnow_screen->onInitialize();
        }
    }
    
    LOG_INFO("Handheld", "Screens initialized");
    return HAL_OK;
}

hal_status_t HandheldApp::initInput() {
    if (!hardware.input || !hardware.input->interface) {
        LOG_WARNING("Handheld", "No input available");
        return HAL_OK;
    }
    
    input_handler = new InputHandler(hardware.input);
    if (!input_handler) return HAL_ERROR;
    
    hal_status_t status = input_handler->init();
    if (status != HAL_OK) return status;
    
    LOG_INFO("Handheld", "Input initialized with %u channels", 
             hardware.input->interface->get_channel_count(hardware.input));
    return HAL_OK;
}

hal_status_t HandheldApp::initStates() {
    state_machine.registerState(AppState::INIT, "Init",
        [this](uint32_t dt) { return handleInitState(dt); });
    
    state_machine.registerState(AppState::STARTUP_SCREEN, "Startup",
        [this](uint32_t dt) { return handleStartupScreen(dt); });
    
    state_machine.registerState(AppState::ESPNOW_TEST, "ESPNow",
        [this](uint32_t dt) { return handleESPNowTest(dt); });
    
    state_machine.registerState(AppState::BUTTON_TEST, "ButtonTest",
        [this](uint32_t dt) { return handleButtonTest(dt); });
    
    state_machine.registerState(AppState::MENU, "Menu",
        [this](uint32_t dt) { return handleMenuState(dt); });
    
    state_machine.registerState(AppState::FLIGHT_CONTROL, "FlightControl",
        [this](uint32_t dt) { return handleFlightControl(dt); });
    
    state_machine.registerState(AppState::SETTINGS, "Settings",
        [this](uint32_t dt) { return handleSettings(dt); });
    
    state_machine.registerState(AppState::ERROR, "Error",
        [this](uint32_t dt) { return handleErrorState(dt); });
    
    return HAL_OK;
}

hal_status_t HandheldApp::onStart() {
    LOG_INFO("Handheld", "Application started");
    SystemInfo::printSystemInfo("Handheld Controller");
    return HAL_OK;
}

hal_status_t HandheldApp::onUpdate(uint32_t delta_ms) {
    if (system_monitor) {
        system_monitor->update(delta_ms);
    }
    
    if (espnow_manager) {
        espnow_manager->update(delta_ms);
    }
    
    if (input_handler) {
        input_handler->update(delta_ms);
    }
    
    handleScreenInput();
    
    if (current_screen && current_screen->isActive()) {
        current_screen->onUpdate(delta_ms);
        
        if (current_screen->needsRedraw() && hardware.display) {
            current_screen->onDraw(hardware.display);
            current_screen->clearRedrawFlag();
            // Note: For OLED, the display() call is done inside onDraw
        }
    }
    
    return state_machine.update(delta_ms);
}

hal_status_t HandheldApp::onShutdown() {
    LOG_INFO("Handheld", "Shutting down");
    
    delete startup_screen;
    delete button_test_screen;
    delete menu_screen;
    delete flight_screen;
    delete settings_screen;
    delete espnow_screen;
    
    // No managers to delete
    
    if (espnow_manager) {
        espnow_manager->shutdown();
        delete espnow_manager;
    }
    
    delete input_handler;
    delete system_monitor;
    
    handheld_bsp_deinit(&hardware);
    return HAL_OK;
}

hal_status_t HandheldApp::onError(hal_status_t error) {
    LOG_ERROR("Handheld", "Application error: %s", DataFormatter::errorToString(error));
    return state_machine.transitionTo(AppState::ERROR);
}

hal_status_t HandheldApp::handleInitState(uint32_t delta_ms) {
    return state_machine.transitionTo(AppState::STARTUP_SCREEN);
}

hal_status_t HandheldApp::handleStartupScreen(uint32_t delta_ms) {
    if (current_screen != startup_screen) {
        switchToScreen(startup_screen);
    }
    
    if (startup_screen && startup_screen->isComplete()) {
        LOG_INFO("Handheld", "Startup complete, transitioning to menu");
        return state_machine.transitionTo(AppState::MENU);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleButtonTest(uint32_t delta_ms) {
    if (current_screen != button_test_screen) {
        switchToScreen(button_test_screen);
    }
    
    if (input_handler && input_handler->isPressed(7)) {
        return state_machine.transitionTo(AppState::MENU);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleMenuState(uint32_t delta_ms) {
    if (current_screen != menu_screen) {
        switchToScreen(menu_screen);
    }
    
    if (menu_screen && menu_screen->getMenu() && input_handler && input_handler->isPressed(2)) {
        int selected = menu_screen->getMenu()->getCurrentItem();
        switch (selected) {
            case 0:
                return state_machine.transitionTo(AppState::FLIGHT_CONTROL);
            case 1:
                return state_machine.transitionTo(AppState::ESPNOW_TEST);
            case 2:
                return state_machine.transitionTo(AppState::BUTTON_TEST);
            case 3:
                return state_machine.transitionTo(AppState::SETTINGS);
            // case 4 would be System Info - not implemented yet
        }
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleFlightControl(uint32_t delta_ms) {
    if (current_screen != flight_screen) {
        switchToScreen(flight_screen);
    }
    
    if (input_handler && input_handler->isPressed(7)) {
        return state_machine.transitionTo(AppState::MENU);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleSettings(uint32_t delta_ms) {
    if (current_screen != settings_screen) {
        switchToScreen(settings_screen);
    }
    
    if (input_handler && input_handler->isPressed(7)) {
        return state_machine.transitionTo(AppState::MENU);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleErrorState(uint32_t delta_ms) {
    if (hardware.display && hardware.display->interface) {
        hardware.display->interface->clear(hardware.display);
        DisplayController::drawCenteredText(hardware.display, 30, "ERROR", 2);
        hardware.display->interface->refresh(hardware.display);
    }
    
    return HAL_OK;
}

void HandheldApp::switchToScreen(AppScreen* screen) {
    if (!screen) return;
    
    if (current_screen && current_screen != screen) {
        current_screen->onExit();
    }
    
    current_screen = screen;
    current_screen->onEnter();
    
    // Send simple screen sync
    uint8_t screenType = 0; // NONE
    if (screen == startup_screen) screenType = 1; // STARTUP
    else if (screen == menu_screen) screenType = 2; // MENU
    else if (screen == button_test_screen) screenType = 3; // BUTTON_TEST
    else if (screen == flight_screen) screenType = 4; // FLIGHT_CONTROL
    else if (screen == settings_screen) screenType = 5; // SETTINGS
    else if (screen == espnow_screen) screenType = 6; // ESPNOW_STATUS
    
    sendScreenSync(screenType);
}

void HandheldApp::handleScreenInput() {
    if (!current_screen || !input_handler) return;
    
    if (hardware.input) {
        current_screen->onInput(hardware.input);
    }
    
    static uint8_t prev_button_states = 0;
    uint8_t current_button_states = 0;
    
    for (uint8_t i = 0; i < 8; i++) {
        if (input_handler->isPressed(i)) {
            current_button_states |= (1 << i);
            
            if (!(prev_button_states & (1 << i))) {
                current_screen->onButtonPress(i);
            }
        } else {
            if (prev_button_states & (1 << i)) {
                current_screen->onButtonRelease(i);
            }
        }
    }
    
    // Send button states to base station if on button test screen
    if (current_screen == button_test_screen && current_button_states != prev_button_states) {
        sendButtonData(current_button_states);
    }
    
    prev_button_states = current_button_states;
}

hal_status_t HandheldApp::initESPNow() {
    const char* base_mac_str = "30:ED:A0:A8:A8:28";
    uint8_t base_mac[6];
    
    if (!ESPNowUtils::parseMacAddress(base_mac_str, base_mac)) {
        LOG_ERROR("Handheld", "Failed to parse base station MAC address");
        return HAL_ERROR;
    }
    
    espnow_manager = new ESPNowManager(ESPNowConfig::ROLE_HANDHELD, base_mac);
    if (!espnow_manager) {
        LOG_ERROR("Handheld", "Failed to create ESP-NOW manager");
        return HAL_ERROR;
    }
    
    hal_status_t status = espnow_manager->init();
    if (status != HAL_OK) {
        LOG_ERROR("Handheld", "Failed to initialize ESP-NOW");
        delete espnow_manager;
        espnow_manager = nullptr;
        return status;
    }
    
    LOG_INFO("Handheld", "ESP-NOW initialized");
    return HAL_OK;
}

hal_status_t HandheldApp::handleESPNowTest(uint32_t delta_ms) {
    if (espnow_screen && current_screen != espnow_screen) {
        LOG_INFO("Handheld", "Switching to ESP-NOW screen");
        switchToScreen(espnow_screen);
    }
    
    // Button 7 is typically the back button
    if (input_handler && input_handler->isPressed(7)) {
        LOG_INFO("Handheld", "Returning to menu from ESP-NOW screen");
        return state_machine.transitionTo(AppState::MENU);
    }
    
    return HAL_OK;
}

void HandheldApp::sendScreenSync(uint8_t screenType) {
    if (!espnow_manager || !espnow_manager->isPaired()) return;
    
    // Track last sent screen to avoid flooding
    static uint8_t last_sent_screen = 255; // Invalid initial value
    static uint32_t last_send_time = 0;
    uint32_t now = millis();
    
    // Only send if screen changed or it's been more than 1 second (for reconnection)
    if (screenType != last_sent_screen || (now - last_send_time) > 1000) {
        ESPNowMessage msg;
        msg.setScreenSync(screenType, ""); // Don't send name, just type
        espnow_manager->sendMessage(msg);
        LOG_INFO("Handheld", "Sent screen sync: %d", screenType);
        
        last_sent_screen = screenType;
        last_send_time = now;
    }
}

void HandheldApp::sendButtonData(uint8_t buttonStates) {
    if (!espnow_manager || !espnow_manager->isPaired()) return;
    
    // Rate limit button updates to 20Hz max
    static uint32_t last_button_send = 0;
    uint32_t now = millis();
    if (now - last_button_send < 50) return; // 50ms = 20Hz
    
    ESPNowMessage msg;
    msg.setButtonData(buttonStates);
    espnow_manager->sendMessage(msg);
    last_button_send = now;
}