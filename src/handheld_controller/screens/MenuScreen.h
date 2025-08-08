#ifndef HANDHELD_MENU_SCREEN_H
#define HANDHELD_MENU_SCREEN_H

#include "../../../lib/Core/AppScreen.h"
#include "../../../lib/Business/MenuManager.h"

class HandheldMenuScreen : public AppScreen {
public:
    HandheldMenuScreen();
    ~HandheldMenuScreen();
    
    hal_status_t onInitialize() override;
    void onEnter() override;
    void onDraw(display_instance_t* display) override;
    void onButtonPress(uint8_t button_id) override;
    
    MenuManager* getMenu() { return menu; }
    
private:
    MenuManager* menu;
    
    void initMenuItems();
};

#endif