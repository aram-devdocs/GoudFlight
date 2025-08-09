#ifndef HANDHELD_ESPNOW_SCREEN_H
#define HANDHELD_ESPNOW_SCREEN_H

#include "../../../lib/Core/AppScreen.h"
#include "../../../lib/Communication/ESPNow/ESPNowManager.h"

class HandheldESPNowScreen : public AppScreen {
public:
    HandheldESPNowScreen(ESPNowManager* manager);
    ~HandheldESPNowScreen() = default;
    
    hal_status_t onInitialize() override;
    void onEnter() override;
    void onUpdate(uint32_t delta_ms) override;
    void onDraw(display_instance_t* display) override;
    void onButtonPress(uint8_t button_id) override;
    
private:
    ESPNowManager* espnow_manager;
    uint32_t update_timer;
    uint32_t animation_frame;
    
    void drawHeader(display_instance_t* display);
    void drawMacAddress(display_instance_t* display);
    void drawConnectionStatus(display_instance_t* display);
    void drawStatistics(display_instance_t* display);
    void drawSearchingAnimation(display_instance_t* display);
    void drawPairingAnimation(display_instance_t* display);
};

#endif