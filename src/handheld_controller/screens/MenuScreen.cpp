#include "MenuScreen.h"
#include "../../../lib/Core/Logger.h"

HandheldMenuScreen::HandheldMenuScreen()
    : AppScreen("Menu")
    , menu(nullptr) {
}

HandheldMenuScreen::~HandheldMenuScreen() {
    if (menu) {
        delete menu;
        menu = nullptr;
    }
}

hal_status_t HandheldMenuScreen::onInitialize() {
    AppScreen::onInitialize();
    
    menu = new MenuManager();
    if (!menu) return HAL_ERROR;
    
    hal_status_t status = menu->init();
    if (status != HAL_OK) return status;
    
    initMenuItems();
    return HAL_OK;
}

void HandheldMenuScreen::onEnter() {
    AppScreen::onEnter();
    if (menu) {
        menu->reset();
    }
}

void HandheldMenuScreen::onDraw(display_instance_t* display) {
    if (menu && display) {
        menu->draw(display);
        display->interface->refresh(display);
    }
}

void HandheldMenuScreen::onButtonPress(uint8_t button_id) {
    if (!menu) return;
    
    switch (button_id) {
        case 0:
            menu->navigateUp();
            requestRedraw();
            break;
        case 1:
            menu->navigateDown();
            requestRedraw();
            break;
        case 2:
            menu->select();
            requestRedraw();
            break;
        case 3:
            menu->back();
            requestRedraw();
            break;
    }
}

void HandheldMenuScreen::initMenuItems() {
    if (!menu) return;
    
    menu->addItem("Flight Control", 1, [this]() { 
        LOG_INFO("Menu", "Flight Control selected");
    });
    
    menu->addItem("ESP-NOW Status", 2, [this]() {
        LOG_INFO("Menu", "ESP-NOW Status selected");
    });
    
    menu->addItem("Button Test", 3, [this]() {
        LOG_INFO("Menu", "Button Test selected");
    });
    
    menu->addItem("Settings", 4, [this]() {
        LOG_INFO("Menu", "Settings selected");
    });
    
    menu->addItem("System Info", 5, [this]() {
        LOG_INFO("Menu", "System Info selected");
    });
}