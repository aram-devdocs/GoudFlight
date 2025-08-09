#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Preferences.h>
#include "../../Core/StateManager.h"
#include "../../Core/Logger.h"
#include "../../HAL/Core/hal_types.h"
#include "ESPNowMessage.h"
#include "ESPNowConfig.h"

class ESPNowManager {
public:
    enum class State : uint16_t {
        UNINITIALIZED = 0,
        SEARCHING,
        PAIRING,
        PAIRED,
        RECONNECTING,
        ERROR
    };
    
    struct Stats {
        uint32_t messages_sent;
        uint32_t messages_received;
        uint32_t ping_count;
        uint32_t pong_count;
        uint32_t last_ping_time;
        uint32_t last_pong_time;
        uint32_t latency_ms;
        int8_t rssi;
    };
    
    ESPNowManager(ESPNowConfig::DeviceRole role, const uint8_t* peer_mac);
    ~ESPNowManager();
    
    hal_status_t init();
    hal_status_t update(uint32_t delta_ms);
    hal_status_t shutdown();
    hal_status_t startConnection();  // Manual connection start
    hal_status_t stopConnection();   // Manual connection stop
    bool isSearching() const { return current_state == State::SEARCHING; }
    
    hal_status_t sendPing();
    hal_status_t sendPong(uint32_t counter);
    hal_status_t sendDisconnect();
    hal_status_t disconnect();  // User-initiated disconnect
    hal_status_t sendMessage(const ESPNowMessage& msg);  // Made public for sync managers
    
    State getState() const { return current_state; }
    const char* getStateString() const;
    const Stats& getStats() const { return stats; }
    bool isPaired() const { return current_state == State::PAIRED; }
    
    void getMacAddress(uint8_t* mac) const;
    void getPeerMacAddress(uint8_t* mac) const;
    
    // Connection info methods
    uint32_t getConnectionUptime() const;
    uint32_t getLastActivityTime() const { return last_activity_time; }
    bool isConnected() const { return current_state == State::PAIRED; }
    bool isConnecting() const { return current_state == State::PAIRING || current_state == State::RECONNECTING; }
    float getPacketLossRate() const;
    
    // Message callbacks for sync managers
    typedef void (*MessageCallback)(const ESPNowMessage* msg);
    void setScreenSyncCallback(MessageCallback callback) { screen_sync_callback = callback; }
    void setButtonDataCallback(MessageCallback callback) { button_data_callback = callback; }
    void setInputEventCallback(MessageCallback callback) { input_event_callback = callback; }
    
    // Persistence methods
    bool loadSavedPeer();
    void savePeer();
    void clearSavedPeer();
    bool hasAutoReconnect() const;
    void setAutoReconnect(bool enabled);
    
private:
    ESPNowConfig::DeviceRole device_role;
    State current_state;
    uint8_t peer_mac_address[6];
    uint8_t own_mac_address[6];
    
    StateMachine<State> state_machine;
    Stats stats;
    
    uint32_t message_sequence;
    uint32_t ping_counter;
    uint32_t last_activity_time;
    uint32_t state_timer;
    uint32_t connection_start_time;
    
    bool peer_added;
    bool is_initialized;
    bool auto_reconnect;
    
    Preferences preferences;
    
    // Message callbacks
    MessageCallback screen_sync_callback;
    MessageCallback button_data_callback;
    MessageCallback input_event_callback;
    
    static ESPNowManager* instance;
    static void onDataReceived(const uint8_t* mac_addr, const uint8_t* data, int len);
    static void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
    
    hal_status_t handleUninitialized(uint32_t delta_ms);
    hal_status_t handleSearching(uint32_t delta_ms);
    hal_status_t handlePairing(uint32_t delta_ms);
    hal_status_t handlePaired(uint32_t delta_ms);
    hal_status_t handleReconnecting(uint32_t delta_ms);
    hal_status_t handleError(uint32_t delta_ms);
    
    hal_status_t processMessage(const uint8_t* sender_mac, const ESPNowMessage* msg);
    hal_status_t sendAnnounce();
    hal_status_t sendPairRequest();
    hal_status_t sendPairResponse();
    
    hal_status_t addPeer();
    hal_status_t removePeer();
    
    bool isMacEqual(const uint8_t* mac1, const uint8_t* mac2) const;
    void transitionToState(State new_state);
};

#endif