#include "ButtonTestScreen.h"
#include "../../../lib/Business/DisplayController.h"
#include <Arduino.h>

HandheldButtonTestScreen::HandheldButtonTestScreen()
    : AppScreen("ButtonTest")
    , button_states(0)
    , last_press_time(0) {
}

void HandheldButtonTestScreen::onEnter() {
    AppScreen::onEnter();
    button_states = 0;
}

void HandheldButtonTestScreen::onDraw(display_instance_t* display) {
    if (!display || !display->interface) return;
    
    display->interface->clear(display);
    DisplayController::drawHeader(display, "Button Test");
    drawButtonBoxes(display);
    
    char hex_str[16];
    snprintf(hex_str, sizeof(hex_str), "0x%02X", button_states);
    DisplayController::drawFooter(display, hex_str);
    
    display->interface->refresh(display);
}

void HandheldButtonTestScreen::onButtonPress(uint8_t button_id) {
    if (button_id < MAX_BUTTONS) {
        button_states |= (1 << button_id);
        last_press_time = millis();
        requestRedraw();
    }
}

void HandheldButtonTestScreen::onButtonRelease(uint8_t button_id) {
    if (button_id < MAX_BUTTONS) {
        button_states &= ~(1 << button_id);
        requestRedraw();
    }
}

void HandheldButtonTestScreen::onInput(input_instance_t* input) {
    if (!input || !input->interface) return;
    
    uint8_t num_buttons = input->interface->get_channel_count(input);
    if (num_buttons > MAX_BUTTONS) num_buttons = MAX_BUTTONS;
    
    uint8_t new_states = 0;
    for (uint8_t i = 0; i < num_buttons; i++) {
        bool pressed = false;
        input->interface->read_channel(input, i, &pressed);
        if (pressed) {
            new_states |= (1 << i);
        }
    }
    
    if (new_states != button_states) {
        button_states = new_states;
        requestRedraw();
    }
}

void HandheldButtonTestScreen::drawButtonBoxes(display_instance_t* display) {
    for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
        bool pressed = (button_states & (1 << i)) != 0;
        drawButtonBox(display, i, pressed);
    }
}

void HandheldButtonTestScreen::drawButtonBox(display_instance_t* display, uint8_t index, bool pressed) {
    if (!display || !display->interface) return;
    
    uint8_t x = BUTTON_BOX_X_START + (index * BUTTON_BOX_SPACING);
    uint8_t y = BUTTON_BOX_Y;
    
    display_rect_t rect = {
        .x = (int16_t)x,
        .y = (int16_t)y,
        .width = BUTTON_BOX_SIZE,
        .height = BUTTON_BOX_SIZE
    };
    
    if (pressed) {
        display->interface->draw_rect(display, &rect, 1, true);
    } else {
        display->interface->draw_rect(display, &rect, 1, false);
    }
    
    char num_str[2] = {(char)('0' + index + 1), '\0'};
    display_point_t cursor = {(int16_t)(x + 3), (int16_t)(y + 2)};
    display->interface->set_text_cursor(display, &cursor);
    display->interface->write_text(display, num_str, 1);
}