#include "DroneApp.h"
#include "../../lib/Core/Logger.h"
#include "../../lib/Core/Constants.h"
#include "../../lib/Business/DataFormatter.h"
#include "../../lib/SystemInfo/system_info.h"

DroneApp::DroneApp()
    : AppFramework("DroneFC", HAL_BOARD_DRONE)
    , flight_state_machine("FlightSM")
    , system_monitor(nullptr) {
    
    memset(&flight_data, 0, sizeof(flight_data));
    flight_data.battery_voltage = 3.7f;
    flight_data.signal_strength = 100;
}

hal_status_t DroneApp::onInitialize() {
    hal_status_t status = initHardware();
    if (status != HAL_OK) return status;
    
    uint8_t led_pin = LED_BUILTIN;
    #ifdef ESP32
        led_pin = Constants::Hardware::LED_BUILTIN_ESP32;
    #endif
    
    system_monitor = new SystemMonitor("DroneFC", led_pin);
    if (!system_monitor) return HAL_ERROR;
    
    status = system_monitor->init();
    if (status != HAL_OK) return status;
    
    system_monitor->setHealthCallback([this]() { return checkSafetyConditions(); });
    
    status = initFlightStates();
    if (status != HAL_OK) return status;
    
    return flight_state_machine.transitionTo(FlightState::PREFLIGHT_CHECK);
}

hal_status_t DroneApp::initHardware() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    LOG_INFO("DroneFC", "Hardware initialized");
    return HAL_OK;
}

hal_status_t DroneApp::initFlightStates() {
    flight_state_machine.registerState(FlightState::INIT, "Init",
        [this](uint32_t dt) { return handleInitState(dt); });
    
    flight_state_machine.registerState(FlightState::PREFLIGHT_CHECK, "PreflightCheck",
        [this](uint32_t dt) { return handlePreflightCheck(dt); });
    
    flight_state_machine.registerState(FlightState::IDLE, "Idle",
        [this](uint32_t dt) { return handleIdleState(dt); });
    
    flight_state_machine.registerState(FlightState::ARMING, "Arming",
        [this](uint32_t dt) { return handleArmingState(dt); });
    
    flight_state_machine.registerState(FlightState::ARMED, "Armed",
        [this](uint32_t dt) { return handleArmedState(dt); });
    
    flight_state_machine.registerState(FlightState::TAKEOFF, "Takeoff",
        [this](uint32_t dt) { return handleTakeoffState(dt); });
    
    flight_state_machine.registerState(FlightState::HOVER, "Hover",
        [this](uint32_t dt) { return handleHoverState(dt); });
    
    flight_state_machine.registerState(FlightState::FLYING, "Flying",
        [this](uint32_t dt) { return handleFlyingState(dt); });
    
    flight_state_machine.registerState(FlightState::LANDING, "Landing",
        [this](uint32_t dt) { return handleLandingState(dt); });
    
    flight_state_machine.registerState(FlightState::EMERGENCY, "Emergency",
        [this](uint32_t dt) { return handleEmergencyState(dt); });
    
    flight_state_machine.registerState(FlightState::ERROR, "Error",
        [this](uint32_t dt) { return handleErrorState(dt); });
    
    LOG_INFO("DroneFC", "Flight states initialized");
    return HAL_OK;
}

hal_status_t DroneApp::onStart() {
    LOG_INFO("DroneFC", "====================================");
    LOG_INFO("DroneFC", "    Drone Flight Controller");
    LOG_INFO("DroneFC", "====================================");
    SystemInfo::printSystemInfo("Drone Flight Controller");
    return HAL_OK;
}

hal_status_t DroneApp::onUpdate(uint32_t delta_ms) {
    if (system_monitor) {
        system_monitor->update(delta_ms);
    }
    
    updateTelemetry();
    sendHeartbeat();
    
    if (flight_data.emergency_stop) {
        emergencyStop();
        return flight_state_machine.transitionTo(FlightState::EMERGENCY);
    }
    
    return flight_state_machine.update(delta_ms);
}

