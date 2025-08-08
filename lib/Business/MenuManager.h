#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include "../HAL/Display/display_interface.h"
#include "../HAL/Core/hal_types.h"
#include <Arduino.h>
#include <functional>

class MenuManager {
public:
    static constexpr uint8_t MAX_MENU_ITEMS = 10;
    static constexpr uint8_t MAX_LABEL_LENGTH = 32;
    
    struct MenuItem {
        char label[MAX_LABEL_LENGTH];
        uint8_t id;
        std::function<void()> callback;
        bool enabled;
        MenuItem* submenu;
    };
    
    MenuManager(const char* title = "Menu");
    ~MenuManager() = default;
    
    hal_status_t init();
    void reset();
    
    hal_status_t addItem(const char* label, uint8_t id, std::function<void()> callback = nullptr);
    hal_status_t addItem(const MenuItem& item);
    hal_status_t removeItem(uint8_t id);
    
    void navigateUp();
    void navigateDown();
    void select();
    void back();
    
    void draw(display_instance_t* display);
    
    uint8_t getCurrentItem() const { return current_index; }
    uint8_t getItemCount() const { return item_count; }
    MenuItem* getItem(uint8_t index);
    const char* getCurrentLabel() const;
    
    void setTitle(const char* title);
    void setWrapAround(bool wrap) { wrap_around = wrap; }
    void setVisibleItems(uint8_t count) { visible_items = count; }
    
private:
    char menu_title[MAX_LABEL_LENGTH];
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t item_count;
    uint8_t current_index;
    uint8_t scroll_offset;
    uint8_t visible_items;
    bool wrap_around;
    bool needs_redraw;
    
    void drawTitle(display_instance_t* display);
    void drawItems(display_instance_t* display);
    void drawScrollIndicators(display_instance_t* display);
    void updateScrollOffset();
};

#endif