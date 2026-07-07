/**
 * @file SessionConnectionStateMachine.h
 * @brief Validated connection state machine for ModbusSessionPresenter.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>

namespace ui::application::modbus {

enum class SessionConnectionState {
    Disconnected,
    Connecting,
    TransportConnected,
    Connected,
    Disconnecting
};

/**
 * @brief Centralized, validated connection-state transitions.
 *
 * Replaces the scattered `connectionState_` writes that previously lived in
 * ModbusSessionPresenter. A synchronous cache (`state_`) is the authoritative
 * source of truth so callers can read the current state synchronously
 * (`currentState()`).
 *
 * `transitionTo()` validates every transition against a legal-transition
 * table and rejects illegal ones (returning false, leaving the state
 * unchanged). The `stateChanged` signal is emitted synchronously (direct
 * connection, same thread) so the presenter can apply synchronous side
 * effects (widget sync, alert-suppression flags) before control returns to
 * the caller.
 *
 * @thread Lives on the GUI thread. All methods must be called from the GUI
 *         thread. The cache is not guarded by a mutex because writes are
 *         serialized by the Qt event loop.
 */
class SessionConnectionStateMachine : public QObject {
    Q_OBJECT

public:
    explicit SessionConnectionStateMachine(QObject* parent = nullptr);

    /**
     * @brief Apply a validated transition to @p target.
     *
     * Illegal transitions are rejected (state unchanged, returns false) and
     * logged as a warning. Legal transitions update the synchronous cache
     * and emit `stateChanged` (synchronous, so observers run their side
     * effects inline).
     *
     * A no-op transition to the current state is a success and does not emit
     * `stateChanged` (idempotent re-entry).
     */
    bool transitionTo(SessionConnectionState target);

    SessionConnectionState currentState() const noexcept { return state_; }

    static bool isLegalTransition(SessionConnectionState from, SessionConnectionState to);

signals:
    /**
     * @brief Emitted synchronously from `transitionTo` after the cache is
     *        updated. Observers may read `currentState()` and apply
     *        synchronous side effects inline.
     */
    void stateChanged(SessionConnectionState newState);

private:
    SessionConnectionState state_ = SessionConnectionState::Disconnected;
};

} // namespace ui::application::modbus

// Required so QSignalSpy (and any queued connection) can capture the enum
// argument of SessionConnectionStateMachine::stateChanged.
Q_DECLARE_METATYPE(ui::application::modbus::SessionConnectionState)
