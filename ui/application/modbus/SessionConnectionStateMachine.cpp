/**
 * @file SessionConnectionStateMachine.cpp
 * @brief Validated connection state machine for ModbusSessionPresenter.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SessionConnectionStateMachine.h"
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

namespace {

struct TransitionRule {
    SessionConnectionState from;
    SessionConnectionState to;
};

// Legal cross-state transitions. Self-transitions (re-entering the current
// state) are always allowed for idempotent re-entry and are not listed here.
constexpr TransitionRule kLegalTransitions[] = {
    {SessionConnectionState::Disconnected, SessionConnectionState::Connecting},

    {SessionConnectionState::Connecting, SessionConnectionState::TransportConnected},
    {SessionConnectionState::Connecting, SessionConnectionState::Connected},
    {SessionConnectionState::Connecting, SessionConnectionState::Disconnected},
    {SessionConnectionState::Connecting, SessionConnectionState::Disconnecting},

    {SessionConnectionState::TransportConnected, SessionConnectionState::Connected},
    {SessionConnectionState::TransportConnected, SessionConnectionState::Disconnected},
    {SessionConnectionState::TransportConnected, SessionConnectionState::Disconnecting},

    {SessionConnectionState::Connected, SessionConnectionState::Disconnected},
    {SessionConnectionState::Connected, SessionConnectionState::Disconnecting},
    // A spurious channel-Open while already Connected must not be rejected
    // (the original code tolerated this); it re-affirms transport state.
    {SessionConnectionState::Connected, SessionConnectionState::TransportConnected},

    {SessionConnectionState::Disconnecting, SessionConnectionState::Disconnected},
};

constexpr const char* stateName(SessionConnectionState s) {
    switch (s) {
    case SessionConnectionState::Disconnected: return "Disconnected";
    case SessionConnectionState::Connecting: return "Connecting";
    case SessionConnectionState::TransportConnected: return "TransportConnected";
    case SessionConnectionState::Connected: return "Connected";
    case SessionConnectionState::Disconnecting: return "Disconnecting";
    }
    return "Unknown";
}

} // namespace

SessionConnectionStateMachine::SessionConnectionStateMachine(QObject* parent)
    : QObject(parent) {
}

bool SessionConnectionStateMachine::isLegalTransition(SessionConnectionState from,
                                                      SessionConnectionState to) {
    if (from == to) {
        return true; // idempotent re-entry
    }
    for (const auto& rule : kLegalTransitions) {
        if (rule.from == from && rule.to == to) {
            return true;
        }
    }
    return false;
}

bool SessionConnectionStateMachine::transitionTo(SessionConnectionState target) {
    if (target == state_) {
        return true; // no-op, no signal
    }

    if (!isLegalTransition(state_, target)) {
        spdlog::warn("SessionConnectionStateMachine: rejected illegal transition {} -> {}",
                     stateName(state_), stateName(target));
        return false;
    }

    spdlog::debug("SessionConnectionStateMachine: {} -> {}", stateName(state_), stateName(target));
    state_ = target;
    emit stateChanged(target); // synchronous (direct connection): side effects inline

    return true;
}

} // namespace ui::application::modbus
