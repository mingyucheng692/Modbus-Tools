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
#include <QStateMachine>
#include <QState>

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
 * (`currentState()`); a QStateMachine mirrors the cache for architectural
 * consistency with PollingController and provides state-entry logging in
 * production (its transitions are driven asynchronously via the Qt event
 * queue, mirroring the PollingController pattern).
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
     * logged as a warning. Legal transitions update the synchronous cache,
     * emit `stateChanged` (synchronous, so observers run their side effects
     * inline), and emit the internal goto signal that drives the decorative
     * QStateMachine mirror.
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
    // Internal triggers that drive the decorative QStateMachine mirror.
    // Public only because Qt signals cannot be made private; callers must
    // never emit these directly — use `transitionTo()`.
    void gotoDisconnected();
    void gotoConnecting();
    void gotoTransportConnected();
    void gotoConnected();
    void gotoDisconnecting();

private:
    void buildMachine();
    static auto signalFor(SessionConnectionState target)
        -> void (SessionConnectionStateMachine::*)();

    SessionConnectionState state_ = SessionConnectionState::Disconnected;

    // Decorative mirror; `state_` is authoritative. Matches the
    // PollingController pattern where a synchronous context/cache is the
    // source of truth and the QStateMachine provides logging + a documented
    // transition graph.
    QStateMachine* machine_ = nullptr;
    QState* disconnectedState_ = nullptr;
    QState* connectingState_ = nullptr;
    QState* transportConnectedState_ = nullptr;
    QState* connectedState_ = nullptr;
    QState* disconnectingState_ = nullptr;
};

} // namespace ui::application::modbus

// Required so QSignalSpy (and any queued connection) can capture the enum
// argument of SessionConnectionStateMachine::stateChanged.
Q_DECLARE_METATYPE(ui::application::modbus::SessionConnectionState)
