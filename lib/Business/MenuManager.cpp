#include "MenuManager.h"
#include "../Core/Logger.h"
#include "DisplayController.h"
#include <string.h>

MenuManager::MenuManager(const char* title)
    : item_count(0)
    , current_index(0)
    , scroll_offset(0)
    , visible_items(4)
    , wrap_around(true)
    , needs_redraw(true) {
    setTitle(title);
    memset(items, 0, sizeof(items));
}

hal_status_t MenuManager::init() {
    reset();
    LOG_INFO("MenuManager", "Initialized with title: %s", menu_title);
    return HAL_OK;
}

void MenuManager::reset() {
    current_index = 0;
    scroll_offset = 0;
    needs_redraw = true;
}

hal_status_t MenuManager::addItem(const char* label, uint8_t id, std::function<void()> callback) {
    if (item_count >= MAX_MENU_ITEMS) {
        LOG_ERROR("MenuManager", "Menu full, cannot add item");
        return HAL_ERROR;
    }
    
    MenuItem& item = items[item_count];
    strncpy(item.label, label, MAX_LABEL_LENGTH - 1);
    item.label[MAX_LABEL_LENGTH - 1] = '\0';
    item.id = id;
    item.callback = callback;
    item.enabled = true;
    item.submenu = nullptr;
    
    item_count++;
    needs_redraw = true;
    
    LOG_DEBUG("MenuManager", "Added item: %s (id=%d)", label, id);
    return HAL_OK;
}

hal_status_t MenuManager::addItem(const MenuItem& item) {
    if (item_count >= MAX_MENU_ITEMS) {
        return HAL_ERROR;
    }
    
    items[item_count] = item;
    item_count++;
    needs_redraw = true;
    
    return HAL_OK;
}

hal_status_t MenuManager::removeItem(uint8_t id) {
    for (uint8_t i = 0; i < item_count; i++) {
        if (items[i].id == id) {
            for (uint8_t j = i; j < item_count - 1; j++) {
                items[j] = items[j + 1];
            }
            item_count--;
            
            if (current_index >= item_count && item_count > 0) {
                current_index = item_count - 1;
            }
            
            updateScrollOffset();
            needs_redraw = true;
            return HAL_OK;
        }
    }
    
    return HAL_ERROR;
}

void MenuManager::navigateUp() {
    if (item_count == 0) return;
    
    if (current_index > 0) {
        current_index--;
    } else if (wrap_around) {
        current_index = item_count - 1;
    }
    
    updateScrollOffset();
    needs_redraw = true;
}

void MenuManager::navigateDown() {
    if (item_count == 0) return;
    
    if (current_index < item_count - 1) {
        current_index++;
    } else if (wrap_around) {
        current_index = 0;
    }
    
    updateScrollOffset();
    needs_redraw = true;
}

void MenuManager::select() {
    if (item_count == 0 || current_index >= item_count) return;
    
    MenuItem& item = items[current_index];
    if (item.enabled && item.callback) {
        LOG_INFO("MenuManager", "Selected: %s", item.label);
        item.callback();
    }
}

void MenuManager::back() {
    LOG_INFO("MenuManager", "Back pressed");
}

void MenuManager::draw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display->interface->clear(display);
    drawTitle(display);
    drawItems(display);
    drawScrollIndicators(display);
    display->interface->refresh(display);
    
    needs_redraw = false;
}

MenuManager::MenuItem* MenuManager::getItem(uint8_t index) {
    if (index >= item_count) return nullptr;
    return &items[index];
}

const char* MenuManager::getCurrentLabel() const {
    if (current_index >= item_count) return "";
    return items[current_index].label;
}

void MenuManager::setTitle(const char* title) {
    strncpy(menu_title, title, MAX_LABEL_LENGTH - 1);
    menu_title[MAX_LABEL_LENGTH - 1] = '\0';
    needs_redraw = true;
}

void MenuManager::drawTitle(display_instance_t* display) {
    DisplayController::drawHeader(display, menu_title);
}

void MenuManager::drawItems(display_instance_t* display) {
    if (!display || !display->interface || item_count == 0) return;
    
    uint8_t start_item = scroll_offset;
    uint8_t end_item = scroll_offset + visible_items;
    if (end_item > item_count) end_item = item_count;
    
    uint8_t y_pos = 15;
    for (uint8_t i = start_item; i < end_item; i++) {
        display_point_t cursor = {5, (int16_t)y_pos};
        
        if (i == current_index) {
            display->interface->set_text_cursor(display, &cursor);
            display->interface->write_text(display, ">", 1);
        }
        
        cursor.x = 15;
        display->interface->set_text_cursor(display, &cursor);
        
        if (!items[i].enabled) {
            display->interface->write_text(display, "[", 1);
            display->interface->write_text(display, items[i].label, 1);
            display->interface->write_text(display, "]", 1);
        } else {
            display->interface->write_text(display, items[i].label, 1);
        }
        
        y_pos += 10;
    }
}

void MenuManager::drawScrollIndicators(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    if (scroll_offset > 0) {
        display_point_t cursor = {120, 15};
        display->interface->set_text_cursor(display, &cursor);
        display->interface->write_text(display, "^", 1);
    }
    
    if (scroll_offset + visible_items < item_count) {
        display_point_t cursor = {120, 50};
        display->interface->set_text_cursor(display, &cursor);
        display->interface->write_text(display, "v", 1);
    }
}

void MenuManager::updateScrollOffset() {
    if (current_index < scroll_offset) {
        scroll_offset = current_index;
    } else if (current_index >= scroll_offset + visible_items) {
        scroll_offset = current_index - visible_items + 1;
    }
}