hal_status_t DroneApp::onShutdown() {
    LOG_INFO("DroneFC", "Shutting down flight controller");
    
    if (flight_data.motors_armed) {
        emergencyStop();
    }
    
    delete system_monitor;
    system_monitor = nullptr;
    
    return HAL_OK;
}

hal_status_t DroneApp::onError(hal_status_t error) {
    LOG_ERROR("DroneFC", "Flight controller error: %s", DataFormatter::errorToString(error));
    emergencyStop();
    return flight_state_machine.transitionTo(FlightState::ERROR);
}

hal_status_t DroneApp::performPreflightCheck() {
    LOG_INFO("DroneFC", "Performing preflight checks...");
    
    bool checks_passed = true;
    
    uint32_t free_heap = SystemInfo::getFreeHeap();
    if (free_heap < Constants::Memory::MIN_FREE_HEAP_WARNING) {
        LOG_WARNING("DroneFC", "Low memory: %lu bytes", free_heap);
        checks_passed = false;
    }
    
    if (flight_data.battery_voltage < 3.3f) {
        LOG_WARNING("DroneFC", "Low battery: %.2fV", flight_data.battery_voltage);
        checks_passed = false;
    }
    
    if (flight_data.signal_strength < 50) {
        LOG_WARNING("DroneFC", "Weak signal: %u%%", flight_data.signal_strength);
        checks_passed = false;
    }
    
    if (checks_passed) {
        LOG_INFO("DroneFC", "Preflight checks PASSED");
        return HAL_OK;
    } else {
        LOG_ERROR("DroneFC", "Preflight checks FAILED");
        return HAL_ERROR;
    }
}

hal_status_t DroneApp::handleInitState(uint32_t delta_ms) {
    return flight_state_machine.transitionTo(FlightState::PREFLIGHT_CHECK);
}

hal_status_t DroneApp::handlePreflightCheck(uint32_t delta_ms) {
    static uint32_t check_start_time = 0;
    
    if (check_start_time == 0) {
        check_start_time = millis();
        LOG_INFO("DroneFC", "Starting preflight checks");
    }
    
    if (millis() - check_start_time > 2000) {
        hal_status_t status = performPreflightCheck();
        if (status == HAL_OK) {
            return flight_state_machine.transitionTo(FlightState::IDLE);
        } else {
            return flight_state_machine.transitionTo(FlightState::ERROR);
        }
    }
    
    return HAL_OK;
}

hal_status_t DroneApp::handleIdleState(uint32_t delta_ms) {
    static uint32_t last_status = 0;
    
    if (millis() - last_status > 5000) {
        LOG_INFO("DroneFC", "System idle - Ready for arming");
        last_status = millis();
    }
    
    return HAL_OK;
}

hal_status_t DroneApp::handleArmingState(uint32_t delta_ms) {
    LOG_INFO("DroneFC", "Arming motors...");
    
    if (!checkSafetyConditions()) {
        LOG_ERROR("DroneFC", "Safety check failed - cannot arm");
        return flight_state_machine.transitionTo(FlightState::IDLE);
    }
    
    flight_data.motors_armed = true;
    flight_data.arm_time = millis();
    
    LOG_INFO("DroneFC", "Motors ARMED");
    return flight_state_machine.transitionTo(FlightState::ARMED);
}

hal_status_t DroneApp::handleArmedState(uint32_t delta_ms) {
    static uint32_t last_status = 0;
    
    if (millis() - last_status > 2000) {
        LOG_INFO("DroneFC", "Armed - Ready for takeoff");
        last_status = millis();
    }
    
    if (millis() - flight_data.arm_time > 30000) {
        LOG_WARNING("DroneFC", "Auto-disarm after 30s idle");
        flight_data.motors_armed = false;
        return flight_state_machine.transitionTo(FlightState::IDLE);
    }
    
    return HAL_OK;
}

