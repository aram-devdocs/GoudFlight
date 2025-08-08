#include "StateManager.h"
#include "Logger.h"

StateManager::StateManager(const char* name)
    : manager_name(name)
    , current_state(INVALID_STATE)
    , previous_state(INVALID_STATE)
    , pending_state(INVALID_STATE)
    , state_time(0)
    , last_update_time(0)
    , in_transition(false)
    , transition_handler(nullptr) {
}

hal_status_t StateManager::registerState(StateId id, const State& state) {
    if (states.find(id) != states.end()) {
        LOG_WARNING(manager_name, "State %u already registered", id);
        return HAL_ERROR;
    }
    
    states[id] = state;
    LOG_DEBUG(manager_name, "Registered state %u: %s", id, state.name);
    return HAL_OK;
}

hal_status_t StateManager::registerState(StateId id, const char* name, StateHandler onUpdate) {
    State state = {
        .name = name,
        .onEnter = nullptr,
        .onUpdate = onUpdate,
        .onExit = nullptr
    };
    return registerState(id, state);
}

hal_status_t StateManager::transitionTo(StateId new_state) {
    if (in_transition) {
        LOG_WARNING(manager_name, "Already in transition, deferring to state %u", new_state);
        pending_state = new_state;
        return HAL_BUSY;
    }
    
    if (!hasState(new_state)) {
        LOG_ERROR(manager_name, "Invalid state transition to %u", new_state);
        return HAL_INVALID_PARAM;
    }
    
    if (current_state == new_state) {
        return HAL_OK;
    }
    
    in_transition = true;
    pending_state = new_state;
    
    hal_status_t status = executeTransition();
    
    in_transition = false;
    
    if (status == HAL_OK && pending_state != current_state && pending_state != INVALID_STATE) {
        StateId deferred = pending_state;
        pending_state = INVALID_STATE;
        return transitionTo(deferred);
    }
    
    pending_state = INVALID_STATE;
    return status;
}

hal_status_t StateManager::executeTransition() {
    if (current_state != INVALID_STATE) {
        auto current_it = states.find(current_state);
        if (current_it != states.end() && current_it->second.onExit) {
            LOG_DEBUG(manager_name, "Exiting state %s", current_it->second.name);
            hal_status_t status = current_it->second.onExit(state_time);
            if (status != HAL_OK) {
                LOG_ERROR(manager_name, "Failed to exit state %s", current_it->second.name);
                return status;
            }
        }
    }
    
    previous_state = current_state;
    current_state = pending_state;
    state_time = 0;
    last_update_time = millis();
    
    auto new_it = states.find(current_state);
    if (new_it != states.end()) {
        LOG_INFO(manager_name, "Transitioning to state %s", new_it->second.name);
        
        if (new_it->second.onEnter) {
            hal_status_t status = new_it->second.onEnter(0);
            if (status != HAL_OK) {
                LOG_ERROR(manager_name, "Failed to enter state %s", new_it->second.name);
                current_state = previous_state;
                return status;
            }
        }
        
        if (transition_handler) {
            transition_handler(previous_state, current_state);
        }
    }
    
    return HAL_OK;
}

hal_status_t StateManager::update(uint32_t delta_ms) {
    if (current_state == INVALID_STATE) {
        return HAL_NOT_INITIALIZED;
    }
    
    state_time += delta_ms;
    
    auto it = states.find(current_state);
    if (it != states.end() && it->second.onUpdate) {
        return it->second.onUpdate(delta_ms);
    }
    
    return HAL_OK;
}

const char* StateManager::getCurrentStateName() const {
    if (current_state == INVALID_STATE) {
        return "INVALID";
    }
    
    auto it = states.find(current_state);
    if (it != states.end()) {
        return it->second.name;
    }
    
    return "UNKNOWN";
}

bool StateManager::hasState(StateId state) const {
    return states.find(state) != states.end();
}