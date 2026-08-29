/**
 * @file ConnectionStateMachine.h
 * @brief Compile-time validated state machine for ModbusClient connection lifecycle.
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

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Disconnecting,
    Failed
};

class ConnectionStateMachine : public StateMachineBase<ConnectionStateMachine, ConnectionState> {
public:
    using State = ConnectionState;

    struct Transition {
        State from;
        State to;
    };

    ConnectionStateMachine() = default;

    static constexpr const char* toString(State s) {
        switch (s) {
        case State::Disconnected:  return "Disconnected";
        case State::Connecting:    return "Connecting";
        case State::Connected:     return "Connected";
        case State::Reconnecting:  return "Reconnecting";
        case State::Disconnecting: return "Disconnecting";
        case State::Failed:        return "Failed";
        }
        return "Unknown";
    }

    static constexpr std::size_t kStateCount = 6;

    static constexpr const char* kName = "ConnectionStateMachine";
    static constexpr State kInitialState = State::Disconnected;

private:
    friend class StateMachineBase<ConnectionStateMachine, ConnectionState>;

    static constexpr std::array kValidTransitions = {
        Transition{State::Disconnected,  State::Connecting},
        Transition{State::Disconnected,  State::Disconnecting},
        Transition{State::Connecting,    State::Connected},
        Transition{State::Connecting,    State::Failed},
        Transition{State::Connecting,    State::Disconnecting},
        Transition{State::Connected,     State::Disconnecting},
        Transition{State::Connected,     State::Failed},
        Transition{State::Disconnecting, State::Disconnected},
        // Failed -> Connecting: a user-initiated retry from a failed session
        // (distinct from Failed -> Reconnecting, which is an automatic retry
        // inside a reconnect loop). Without this edge the machine gets stuck
        // in Failed after the first failed connect() and every subsequent
        // attempt logs an invalid transition.
        Transition{State::Failed,        State::Connecting},
        Transition{State::Failed,        State::Reconnecting},
        Transition{State::Failed,        State::Disconnecting},
        Transition{State::Failed,        State::Disconnected},
        Transition{State::Reconnecting,  State::Connected},
        Transition{State::Reconnecting,  State::Failed},
        Transition{State::Reconnecting,  State::Disconnecting},
    };
};

static_assert(ConnectionStateMachine::kStateCount == 6,
    "toString() must cover all ConnectionState values");

} // namespace modbus::session
