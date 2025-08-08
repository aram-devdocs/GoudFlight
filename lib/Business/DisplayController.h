#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Arduino.h>
#include "../HAL/Core/hal_types.h"
#include "../HAL/Display/display_interface.h"
#include <vector>
#include <functional>

class DisplayController {
public:
    using DrawCallback = std::function<void(display_instance_t*)>;
    
    struct Screen {
        const char* name;
        DrawCallback onDraw;
        DrawCallback onEnter;
        DrawCallback onExit;
        uint32_t refresh_interval_ms;
    };
    
    DisplayController(display_instance_t* display);
    ~DisplayController() = default;
    
    hal_status_t init();
    hal_status_t update(uint32_t delta_ms);
    
    hal_status_t registerScreen(uint8_t id, const Screen& screen);
    hal_status_t switchToScreen(uint8_t screen_id);
    
    void clear();
    void refresh();
    void setBrightness(uint8_t brightness);
    
    void drawText(int16_t x, int16_t y, const char* text, uint8_t size = 1);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool filled = false);
    void drawCircle(int16_t x, int16_t y, int16_t r, bool filled = false);
    
    void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent);
    void drawBattery(int16_t x, int16_t y, uint8_t percent, bool charging = false);
    void drawSignalStrength(int16_t x, int16_t y, uint8_t bars);
    
    display_instance_t* getDisplay() { return display; }
    uint8_t getCurrentScreen() const { return current_screen; }
    
    static void drawCenteredText(display_instance_t* display, int16_t y, const char* text, uint8_t size = 1);
    static void drawHeader(display_instance_t* display, const char* title);
    static void drawFooter(display_instance_t* display, const char* text);
    
private:
    display_instance_t* display;
    std::vector<std::pair<uint8_t, Screen>> screens;
    uint8_t current_screen;
    uint32_t last_refresh_time;
    uint32_t screen_time;
    
    const Screen* findScreen(uint8_t id) const;
};

class MenuScreen {
public:
    struct MenuItem {
        const char* label;
        std::function<void()> onSelect;
        bool enabled;
    };
    
    MenuScreen(const char* title, uint8_t max_visible = 4);
    ~MenuScreen() = default;
    
    void addItem(const MenuItem& item);
    void draw(display_instance_t* display);
    void handleInput(uint8_t button_mask);
    
    void selectNext();
    void selectPrevious();
    void activateSelected();
    
    uint8_t getSelectedIndex() const { return selected_index; }
    
private:
    const char* title;
    std::vector<MenuItem> items;
    uint8_t selected_index;
    uint8_t scroll_offset;
    uint8_t max_visible_items;
};

#endif