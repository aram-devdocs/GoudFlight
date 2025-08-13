#include "SerialInterface.h"
#include <HardwareSerial.h>

SerialInterface::SerialInterface() 
    : serial_port(&Serial2),
      rx_index(0),
      esp_now_connected(false),
      remote_devices(0),
      signal_strength(-100),
      battery_voltage(0.0f),
      last_heartbeat_ms(0) {
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
}

hal_status_t SerialInterface::initialize() {
    LOG_INFO("SerialInterface", "Initializing UART interface on pins TX:%d RX:%d", TX_PIN, RX_PIN);
    
    // Initialize UART2 with specified pins
    serial_port->begin(DEFAULT_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    serial_port->setTimeout(100);
    
    // Clear any pending data
    while (serial_port->available()) {
        serial_port->read();
    }
    
    // Send initial status
    sendStatus("INITIALIZED");
    
    LOG_INFO("SerialInterface", "Serial interface initialized at %d baud", DEFAULT_BAUDRATE);
    return HAL_OK;
}

hal_status_t SerialInterface::update() {
    // Process incoming commands
    processIncomingData();
    
    // Send periodic heartbeat
    uint32_t current_ms = millis();
    if (current_ms - last_heartbeat_ms >= HEARTBEAT_INTERVAL_MS) {
        sendHeartbeat();
        last_heartbeat_ms = current_ms;
    }
    
    return HAL_OK;
}

hal_status_t SerialInterface::shutdown() {
    LOG_INFO("SerialInterface", "Shutting down serial interface");
    
    sendStatus("SHUTDOWN");
    serial_port->end();
    
    return HAL_OK;
}

void SerialInterface::processIncomingData() {
    while (serial_port->available()) {
        char c = serial_port->read();
        
        if (c == '\n' || c == '\r') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0';
                parseJsonCommand(rx_buffer);
                rx_index = 0;
            }
        } else if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = c;
        } else {
            // Buffer overflow, reset
            LOG_WARNING("SerialInterface", "RX buffer overflow, resetting");
            rx_index = 0;
        }
    }
}

bool SerialInterface::parseJsonCommand(const char* buffer) {
    StaticJsonDocument<JSON_BUFFER_SIZE> doc;
    DeserializationError error = deserializeJson(doc, buffer);
    
    if (error) {
        LOG_WARNING("SerialInterface", "JSON parse error: %s", error.c_str());
        sendError("Invalid JSON format");
        return false;
    }
    
    const char* cmd = doc["cmd"];
    if (!cmd) {
        sendError("Missing 'cmd' field");
        return false;
    }
    
    JsonObject params = doc["params"];
    hal_status_t status = processCommand(cmd, params);
    
    return (status == HAL_OK);
}

hal_status_t SerialInterface::processCommand(const char* cmd, JsonObject& params) {
    LOG_DEBUG("SerialInterface", "Processing command: %s", cmd);
    
    if (strcmp(cmd, "GET_STATUS") == 0) {
        return handleGetStatus();
    } else if (strcmp(cmd, "GET_TELEMETRY") == 0) {
        return handleGetTelemetry();
    } else if (strcmp(cmd, "RESET") == 0) {
        return handleReset();
    } else if (strcmp(cmd, "PING") == 0) {
        return handlePing();
    } else {
        LOG_WARNING("SerialInterface", "Unknown command: %s", cmd);
        sendError("Unknown command");
        return HAL_ERROR;
    }
}

hal_status_t SerialInterface::handleGetStatus() {
    return sendStatus("RUNNING");
}

hal_status_t SerialInterface::handleGetTelemetry() {
    uint32_t uptime = millis();
    uint32_t free_heap = ESP.getFreeHeap();
    float cpu_usage = 0.0f;  // Placeholder - implement actual CPU monitoring if needed
    
    return sendTelemetry(uptime, free_heap, cpu_usage);
}

hal_status_t SerialInterface::handleReset() {
    LOG_INFO("SerialInterface", "Reset command received");
    sendStatus("RESETTING");
    delay(100);
    ESP.restart();
    return HAL_OK;
}

hal_status_t SerialInterface::handlePing() {
    StaticJsonDocument<256> doc;
    doc["type"] = "PONG";
    doc["timestamp"] = millis();
    
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendTelemetry(uint32_t uptime_ms, uint32_t free_heap, float cpu_usage) {
    StaticJsonDocument<JSON_BUFFER_SIZE> doc;
    doc["type"] = "TELEMETRY";
    
    JsonObject data = doc.createNestedObject("data");
    data["uptime_ms"] = uptime_ms;
    data["free_heap"] = free_heap;
    data["cpu_usage"] = cpu_usage;
    data["esp_now_connected"] = esp_now_connected;
    data["remote_devices"] = remote_devices;
    data["signal_strength"] = signal_strength;
    data["battery_voltage"] = battery_voltage;
    
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendStatus(const char* status) {
    StaticJsonDocument<256> doc;
    doc["type"] = "STATUS";
    doc["status"] = status;
    doc["timestamp"] = millis();
    
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendEvent(const char* event, const char* data) {
    StaticJsonDocument<512> doc;
    doc["type"] = "EVENT";
    doc["event"] = event;
    if (data) {
        doc["data"] = data;
    }
    doc["timestamp"] = millis();
    
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendError(const char* message) {
    StaticJsonDocument<256> doc;
    doc["type"] = "ERROR";
    doc["message"] = message;
    doc["timestamp"] = millis();
    
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendHeartbeat() {
    StaticJsonDocument<128> doc;
    doc["type"] = "HEARTBEAT";
    doc["timestamp"] = millis();
    
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendButtonEvent(uint8_t buttonStates, uint32_t timestamp) {
    StaticJsonDocument<256> doc;
    doc["type"] = "BUTTON_EVENT";
    doc["button_states"] = buttonStates;
    
    // Decode button states into readable format
    JsonArray buttons = doc.createNestedArray("buttons");
    for (int i = 0; i < 8; i++) {
        buttons.add((buttonStates >> i) & 1);
    }
    
    doc["timestamp"] = timestamp;
    
    LOG_INFO("SerialInterface", "Button event: states=0x%02X", buttonStates);
    return sendJsonMessage(doc);
}

hal_status_t SerialInterface::sendJsonMessage(JsonDocument& doc) {
    size_t bytes_written = serializeJson(doc, *serial_port);
    serial_port->println();  // Add newline for message termination
    
    if (bytes_written == 0) {
        LOG_ERROR("SerialInterface", "Failed to send JSON message");
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

void SerialInterface::setESPNowConnected(bool connected) {
    // Only send event if status has changed
    if (esp_now_connected != connected) {
        esp_now_connected = connected;
        sendEvent("ESPNOW_STATUS", connected ? "CONNECTED" : "DISCONNECTED");
    }
}

void SerialInterface::setRemoteDeviceCount(uint8_t count) {
    remote_devices = count;
}

void SerialInterface::setSignalStrength(int8_t rssi) {
    signal_strength = rssi;
}

void SerialInterface::setBatteryVoltage(float voltage) {
    battery_voltage = voltage;
}