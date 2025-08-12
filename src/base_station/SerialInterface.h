#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../lib/Core/Logger.h"
#include "../../lib/HAL/Core/hal_types.h"

class SerialInterface {
public:
    static constexpr uint32_t DEFAULT_BAUDRATE = 115200;
    static constexpr uint8_t TX_PIN = 17;  // ESP32-S3 UART2 TX
    static constexpr uint8_t RX_PIN = 18;  // ESP32-S3 UART2 RX
    static constexpr size_t RX_BUFFER_SIZE = 512;
    static constexpr size_t TX_BUFFER_SIZE = 512;
    static constexpr size_t JSON_BUFFER_SIZE = 1024;
    
    SerialInterface();
    ~SerialInterface() = default;
    
    hal_status_t initialize();
    hal_status_t update();
    hal_status_t shutdown();
    
    hal_status_t sendTelemetry(uint32_t uptime_ms, uint32_t free_heap, float cpu_usage);
    hal_status_t sendStatus(const char* status);
    hal_status_t sendEvent(const char* event, const char* data);
    hal_status_t sendError(const char* message);
    hal_status_t sendHeartbeat();
    
    void setESPNowConnected(bool connected);
    void setRemoteDeviceCount(uint8_t count);
    void setSignalStrength(int8_t rssi);
    void setBatteryVoltage(float voltage);
    
private:
    HardwareSerial* serial_port;
    char rx_buffer[RX_BUFFER_SIZE];
    size_t rx_index;
    
    // Telemetry data cache
    bool esp_now_connected;
    uint8_t remote_devices;
    int8_t signal_strength;
    float battery_voltage;
    
    // Timing
    uint32_t last_heartbeat_ms;
    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 5000;
    
    hal_status_t processCommand(const char* cmd, JsonObject& params);
    hal_status_t handleGetStatus();
    hal_status_t handleGetTelemetry();
    hal_status_t handleReset();
    hal_status_t handlePing();
    
    hal_status_t sendJsonMessage(JsonDocument& doc);
    void processIncomingData();
    bool parseJsonCommand(const char* buffer);
};

#endif // SERIAL_INTERFACE_H