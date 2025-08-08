#ifndef HANDHELD_BUTTON_TEST_SCREEN_H
#define HANDHELD_BUTTON_TEST_SCREEN_H

#include "../../../lib/Core/AppScreen.h"

class HandheldButtonTestScreen : public AppScreen {
public:
    HandheldButtonTestScreen();
    
    void onEnter() override;
    void onDraw(display_instance_t* display) override;
    void onButtonPress(uint8_t button_id) override;
    void onButtonRelease(uint8_t button_id) override;
    void onInput(input_instance_t* input) override;
    
private:
    static constexpr uint8_t MAX_BUTTONS = 8;
    static constexpr uint8_t BUTTON_BOX_SIZE = 12;
    static constexpr uint8_t BUTTON_BOX_SPACING = 14;
    static constexpr uint8_t BUTTON_BOX_Y = 20;
    static constexpr uint8_t BUTTON_BOX_X_START = 10;
    
    uint8_t button_states;
    uint32_t last_press_time;
    
    void drawButtonBoxes(display_instance_t* display);
    void drawButtonBox(display_instance_t* display, uint8_t index, bool pressed);
};

#endif