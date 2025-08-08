#include "ESPNowManager.h"

ESPNowManager* ESPNowManager::instance = nullptr;

ESPNowManager::ESPNowManager(ESPNowConfig::DeviceRole role, const uint8_t* peer_mac)
    : device_role(role)
    , current_state(State::UNINITIALIZED)
    , state_machine("ESPNow")
    , message_sequence(0)
    , ping_counter(0)
    , last_activity_time(0)
    , state_timer(0)
    , peer_added(false)
    , is_initialized(false) {
    
    memcpy(peer_mac_address, peer_mac, 6);
    memset(&stats, 0, sizeof(stats));
    memset(own_mac_address, 0, 6);
    instance = this;
}

ESPNowManager::~ESPNowManager() {
    shutdown();
    instance = nullptr;
}

hal_status_t ESPNowManager::init() {
    LOG_INFO("ESPNow", "Initializing ESP-NOW");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    WiFi.macAddress(own_mac_address);
    LOG_INFO("ESPNow", "Own MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             own_mac_address[0], own_mac_address[1], own_mac_address[2],
             own_mac_address[3], own_mac_address[4], own_mac_address[5]);
    LOG_INFO("ESPNow", "Peer MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             peer_mac_address[0], peer_mac_address[1], peer_mac_address[2],
             peer_mac_address[3], peer_mac_address[4], peer_mac_address[5]);
    
    if (esp_now_init() != ESP_OK) {
        LOG_ERROR("ESPNow", "Failed to initialize ESP-NOW");
        return HAL_ERROR;
    }
    
    esp_now_register_recv_cb(onDataReceived);
    esp_now_register_send_cb(onDataSent);
    
    state_machine.registerState(State::UNINITIALIZED, "UNINITIALIZED",
        [this](uint32_t dt) { return handleUninitialized(dt); });
    state_machine.registerState(State::SEARCHING, "SEARCHING",
        [this](uint32_t dt) { return handleSearching(dt); });
    state_machine.registerState(State::PAIRING, "PAIRING",
        [this](uint32_t dt) { return handlePairing(dt); });
    state_machine.registerState(State::PAIRED, "PAIRED",
        [this](uint32_t dt) { return handlePaired(dt); });
    state_machine.registerState(State::ERROR, "ERROR",
        [this](uint32_t dt) { return handleError(dt); });
    
    is_initialized = true;
    transitionToState(State::SEARCHING);
    
    return HAL_OK;
}

hal_status_t ESPNowManager::update(uint32_t delta_ms) {
    if (!is_initialized) return HAL_ERROR;
    
    state_timer += delta_ms;
    return state_machine.update(delta_ms);
}

hal_status_t ESPNowManager::shutdown() {
    if (!is_initialized) return HAL_OK;
    
    removePeer();
    esp_now_deinit();
    is_initialized = false;
    
    return HAL_OK;
}

hal_status_t ESPNowManager::handleUninitialized(uint32_t delta_ms) {
    return HAL_OK;
}

hal_status_t ESPNowManager::handleSearching(uint32_t delta_ms) {
    static uint32_t last_announce = 0;
    
    // Only base station broadcasts announcements
    if (device_role == ESPNowConfig::ROLE_BASE_STATION) {
        if (millis() - last_announce > ESPNowConfig::SEARCH_INTERVAL_MS) {
            sendAnnounce();
            last_announce = millis();
            LOG_DEBUG("ESPNow", "Base station searching for peer...");
        }
    } else {
        // Handheld just listens
        if (state_timer % 2000 == 0) {
            LOG_DEBUG("ESPNow", "Handheld listening for base station...");
        }
    }
    
    if (state_timer > ESPNowConfig::PAIRING_TIMEOUT_MS * 3) {
        LOG_WARNING("ESPNow", "Search timeout, restarting");
        state_timer = 0;
    }
    
    return HAL_OK;
}

hal_status_t ESPNowManager::handlePairing(uint32_t delta_ms) {
    static uint32_t last_retry = 0;
    
    // Reduced timeout for faster recovery
    if (state_timer > 3000) {
        LOG_WARNING("ESPNow", "Pairing timeout, back to searching");
        removePeer();
        transitionToState(State::SEARCHING);
        return HAL_OK;
    }
    
    // Only handheld should be in PAIRING state waiting for response
    if (device_role == ESPNowConfig::ROLE_HANDHELD) {
        if (millis() - last_retry > 1000) {
            LOG_DEBUG("ESPNow", "Handheld waiting for pair response...");
            last_retry = millis();
        }
    }
    
    return HAL_OK;
}

hal_status_t ESPNowManager::handlePaired(uint32_t delta_ms) {
    static uint32_t last_ping = 0;
    
    if (device_role == ESPNowConfig::ROLE_BASE_STATION) {
        if (millis() - last_ping > ESPNowConfig::PING_INTERVAL_MS) {
            if (sendPing() == HAL_OK) {
                last_ping = millis();
            } else {
                LOG_ERROR("ESPNow", "Failed to send ping");
            }
        }
    }
    
    if (millis() - last_activity_time > ESPNowConfig::CONNECTION_TIMEOUT_MS) {
        LOG_WARNING("ESPNow", "Connection timeout after %u ms of inactivity", 
                    millis() - last_activity_time);
        removePeer();
        transitionToState(State::SEARCHING);
    }
    
    return HAL_OK;
}

