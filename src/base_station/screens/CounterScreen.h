#ifndef BASE_STATION_COUNTER_SCREEN_H
#define BASE_STATION_COUNTER_SCREEN_H

#include "../../../lib/Core/AppScreen.h"

class CounterScreen : public AppScreen {
public:
    CounterScreen();
    
    void onEnter() override;
    void onUpdate(uint32_t delta_ms) override;
    void onDraw(display_instance_t* display) override;
    
    uint32_t getCounter() const { return counter; }
    
private:
    uint32_t counter;
    uint32_t last_counter_update;
    
    void drawStatus(display_instance_t* display);
    void drawCounter(display_instance_t* display);
};

#endif