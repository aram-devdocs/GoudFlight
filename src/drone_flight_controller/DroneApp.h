#ifndef DRONE_APP_H
#define DRONE_APP_H

#include "../../lib/Core/AppFramework.h"
#include "../../lib/Core/StateManager.h"
#include "../../lib/Business/SystemMonitor.h"

class DroneApp : public AppFramework {
public:
    enum class FlightState : uint16_t {
        INIT = 0,
        PREFLIGHT_CHECK,
        IDLE,
        ARMING,
        ARMED,
        TAKEOFF,
        HOVER,
        FLYING,
        LANDING,
        EMERGENCY,
        ERROR
    };
    
    DroneApp();
    ~DroneApp() = default;
    
protected:
    hal_status_t onInitialize() override;
    hal_status_t onStart() override;
    hal_status_t onUpdate(uint32_t delta_ms) override;
    hal_status_t onShutdown() override;
    hal_status_t onError(hal_status_t error) override;
    
private:
    StateMachine<FlightState> flight_state_machine;
    SystemMonitor* system_monitor;
    
    struct FlightData {
        bool motors_armed;
        uint32_t arm_time;
        uint32_t flight_time;
        float battery_voltage;
        uint8_t signal_strength;
        bool emergency_stop;
    } flight_data;
    
    hal_status_t initHardware();
    hal_status_t initFlightStates();
    hal_status_t performPreflightCheck();
    
    hal_status_t handleInitState(uint32_t delta_ms);
    hal_status_t handlePreflightCheck(uint32_t delta_ms);
    hal_status_t handleIdleState(uint32_t delta_ms);
    hal_status_t handleArmingState(uint32_t delta_ms);
    hal_status_t handleArmedState(uint32_t delta_ms);
    hal_status_t handleTakeoffState(uint32_t delta_ms);
    hal_status_t handleHoverState(uint32_t delta_ms);
    hal_status_t handleFlyingState(uint32_t delta_ms);
    hal_status_t handleLandingState(uint32_t delta_ms);
    hal_status_t handleEmergencyState(uint32_t delta_ms);
    hal_status_t handleErrorState(uint32_t delta_ms);
    
    void updateTelemetry();
    void sendHeartbeat();
    bool checkSafetyConditions();
    void emergencyStop();
};

#endif