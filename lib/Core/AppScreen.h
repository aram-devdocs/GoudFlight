#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include "../HAL/Display/display_interface.h"
#include "../HAL/Input/input_interface.h"
#include "../HAL/Core/hal_types.h"
#include <Arduino.h>

class AppScreen {
public:
    AppScreen(const char* name) 
        : screen_name(name)
        , needs_redraw(true)
        , is_active(false)
        , is_initialized(false) {
    }
    
    virtual ~AppScreen() = default;
    
    virtual hal_status_t onInitialize() { 
        is_initialized = true;
        return HAL_OK; 
    }
    
    virtual void onEnter() {
        is_active = true;
        needs_redraw = true;
    }
    
    virtual void onExit() {
        is_active = false;
    }
    
    virtual void onUpdate(uint32_t delta_ms) {
    }
    
    virtual void onDraw(display_instance_t* display) = 0;
    
    virtual void onButtonPress(uint8_t button_id) {
    }
    
    virtual void onButtonRelease(uint8_t button_id) {
    }
    
    virtual void onInput(input_instance_t* input) {
    }
    
    void requestRedraw() { needs_redraw = true; }
    void clearRedrawFlag() { needs_redraw = false; }
    bool needsRedraw() const { return needs_redraw; }
    bool isActive() const { return is_active; }
    bool isInitialized() const { return is_initialized; }
    const char* getName() const { return screen_name; }
    
protected:
    const char* screen_name;
    bool needs_redraw;
    bool is_active;
    bool is_initialized;
};

#endif