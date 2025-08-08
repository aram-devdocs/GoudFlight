#include "CounterScreen.h"
#include "../../../lib/Business/DataFormatter.h"
#include <Arduino.h>

CounterScreen::CounterScreen()
    : AppScreen("Counter")
    , counter(0)
    , last_counter_update(0) {
}

void CounterScreen::onEnter() {
    AppScreen::onEnter();
    counter = 0;
    last_counter_update = millis();
}

void CounterScreen::onUpdate(uint32_t delta_ms) {
    uint32_t current_time = millis();
    
    if (current_time - last_counter_update >= 1000) {
        counter++;
        last_counter_update = current_time;
        requestRedraw();
    }
}

void CounterScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    drawStatus(display);
    drawCounter(display);
}

void CounterScreen::drawStatus(display_instance_t* display) {
    display_point_t cursor = {0, 0};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, "Base Station OK", 1);
}

void CounterScreen::drawCounter(display_instance_t* display) {
    char time_str[32];
    DataFormatter::formatUptime(time_str, sizeof(time_str), counter);
    
    display_point_t cursor = {0, 1};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, time_str, 1);
}