#include <Arduino.h>
#include "DroneApp.h"
#include "../../lib/Core/Logger.h"
#include "../../lib/Core/Constants.h"

static DroneApp* app = nullptr;

void setup() {
    Logger::init(Constants::Serial::BAUD_RATE, Constants::Serial::INIT_TIMEOUT_MS);
    delay(Constants::Serial::INIT_DELAY_MS);
    
    LOG_INFO("Main", "=====================================");
    LOG_INFO("Main", "   Drone Flight Controller");
    LOG_INFO("Main", "=====================================");
    LOG_INFO("Main", "Version: %s", Constants::System::VERSION);
    LOG_INFO("Main", "Build: %s %s", Constants::System::BUILD_DATE, Constants::System::BUILD_TIME);
    
    app = new DroneApp();
    if (!app) {
        LOG_CRITICAL("Main", "Failed to create application instance");
        while(1) { delay(1000); }
    }
    
    hal_status_t status = app->run();
    if (status != HAL_OK) {
        LOG_CRITICAL("Main", "Application initialization failed: %d", status);
        while(1) { delay(1000); }
    }
    
    LOG_INFO("Main", "Setup complete");
}

void loop() {
    uint32_t loop_start = micros();
    
    if (app) {
        hal_status_t status = app->run();
        if (status != HAL_OK) {
            LOG_ERROR("Main", "Application error: %d", status);
        }
    }
    
    uint32_t loop_time = micros() - loop_start;
    if (loop_time > 100000) {
        LOG_WARNING("Main", "Slow loop detected: %lu us", loop_time);
    }
    
    delay(Constants::Timing::INPUT_POLL_INTERVAL_MS);
}