#ifndef HANDHELD_STARTUP_SCREEN_H
#define HANDHELD_STARTUP_SCREEN_H

#include "../../../lib/Core/AppScreen.h"
#include "../../../lib/Core/Constants.h"

class HandheldStartupScreen : public AppScreen {
public:
    HandheldStartupScreen();
    
    void onUpdate(uint32_t delta_ms) override;
    void onDraw(display_instance_t* display) override;
    
    bool isComplete() const { return elapsed_time >= Constants::System::STARTUP_SCREEN_DURATION_MS; }
    
private:
    uint32_t elapsed_time;
};

#endif