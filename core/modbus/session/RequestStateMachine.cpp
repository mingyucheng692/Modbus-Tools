/**
 * @file RequestStateMachine.cpp
 * @brief Implementation of RequestStateMachine.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RequestStateMachine.h"
#include <spdlog/spdlog.h>

namespace modbus::session {

bool RequestStateMachine::isValidTransition(State from, State to) {
    for (const auto& t : kValidTransitions) {
        if (t.from == from && t.to == to) {
            return true;
        }
    }
    return false;
}

bool RequestStateMachine::tryTransition(State newState, const char* reason) {
    State oldState = state_.load(std::memory_order_relaxed);

    while (true) {
        if (oldState == newState) {
            return true;
        }

        if (!isValidTransition(oldState, newState)) {
            spdlog::error("RequestStateMachine: invalid transition {} -> {} ({})",
                         toString(oldState), toString(newState),
                         reason ? reason : "");
            return false;
        }

        if (state_.compare_exchange_weak(oldState, newState,
                std::memory_order_acq_rel, std::memory_order_relaxed)) {
            spdlog::debug("RequestStateMachine: {} -> {} ({})",
                          toString(oldState), toString(newState),
                          reason ? reason : "");
            return true;
        }
    }
}

RequestStateMachine::State RequestStateMachine::currentState() const {
    return state_.load(std::memory_order_acquire);
}

void RequestStateMachine::forceReset(State newState) {
    state_.store(newState, std::memory_order_release);
}

} // namespace modbus::session
