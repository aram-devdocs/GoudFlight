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
    , display_controller(nullptr)
    , input_handler(nullptr)
    , button_manager(nullptr)
    , main_menu(nullptr)
    , hardware({nullptr, nullptr}) {
}

hal_status_t HandheldApp::onInitialize() {
    hal_status_t status = initHardware();
    if (status != HAL_OK) return status;
    
    system_monitor = new SystemMonitor("Handheld", LED_BUILTIN);
    if (!system_monitor) return HAL_ERROR;
    status = system_monitor->init();
    if (status != HAL_OK) return status;
    
    status = initDisplay();
    if (status != HAL_OK) return status;
    
    status = initInput();
    if (status != HAL_OK) return status;
    
    status = initMenu();
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

hal_status_t HandheldApp::initDisplay() {
    if (!hardware.display || !hardware.display->interface) {
        LOG_WARNING("Handheld", "No display available");
        return HAL_OK;
    }
    
    display_controller = new DisplayController(hardware.display);
    if (!display_controller) return HAL_ERROR;
    
    hal_status_t status = display_controller->init();
    if (status != HAL_OK) return status;
    
    DisplayController::Screen startup = {
        .name = "Startup",
        .onDraw = [this](display_instance_t* d) { drawStartupScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = 100
    };
    
    DisplayController::Screen button_test = {
        .name = "ButtonTest",
        .onDraw = [this](display_instance_t* d) { drawButtonTestScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = Constants::Timing::DISPLAY_UPDATE_INTERVAL_MS
    };
    
    DisplayController::Screen menu = {
        .name = "Menu",
        .onDraw = [this](display_instance_t* d) { drawMenuScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = 100
    };
    
    DisplayController::Screen flight = {
        .name = "Flight",
        .onDraw = [this](display_instance_t* d) { drawFlightControlScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = 50
    };
    
    DisplayController::Screen settings = {
        .name = "Settings",
        .onDraw = [this](display_instance_t* d) { drawSettingsScreen(d); },
        .onEnter = nullptr,
        .onExit = nullptr,
        .refresh_interval_ms = 100
    };
    
    display_controller->registerScreen(0, startup);
    display_controller->registerScreen(1, button_test);
    display_controller->registerScreen(2, menu);
    display_controller->registerScreen(3, flight);
    display_controller->registerScreen(4, settings);
    
    LOG_INFO("Handheld", "Display initialized");
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
    
    input_handler->registerGlobalCallback(
        [this](uint8_t ch, input_event_t ev) { handleButtonPress(ch, ev); });
    
    button_manager = new ButtonManager(input_handler);
    button_manager->mapButton(ButtonManager::ButtonId::UP, 0);
    button_manager->mapButton(ButtonManager::ButtonId::DOWN, 1);
    button_manager->mapButton(ButtonManager::ButtonId::LEFT, 2);
    button_manager->mapButton(ButtonManager::ButtonId::RIGHT, 3);
    button_manager->mapButton(ButtonManager::ButtonId::SELECT, 4);
    button_manager->mapButton(ButtonManager::ButtonId::BACK, 5);
    button_manager->mapButton(ButtonManager::ButtonId::MENU, 6);
    button_manager->mapButton(ButtonManager::ButtonId::ACTION, 7);
    
    LOG_INFO("Handheld", "Input initialized with %u channels", 
             hardware.input->interface->get_channel_count(hardware.input));
    return HAL_OK;
}

hal_status_t HandheldApp::initMenu() {
    main_menu = new MenuScreen("Main Menu", 4);
    if (!main_menu) return HAL_ERROR;
    
    main_menu->addItem({
        .label = "Button Test",
        .onSelect = [this]() { state_machine.transitionTo(AppState::BUTTON_TEST); },
        .enabled = true
    });
    
    main_menu->addItem({
        .label = "Flight Control",
        .onSelect = [this]() { state_machine.transitionTo(AppState::FLIGHT_CONTROL); },
        .enabled = true
    });
    
    main_menu->addItem({
        .label = "Settings",
        .onSelect = [this]() { state_machine.transitionTo(AppState::SETTINGS); },
        .enabled = true
    });
    
    main_menu->addItem({
        .label = "System Info",
        .onSelect = [this]() { SystemInfo::printSystemInfo("Handheld"); },
        .enabled = true
    });
    
    return HAL_OK;
}

hal_status_t HandheldApp::initStates() {
    state_machine.registerState(AppState::INIT, "Init",
        [this](uint32_t dt) { return handleInitState(dt); });
    
    state_machine.registerState(AppState::STARTUP_SCREEN, "Startup",
        [this](uint32_t dt) { return handleStartupScreen(dt); });
    
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
    
    if (input_handler) {
        input_handler->update(delta_ms);
    }
    
    if (display_controller) {
        display_controller->update(delta_ms);
    }
    
    return state_machine.update(delta_ms);
}

hal_status_t HandheldApp::onShutdown() {
    LOG_INFO("Handheld", "Shutting down");
    
    delete main_menu;
    delete button_manager;
    delete input_handler;
    delete display_controller;
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
    if (display_controller && display_controller->getCurrentScreen() != 0) {
        display_controller->switchToScreen(0);
    }
    
    if (state_machine.getStateTime() >= Constants::System::STARTUP_SCREEN_DURATION_MS) {
        return state_machine.transitionTo(AppState::BUTTON_TEST);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleButtonTest(uint32_t delta_ms) {
    if (display_controller && display_controller->getCurrentScreen() != 1) {
        display_controller->switchToScreen(1);
    }
    
    static uint32_t last_debug = 0;
    uint32_t current_time = millis();
    
    if (current_time - last_debug >= Constants::Timing::DEBUG_PRINT_INTERVAL_MS) {
        if (input_handler) {
            char hex_str[16];
            DataFormatter::formatHex(hex_str, sizeof(hex_str), 
                                    ~input_handler->getRawState() & 0xFF, 2);
            LOG_DEBUG("ButtonTest", "Buttons: %s", hex_str);
        }
        last_debug = current_time;
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleMenuState(uint32_t delta_ms) {
    if (display_controller && display_controller->getCurrentScreen() != 2) {
        display_controller->switchToScreen(2);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleFlightControl(uint32_t delta_ms) {
    if (display_controller && display_controller->getCurrentScreen() != 3) {
        display_controller->switchToScreen(3);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleSettings(uint32_t delta_ms) {
    if (display_controller && display_controller->getCurrentScreen() != 4) {
        display_controller->switchToScreen(4);
    }
    
    return HAL_OK;
}

hal_status_t HandheldApp::handleErrorState(uint32_t delta_ms) {
    if (display_controller) {
        display_controller->clear();
        display_controller->drawText(30, 20, "ERROR", 2);
        display_controller->refresh();
    }
    
    return HAL_OK;
}

void HandheldApp::drawStartupScreen(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    DisplayController::drawCenteredText(display, 10, "GoudFlight", 2);
    DisplayController::drawCenteredText(display, 35, "HAL v1.0", 1);
    DisplayController::drawCenteredText(display, 50, "Initializing...", 1);
}

void HandheldApp::drawButtonTestScreen(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    const display_interface_t* iface = display->interface;
    
    iface->clear(display);
    
    DisplayController::drawHeader(display, "Button Test");
    
    if (hardware.input && hardware.input->interface) {
        uint8_t num_buttons = hardware.input->interface->get_channel_count(hardware.input);
        if (num_buttons > 8) num_buttons = 8;
        
        for (uint8_t i = 0; i < num_buttons; i++) {
            bool pressed = input_handler ? input_handler->isPressed(i) : false;
            
            display_rect_t button_rect = {
                .x = static_cast<int16_t>(10 + i * 14),
                .y = 20,
                .width = 12,
                .height = 12
            };
            
            iface->draw_rect(display, &button_rect, DISPLAY_COLOR_WHITE, pressed);
            
            display_point_t cursor = {
                .x = static_cast<int16_t>(button_rect.x + 3),
                .y = static_cast<int16_t>(button_rect.y + 16)
            };
            iface->set_text_cursor(display, &cursor);
            
            char num_str[2] = {static_cast<char>('1' + i), '\0'};
            iface->write_text(display, num_str, 1);
        }
        
        if (input_handler) {
            char hex_str[16];
            DataFormatter::formatHex(hex_str, sizeof(hex_str), 
                                    ~input_handler->getRawState() & 0xFF, 2);
            
            display_point_t cursor = {5, 50};
            iface->set_text_cursor(display, &cursor);
            iface->write_text(display, "Data: ", 1);
            iface->write_text(display, hex_str, 1);
        }
    }
    
    iface->refresh(display);
}

void HandheldApp::drawMenuScreen(display_instance_t* display) {
    if (main_menu && display) {
        main_menu->draw(display);
        display->interface->refresh(display);
    }
}

void HandheldApp::drawFlightControlScreen(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    DisplayController::drawHeader(display, "Flight Control");
    DisplayController::drawCenteredText(display, 30, "Ready", 1);
    DisplayController::drawFooter(display, "Press BACK");
}

void HandheldApp::drawSettingsScreen(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    DisplayController::drawHeader(display, "Settings");
    DisplayController::drawCenteredText(display, 30, "Coming Soon", 1);
    DisplayController::drawFooter(display, "Press BACK");
}

void HandheldApp::handleButtonPress(uint8_t channel, input_event_t event) {
    if (event == INPUT_EVENT_PRESSED) {
        LOG_DEBUG("Input", "Button %u pressed", channel + 1);
    }
    
    if (state_machine.isInState(AppState::MENU) && main_menu) {
        if (event == INPUT_EVENT_RELEASED) {
            switch (channel) {
                case 0: main_menu->selectPrevious(); break;
                case 1: main_menu->selectNext(); break;
                case 4: main_menu->activateSelected(); break;
                case 5: state_machine.transitionTo(AppState::BUTTON_TEST); break;
            }
        }
    } else if (state_machine.isInState(AppState::BUTTON_TEST)) {
        if (channel == 6 && event == INPUT_EVENT_RELEASED) {
            state_machine.transitionTo(AppState::MENU);
        }
    } else if (state_machine.isInState(AppState::FLIGHT_CONTROL) || 
               state_machine.isInState(AppState::SETTINGS)) {
        if (channel == 5 && event == INPUT_EVENT_RELEASED) {
            state_machine.transitionTo(AppState::MENU);
        }
    }
}