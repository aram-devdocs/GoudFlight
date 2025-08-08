#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include "../HAL/Core/hal_types.h"
#include <functional>
#include <map>

class StateManager {
public:
    using StateId = uint16_t;
    using StateHandler = std::function<hal_status_t(uint32_t)>;
    using TransitionHandler = std::function<void(StateId, StateId)>;
    
    struct State {
        const char* name;
        StateHandler onEnter;
        StateHandler onUpdate;
        StateHandler onExit;
    };
    
    StateManager(const char* name);
    ~StateManager() = default;
    
    hal_status_t registerState(StateId id, const State& state);
    hal_status_t registerState(StateId id, const char* name, StateHandler onUpdate);
    
    hal_status_t transitionTo(StateId new_state);
    hal_status_t update(uint32_t delta_ms);
    
    StateId getCurrentState() const { return current_state; }
    StateId getPreviousState() const { return previous_state; }
    const char* getCurrentStateName() const;
    uint32_t getStateTime() const { return state_time; }
    
    void setTransitionHandler(TransitionHandler handler) { transition_handler = handler; }
    
    bool isInState(StateId state) const { return current_state == state; }
    bool hasState(StateId state) const;
    
    static constexpr StateId INVALID_STATE = 0xFFFF;
    
private:
    const char* manager_name;
    StateId current_state;
    StateId previous_state;
    StateId pending_state;
    uint32_t state_time;
    uint32_t last_update_time;
    bool in_transition;
    
    std::map<StateId, State> states;
    TransitionHandler transition_handler;
    
    hal_status_t executeTransition();
};

template<typename T>
class StateMachine {
public:
    StateMachine(const char* name) : manager(name), current_state_enum(static_cast<T>(0)) {}
    
    hal_status_t registerState(T state, const char* name, 
                               typename StateManager::StateHandler onUpdate) {
        return manager.registerState(static_cast<StateManager::StateId>(state), name, onUpdate);
    }
    
    hal_status_t registerState(T state, const StateManager::State& state_def) {
        return manager.registerState(static_cast<StateManager::StateId>(state), state_def);
    }
    
    hal_status_t transitionTo(T new_state) {
        current_state_enum = new_state;
        return manager.transitionTo(static_cast<StateManager::StateId>(new_state));
    }
    
    hal_status_t update(uint32_t delta_ms) {
        return manager.update(delta_ms);
    }
    
    T getCurrentState() const { return current_state_enum; }
    const char* getCurrentStateName() const { return manager.getCurrentStateName(); }
    uint32_t getStateTime() const { return manager.getStateTime(); }
    
    bool isInState(T state) const { 
        return static_cast<StateManager::StateId>(state) == manager.getCurrentState();
    }
    
private:
    StateManager manager;
    T current_state_enum;
};

#endif