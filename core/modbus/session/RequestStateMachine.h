/**
 * @file RequestStateMachine.h
 * @brief Compile-time validated state machine for ModbusClient request lifecycle.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <array>
#include <cstddef>

#include "StateMachineBase.h"

namespace modbus::session {

class RequestStateMachine : public StateMachineBase<RequestStateMachine> {
public:
    enum class State {
        Idle,
        Sending,
        Waiting,
        Completed,
        Failed,
        Aborted
    };

    struct Transition {
        State from;
        State to;
    };

    RequestStateMachine() = default;

    static constexpr const char* toString(State s) {
        switch (s) {
        case State::Idle:      return "Idle";
        case State::Sending:   return "Sending";
        case State::Waiting:   return "Waiting";
        case State::Completed: return "Completed";
        case State::Failed:    return "Failed";
        case State::Aborted:   return "Aborted";
        }
        return "Unknown";
    }

    static constexpr std::size_t kStateCount = 6;

    static constexpr const char* kName = "RequestStateMachine";
    static constexpr State kInitialState = State::Idle;

private:
    friend class StateMachineBase<RequestStateMachine>;

    static constexpr std::array kValidTransitions = {
        Transition{State::Idle,      State::Sending},
        Transition{State::Idle,      State::Failed},
        Transition{State::Idle,      State::Aborted},
        Transition{State::Sending,   State::Waiting},
        Transition{State::Sending,   State::Failed},
        Transition{State::Sending,   State::Aborted},
        Transition{State::Waiting,   State::Completed},
        Transition{State::Waiting,   State::Failed},
        Transition{State::Waiting,   State::Aborted},
        Transition{State::Failed,    State::Sending},
        Transition{State::Failed,    State::Idle},
        Transition{State::Completed, State::Idle},
        Transition{State::Aborted,   State::Idle},
    };
};

static_assert(RequestStateMachine::kStateCount == 6,
    "toString() must cover all RequestState values");

} // namespace modbus::session
