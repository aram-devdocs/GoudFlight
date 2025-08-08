#include "DisplayController.h"
#include "../Core/Logger.h"
#include "../Core/Constants.h"

DisplayController::DisplayController(display_instance_t* display)
    : display(display)
    , current_screen(0xFF)
    , last_refresh_time(0)
    , screen_time(0) {
}

hal_status_t DisplayController::init() {
    if (!display || !display->interface) {
        LOG_ERROR("DisplayController", "Invalid display instance");
        return HAL_INVALID_PARAM;
    }
    
    clear();
    setBrightness(Constants::Display::DEFAULT_BRIGHTNESS);
    
    LOG_INFO("DisplayController", "Display controller initialized");
    return HAL_OK;
}

hal_status_t DisplayController::update(uint32_t delta_ms) {
    if (current_screen == 0xFF) {
        return HAL_OK;
    }
    
    screen_time += delta_ms;
    
    const Screen* screen = findScreen(current_screen);
    if (!screen) {
        return HAL_ERROR;
    }
    
    uint32_t current_time = millis();
    if (current_time - last_refresh_time >= screen->refresh_interval_ms) {
        if (screen->onDraw) {
            screen->onDraw(display);
        }
        refresh();
        last_refresh_time = current_time;
    }
    
    return HAL_OK;
}

hal_status_t DisplayController::registerScreen(uint8_t id, const Screen& screen) {
    for (auto& s : screens) {
        if (s.first == id) {
            LOG_WARNING("DisplayController", "Screen %u already registered", id);
            return HAL_ERROR;
        }
    }
    
    screens.push_back({id, screen});
    LOG_DEBUG("DisplayController", "Registered screen %u: %s", id, screen.name);
    return HAL_OK;
}

hal_status_t DisplayController::switchToScreen(uint8_t screen_id) {
    const Screen* new_screen = findScreen(screen_id);
    if (!new_screen) {
        LOG_ERROR("DisplayController", "Screen %u not found", screen_id);
        return HAL_INVALID_PARAM;
    }
    
    if (current_screen != 0xFF) {
        const Screen* old_screen = findScreen(current_screen);
        if (old_screen && old_screen->onExit) {
            old_screen->onExit(display);
        }
    }
    
    current_screen = screen_id;
    screen_time = 0;
    last_refresh_time = 0;
    
    clear();
    if (new_screen->onEnter) {
        new_screen->onEnter(display);
    }
    
    LOG_INFO("DisplayController", "Switched to screen: %s", new_screen->name);
    return HAL_OK;
}

const DisplayController::Screen* DisplayController::findScreen(uint8_t id) const {
    for (const auto& s : screens) {
        if (s.first == id) {
            return &s.second;
        }
    }
    return nullptr;
}

void DisplayController::clear() {
    if (display && display->interface) {
        display->interface->clear(display);
    }
}

void DisplayController::refresh() {
    if (display && display->interface) {
        display->interface->refresh(display);
    }
}

void DisplayController::setBrightness(uint8_t brightness) {
    if (display && display->interface) {
        display->interface->set_brightness(display, brightness);
    }
}

void DisplayController::drawText(int16_t x, int16_t y, const char* text, uint8_t size) {
    if (!display || !display->interface) return;
    
    display_point_t cursor = {x, y};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, text, size);
}

void DisplayController::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    if (!display || !display->interface) return;
    
    display_point_t start = {x0, y0};
    display_point_t end = {x1, y1};
    display->interface->draw_line(display, &start, &end, DISPLAY_COLOR_WHITE);
}

void DisplayController::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool filled) {
    if (!display || !display->interface) return;
    
    display_rect_t rect = {
        .x = x,
        .y = y,
        .width = static_cast<uint16_t>(w),
        .height = static_cast<uint16_t>(h)
    };
    display->interface->draw_rect(display, &rect, DISPLAY_COLOR_WHITE, filled);
}

void DisplayController::drawCircle(int16_t x, int16_t y, int16_t r, bool filled) {
    if (!display || !display->interface) return;
    
    display_point_t center = {x, y};
    display->interface->draw_circle(display, &center, r, DISPLAY_COLOR_WHITE, filled);
}

void DisplayController::drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent) {
    if (!display || !display->interface) return;
    
    if (percent > 100) percent = 100;
    
    drawRect(x, y, w, h, false);
    
    int16_t fill_width = (w - 2) * percent / 100;
    if (fill_width > 0) {
        drawRect(x + 1, y + 1, fill_width, h - 2, true);
    }
}

