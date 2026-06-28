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
// This table is the single source of truth for both validation
// (`isLegalTransition`) and the decorative QStateMachine's transition graph,
// so the two cannot drift.
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
    buildMachine();
    machine_->start();
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

    // Drive the decorative QStateMachine mirror (queued; fires entered/logging
    // on the next event-loop iteration in production).
    emit (this->*signalFor(target))();

    return true;
}

auto SessionConnectionStateMachine::signalFor(SessionConnectionState target)
    -> void (SessionConnectionStateMachine::*)() {
    switch (target) {
    case SessionConnectionState::Disconnected: return &SessionConnectionStateMachine::gotoDisconnected;
    case SessionConnectionState::Connecting: return &SessionConnectionStateMachine::gotoConnecting;
    case SessionConnectionState::TransportConnected: return &SessionConnectionStateMachine::gotoTransportConnected;
    case SessionConnectionState::Connected: return &SessionConnectionStateMachine::gotoConnected;
    case SessionConnectionState::Disconnecting: return &SessionConnectionStateMachine::gotoDisconnecting;
    }
    return &SessionConnectionStateMachine::gotoDisconnected; // unreachable
}

void SessionConnectionStateMachine::buildMachine() {
    machine_ = new QStateMachine(this);
    disconnectedState_ = new QState(machine_);
    connectingState_ = new QState(machine_);
    transportConnectedState_ = new QState(machine_);
    connectedState_ = new QState(machine_);
    disconnectingState_ = new QState(machine_);
    machine_->setInitialState(disconnectedState_);

    QState* states[] = {
        disconnectedState_,
        connectingState_,
        transportConnectedState_,
        connectedState_,
        disconnectingState_,
    };

    // Wire the decorative mirror from the same legal-transition table used by
    // isLegalTransition, so the documented graph and validation agree.
    for (int from = 0; from < 5; ++from) {
        for (int to = 0; to < 5; ++to) {
            if (from == to) {
                continue;
            }
            const auto f = static_cast<SessionConnectionState>(from);
            const auto t = static_cast<SessionConnectionState>(to);
            if (isLegalTransition(f, t)) {
                states[from]->addTransition(this, signalFor(t), states[to]);
            }
        }
    }

    connect(disconnectedState_, &QState::entered, this,
            [] { spdlog::debug("SessionConnectionStateMachine: entered Disconnected"); });
    connect(connectingState_, &QState::entered, this,
            [] { spdlog::debug("SessionConnectionStateMachine: entered Connecting"); });
    connect(transportConnectedState_, &QState::entered, this,
            [] { spdlog::debug("SessionConnectionStateMachine: entered TransportConnected"); });
    connect(connectedState_, &QState::entered, this,
            [] { spdlog::debug("SessionConnectionStateMachine: entered Connected"); });
    connect(disconnectingState_, &QState::entered, this,
            [] { spdlog::debug("SessionConnectionStateMachine: entered Disconnecting"); });
}

} // namespace ui::application::modbus