hal_status_t ESPNowManager::handleError(uint32_t delta_ms) {
    if (state_timer > 5000) {
        transitionToState(State::SEARCHING);
    }
    return HAL_OK;
}

hal_status_t ESPNowManager::sendMessage(const ESPNowMessage& msg) {
    if (!peer_added) {
        LOG_DEBUG("ESPNow", "Adding peer before sending message");
        if (addPeer() != HAL_OK) {
            LOG_ERROR("ESPNow", "Failed to add peer for sending");
            return HAL_ERROR;
        }
    }
    
    LOG_DEBUG("ESPNow", "Sending message type %d to %02X:%02X:%02X:%02X:%02X:%02X",
              msg.type, peer_mac_address[0], peer_mac_address[1], peer_mac_address[2],
              peer_mac_address[3], peer_mac_address[4], peer_mac_address[5]);
    
    esp_err_t result = esp_now_send(peer_mac_address, (uint8_t*)&msg, sizeof(ESPNowMessage));
    if (result == ESP_OK) {
        stats.messages_sent++;
        return HAL_OK;
    }
    
    LOG_ERROR("ESPNow", "Send failed with error: %d", result);
    return HAL_ERROR;
}

hal_status_t ESPNowManager::sendAnnounce() {
    ESPNowMessage msg;
    msg.type = ESPNowConfig::MSG_ANNOUNCE;
    msg.role = device_role;
    msg.sequence = message_sequence++;
    msg.timestamp = millis();
    
    uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, broadcast_mac, 6);
    peer_info.channel = ESPNowConfig::CHANNEL;
    peer_info.encrypt = false;
    
    if (!esp_now_is_peer_exist(broadcast_mac)) {
        esp_now_add_peer(&peer_info);
    }
    
    esp_err_t result = esp_now_send(broadcast_mac, (uint8_t*)&msg, sizeof(ESPNowMessage));
    
    if (!esp_now_is_peer_exist(broadcast_mac)) {
        esp_now_del_peer(broadcast_mac);
    }
    
    return (result == ESP_OK) ? HAL_OK : HAL_ERROR;
}

hal_status_t ESPNowManager::sendPairRequest() {
    ESPNowMessage msg;
    msg.type = ESPNowConfig::MSG_PAIR_REQUEST;
    msg.role = device_role;
    msg.sequence = message_sequence++;
    msg.timestamp = millis();
    
    return sendMessage(msg);
}

hal_status_t ESPNowManager::sendPairResponse() {
    ESPNowMessage msg;
    msg.type = ESPNowConfig::MSG_PAIR_RESPONSE;
    msg.role = device_role;
    msg.sequence = message_sequence++;
    msg.timestamp = millis();
    
    return sendMessage(msg);
}

hal_status_t ESPNowManager::sendPing() {
    ESPNowMessage msg;
    msg.type = ESPNowConfig::MSG_PING;
    msg.role = device_role;
    msg.sequence = message_sequence++;
    msg.timestamp = millis();
    msg.setPingData(ping_counter++);
    
    stats.ping_count++;
    stats.last_ping_time = millis();
    
    return sendMessage(msg);
}

hal_status_t ESPNowManager::sendPong(uint32_t counter) {
    ESPNowMessage msg;
    msg.type = ESPNowConfig::MSG_PONG;
    msg.role = device_role;
    msg.sequence = message_sequence++;
    msg.timestamp = millis();
    msg.setPongData(counter);
    
    stats.pong_count++;
    stats.last_pong_time = millis();
    
    return sendMessage(msg);
}