hal_status_t DroneApp::handleTakeoffState(uint32_t delta_ms) {
    LOG_INFO("DroneFC", "Taking off...");
    flight_data.flight_time = millis();
    
    return flight_state_machine.transitionTo(FlightState::HOVER);
}

hal_status_t DroneApp::handleHoverState(uint32_t delta_ms) {
    static uint32_t hover_start = 0;
    
    if (hover_start == 0) {
        hover_start = millis();
        LOG_INFO("DroneFC", "Hovering");
    }
    
    if (millis() - hover_start > 5000) {
        return flight_state_machine.transitionTo(FlightState::FLYING);
    }
    
    return HAL_OK;
}

hal_status_t DroneApp::handleFlyingState(uint32_t delta_ms) {
    static uint32_t last_telemetry = 0;
    
    if (millis() - last_telemetry > 1000) {
        uint32_t flight_duration = (millis() - flight_data.flight_time) / 1000;
        LOG_INFO("DroneFC", "Flying - Duration: %lus", flight_duration);
        last_telemetry = millis();
    }
    
    return HAL_OK;
}

hal_status_t DroneApp::handleLandingState(uint32_t delta_ms) {
    LOG_INFO("DroneFC", "Landing...");
    
    flight_data.motors_armed = false;
    
    uint32_t total_flight_time = (millis() - flight_data.flight_time) / 1000;
    LOG_INFO("DroneFC", "Landed - Total flight time: %lus", total_flight_time);
    
    return flight_state_machine.transitionTo(FlightState::IDLE);
}

hal_status_t DroneApp::handleEmergencyState(uint32_t delta_ms) {
    static bool emergency_logged = false;
    
    if (!emergency_logged) {
        LOG_CRITICAL("DroneFC", "EMERGENCY STOP ACTIVATED");
        emergencyStop();
        emergency_logged = true;
    }
    
    digitalWrite(LED_BUILTIN, (millis() / 100) % 2);
    
    return HAL_OK;
}

hal_status_t DroneApp::handleErrorState(uint32_t delta_ms) {
    static uint32_t last_error_log = 0;
    
    if (millis() - last_error_log > 5000) {
        LOG_ERROR("DroneFC", "System in ERROR state");
        last_error_log = millis();
    }
    
    digitalWrite(LED_BUILTIN, (millis() / 500) % 2);
    
    return HAL_OK;
}

void DroneApp::updateTelemetry() {
    static uint32_t last_telemetry_update = 0;
    
    if (millis() - last_telemetry_update < 100) {
        return;
    }
    
    flight_data.battery_voltage = 3.7f + (random(0, 20) - 10) * 0.01f;
    flight_data.signal_strength = 80 + random(0, 20);
    
    last_telemetry_update = millis();
}

void DroneApp::sendHeartbeat() {
    static uint32_t last_heartbeat = 0;
    
    if (millis() - last_heartbeat > Constants::Timing::HEARTBEAT_INTERVAL_MS) {
        LOG_DEBUG("DroneFC", "Heartbeat - State: %s, Armed: %s",
                  flight_state_machine.getCurrentStateName(),
                  flight_data.motors_armed ? "YES" : "NO");
        last_heartbeat = millis();
    }
}

bool DroneApp::checkSafetyConditions() {
    bool safe = true;
    
    if (flight_data.battery_voltage < 3.0f) {
        safe = false;
    }
    
    if (flight_data.signal_strength < 20) {
        safe = false;
    }
    
    if (flight_data.emergency_stop) {
        safe = false;
    }
    
    if (system_monitor && !system_monitor->isHealthy()) {
        safe = false;
    }
    
    return safe;
}

void DroneApp::emergencyStop() {
    LOG_CRITICAL("DroneFC", "Executing emergency stop");
    flight_data.motors_armed = false;
    flight_data.emergency_stop = true;
    
    digitalWrite(LED_BUILTIN, HIGH);
}