void DisplayController::drawBattery(int16_t x, int16_t y, uint8_t percent, bool charging) {
    if (!display || !display->interface) return;
    
    const int16_t width = 20;
    const int16_t height = 10;
    const int16_t cap_width = 2;
    
    drawRect(x, y, width, height, false);
    drawRect(x + width, y + 2, cap_width, height - 4, true);
    
    int16_t fill_width = (width - 4) * percent / 100;
    if (fill_width > 0) {
        drawRect(x + 2, y + 2, fill_width, height - 4, true);
    }
    
    if (charging) {
        drawText(x + width/2 - 2, y + 1, "+", 1);
    }
}

void DisplayController::drawSignalStrength(int16_t x, int16_t y, uint8_t bars) {
    if (!display || !display->interface) return;
    
    const int16_t bar_width = 3;
    const int16_t bar_spacing = 2;
    const int16_t max_bars = 4;
    
    if (bars > max_bars) bars = max_bars;
    
    for (uint8_t i = 0; i < max_bars; i++) {
        int16_t bar_height = 3 + i * 2;
        int16_t bar_x = x + i * (bar_width + bar_spacing);
        int16_t bar_y = y + (8 - bar_height);
        
        if (i < bars) {
            drawRect(bar_x, bar_y, bar_width, bar_height, true);
        } else {
            drawRect(bar_x, bar_y, bar_width, bar_height, false);
        }
    }
}

void DisplayController::drawCenteredText(display_instance_t* display, int16_t y, const char* text, uint8_t size) {
    if (!display || !display->interface || !text) return;
    
    int16_t text_width = strlen(text) * 6 * size;
    int16_t display_width = 128;
    
    if (display->interface->get_info) {
        display_info_t info;
        display->interface->get_info(display, &info);
        display_width = info.width;
    }
    
    int16_t x = (display_width - text_width) / 2;
    display_point_t cursor = {x, y};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, text, size);
}

void DisplayController::drawHeader(display_instance_t* display, const char* title) {
    if (!display || !display->interface) return;
    
    drawCenteredText(display, 0, title, 1);
    
    display_point_t line_start = {0, 10};
    display_point_t line_end = {127, 10};
    display->interface->draw_line(display, &line_start, &line_end, DISPLAY_COLOR_WHITE);
}

void DisplayController::drawFooter(display_instance_t* display, const char* text) {
    if (!display || !display->interface) return;
    
    display_point_t line_start = {0, 54};
    display_point_t line_end = {127, 54};
    display->interface->draw_line(display, &line_start, &line_end, DISPLAY_COLOR_WHITE);
    
    drawCenteredText(display, 56, text, 1);
}

MenuScreen::MenuScreen(const char* title, uint8_t max_visible)
    : title(title)
    , selected_index(0)
    , scroll_offset(0)
    , max_visible_items(max_visible) {
}

void MenuScreen::addItem(const MenuItem& item) {
    items.push_back(item);
}

void MenuScreen::draw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    DisplayController::drawHeader(display, title);
    
    uint8_t visible_start = scroll_offset;
    uint8_t visible_end = (scroll_offset + max_visible_items < items.size()) ? 
                          (scroll_offset + max_visible_items) : items.size();
    
    for (uint8_t i = visible_start; i < visible_end; i++) {
        int16_t y = 15 + (i - visible_start) * 10;
        display_point_t cursor = {10, y};
        
        if (i == selected_index) {
            cursor.x = 2;
            display->interface->set_text_cursor(display, &cursor);
            display->interface->write_text(display, ">", 1);
            cursor.x = 10;
        }
        
        display->interface->set_text_cursor(display, &cursor);
        display->interface->write_text(display, items[i].label, 1);
        
        if (!items[i].enabled) {
            display_point_t line_start = {10, static_cast<int16_t>(y + 4)};
            display_point_t line_end = {static_cast<int16_t>(10 + strlen(items[i].label) * 6), static_cast<int16_t>(y + 4)};
            display->interface->draw_line(display, &line_start, &line_end, DISPLAY_COLOR_WHITE);
        }
    }
}

void MenuScreen::selectNext() {
    if (selected_index < items.size() - 1) {
        selected_index++;
        if (selected_index >= scroll_offset + max_visible_items) {
            scroll_offset++;
        }
    }
}

void MenuScreen::selectPrevious() {
    if (selected_index > 0) {
        selected_index--;
        if (selected_index < scroll_offset) {
            scroll_offset--;
        }
    }
}

void MenuScreen::activateSelected() {
    if (selected_index < items.size() && items[selected_index].enabled && items[selected_index].onSelect) {
        items[selected_index].onSelect();
    }
}