hal_status_t ESPNowManager::processMessage(const uint8_t* sender_mac, const ESPNowMessage* msg) {
    if (!msg->isValid()) {
        LOG_WARNING("ESPNow", "Invalid message received");
        return HAL_ERROR;
    }
    
    last_activity_time = millis();
    stats.messages_received++;
    
    switch (msg->type) {
        case ESPNowConfig::MSG_ANNOUNCE:
            if (current_state == State::SEARCHING && isMacEqual(sender_mac, peer_mac_address)) {
                LOG_INFO("ESPNow", "Peer announcement received from %02X:%02X:%02X:%02X:%02X:%02X",
                         sender_mac[0], sender_mac[1], sender_mac[2], 
                         sender_mac[3], sender_mac[4], sender_mac[5]);
                
                // Only handheld responds to announcements
                if (device_role == ESPNowConfig::ROLE_HANDHELD) {
                    // Add peer first
                    if (addPeer() == HAL_OK) {
                        LOG_INFO("ESPNow", "Handheld sending pair request");
                        sendPairRequest();
                        transitionToState(State::PAIRING);
                    }
                }
                // Base station stays in SEARCHING until it gets a pair request
            }
            break;
            
        case ESPNowConfig::MSG_PAIR_REQUEST:
            // Base station receives pair request while SEARCHING
            if (device_role == ESPNowConfig::ROLE_BASE_STATION) {
                if (current_state == State::SEARCHING && isMacEqual(sender_mac, peer_mac_address)) {
                    LOG_INFO("ESPNow", "Base station received pair request from %02X:%02X:%02X:%02X:%02X:%02X",
                             sender_mac[0], sender_mac[1], sender_mac[2],
                             sender_mac[3], sender_mac[4], sender_mac[5]);
                    if (addPeer() == HAL_OK) {
                        LOG_INFO("ESPNow", "Base station sending pair response and transitioning to PAIRED");
                        sendPairResponse();
                        transitionToState(State::PAIRED);
                        // Send immediate ping to confirm
                        sendPing();
                    }
                }
            }
            break;
            
        case ESPNowConfig::MSG_PAIR_RESPONSE:
            if (device_role == ESPNowConfig::ROLE_HANDHELD && current_state == State::PAIRING) {
                if (isMacEqual(sender_mac, peer_mac_address)) {
                    LOG_INFO("ESPNow", "Handheld received pair response from base, connection established");
                    transitionToState(State::PAIRED);
                }
            }
            break;
            
        case ESPNowConfig::MSG_PING:
            if (current_state == State::PAIRED) {
                uint32_t counter = msg->getPingPongCounter();
                sendPong(counter);
                LOG_DEBUG("ESPNow", "Ping %u received, sending pong", counter);
            }
            break;
            
        case ESPNowConfig::MSG_PONG:
            if (current_state == State::PAIRED) {
                uint32_t counter = msg->getPingPongCounter();
                stats.latency_ms = millis() - stats.last_ping_time;
                LOG_DEBUG("ESPNow", "Pong %u received, latency: %u ms", counter, stats.latency_ms);
            }
            break;
            
        default:
            LOG_WARNING("ESPNow", "Unknown message type: %d", msg->type);
            break;
    }
    
    return HAL_OK;
}

hal_status_t ESPNowManager::addPeer() {
    if (peer_added) return HAL_OK;
    
    // Check if peer already exists
    if (esp_now_is_peer_exist(peer_mac_address)) {
        peer_added = true;
        return HAL_OK;
    }
    
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, peer_mac_address, 6);
    peer_info.channel = ESPNowConfig::CHANNEL;
    peer_info.encrypt = false;
    
    esp_err_t result = esp_now_add_peer(&peer_info);
    if (result != ESP_OK) {
        LOG_ERROR("ESPNow", "Failed to add peer: %d", result);
        return HAL_ERROR;
    }
    
    LOG_INFO("ESPNow", "Peer added successfully");
    peer_added = true;
    return HAL_OK;
}

hal_status_t ESPNowManager::removePeer() {
    if (!peer_added) return HAL_OK;
    
    esp_now_del_peer(peer_mac_address);
    peer_added = false;
    return HAL_OK;
}

bool ESPNowManager::isMacEqual(const uint8_t* mac1, const uint8_t* mac2) const {
    return memcmp(mac1, mac2, 6) == 0;
}

void ESPNowManager::transitionToState(State new_state) {
    const char* old_state = getStateString();
    current_state = new_state;
    state_machine.transitionTo(new_state);
    
    LOG_INFO("ESPNow", "[%s] State: %s -> %s", 
             (device_role == ESPNowConfig::ROLE_BASE_STATION) ? "BASE" : "HANDHELD",
             old_state, getStateString());
    
    if (new_state == State::SEARCHING) {
        removePeer();
        state_timer = 0;
        last_activity_time = millis();
    } else if (new_state == State::PAIRING) {
        state_timer = 0;
        last_activity_time = millis();
    } else if (new_state == State::PAIRED) {
        ping_counter = 0;
        stats.ping_count = 0;
        stats.pong_count = 0;
        last_activity_time = millis();
        LOG_INFO("ESPNow", "Connection established with peer!");
    }
    
    state_timer = 0;
}

const char* ESPNowManager::getStateString() const {
    switch (current_state) {
        case State::UNINITIALIZED: return "UNINITIALIZED";
        case State::SEARCHING: return "SEARCHING";
        case State::PAIRING: return "PAIRING";
        case State::PAIRED: return "PAIRED";
        case State::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void ESPNowManager::getMacAddress(uint8_t* mac) const {
    memcpy(mac, own_mac_address, 6);
}

void ESPNowManager::getPeerMacAddress(uint8_t* mac) const {
    memcpy(mac, peer_mac_address, 6);
}

void ESPNowManager::onDataReceived(const uint8_t* mac_addr, const uint8_t* data, int len) {
    if (!instance || len != sizeof(ESPNowMessage)) return;
    
    const ESPNowMessage* msg = reinterpret_cast<const ESPNowMessage*>(data);
    instance->processMessage(mac_addr, msg);
}

void ESPNowManager::onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
    if (!instance) return;
    
    if (status != ESP_NOW_SEND_SUCCESS) {
        LOG_WARNING("ESPNow", "Send failed to %02X:%02X:%02X:%02X:%02X:%02X",
                    mac_addr[0], mac_addr[1], mac_addr[2],
                    mac_addr[3], mac_addr[4], mac_addr[5]);
    }
}