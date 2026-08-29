/**
 * @file ConnectionManager.cpp
 * @brief Implementation of ConnectionManager.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ConnectionManager.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include "common/TrContext.h"

namespace modbus::session {
namespace {
    // i18n helper — uses explicit context so lupdate can categorize strings
    constexpr char kConnManagerCtx[] = "modbus::session::ConnectionManager";
} // namespace

ConnectionManager::ConnectionManager(io::IChannel* channel,
                                     ConnectionStateMachine* stateMachine,
                                     std::atomic<bool>& aborted,
                                     RetryStrategy* retryStrategy,
                                     const base::ModbusConfig* config)
    : channel_(channel)
    , stateMachine_(stateMachine)
    , aborted_(aborted)
    , retryStrategy_(retryStrategy)
    , config_(config) {}

bool ConnectionManager::isConnected() const {
    return channel_ && channel_->isOpen();
}

QString ConnectionManager::lastChannelError() const {
    return lastChannelError_;
}

bool ConnectionManager::hasChannelError() const {
    return !lastChannelError_.isEmpty();
}

void ConnectionManager::clearError() {
    lastChannelError_.clear();
}

void ConnectionManager::setError(const QString& error) {
    lastChannelError_ = error;
}

bool ConnectionManager::transitionChecked(ConnectionStateMachine::State to, const char* reason) {
    if (!stateMachine_->tryTransition(to, reason)) {
        // tryTransition() already logged the invalid edge with states and
        // reason; this adds the manager context so the swallow point is
        // visible in production logs.
        SPDLOG_ERROR("ConnectionManager: transition to {} rejected (reason={}, current={})",
                     ConnectionStateMachine::toString(to),
                     reason ? reason : "",
                     ConnectionStateMachine::toString(stateMachine_->currentState()));
        return false;
    }
    return true;
}

void ConnectionManager::normalizeBeforeConnectAttempt() {
    using CS = ConnectionStateMachine::State;
    switch (stateMachine_->currentState()) {
    case CS::Disconnected:
    case CS::Failed:
        // Legal attempt-loop entry states: Disconnected -> Connecting and
        // Failed -> Connecting (user retry) are both valid edges.
        return;
    case CS::Connecting:
    case CS::Reconnecting:
        // A previous attempt was abandoned mid-flight (e.g. abort() without
        // a follow-up disconnect): route through Failed, which is a legal
        // entry state for the next attempt.
        transitionChecked(CS::Failed, "normalize-abandoned-attempt");
        return;
    case CS::Connected:
        // Channel is closed but the FSM still claims Connected: the passive
        // loss the state handler normally reports was missed. Route through
        // Failed so the retry can legally proceed.
        transitionChecked(CS::Failed, "normalize-stale-session");
        return;
    case CS::Disconnecting:
        transitionChecked(CS::Disconnected, "normalize");
        return;
    }
}

bool ConnectionManager::ensureConnected(bool allowReconnect) {
    using CS = ConnectionStateMachine::State;

    if (!channel_) {
        // Never-connected: Disconnected is the truthful state. There is no
        // Disconnected -> Failed edge, and inventing one would corrupt the
        // FSM — report via the error string instead.
        lastChannelError_ = TrContext<kConnManagerCtx>::tr("No channel attached");
        return false;
    }

    io::Timeouts timeouts;
    timeouts.readMs = config_->timeoutMs;
    timeouts.writeMs = config_->timeoutMs;
    channel_->setTimeouts(timeouts);

    if (channel_->isOpen()) {
        // Fast path: the channel is already open. Walk the FSM to Connected
        // through legal edges (a direct jump is illegal from
        // Disconnected/Failed and would previously be rejected and logged).
        switch (stateMachine_->currentState()) {
        case CS::Connected:
            return true;
        case CS::Connecting:
        case CS::Reconnecting:
            transitionChecked(CS::Connected, "already-open");
            return true;
        case CS::Disconnected:
            transitionChecked(CS::Connecting, "already-open-resume");
            transitionChecked(CS::Connected, "already-open");
            return true;
        case CS::Failed:
            transitionChecked(CS::Reconnecting, "already-open-resume");
            transitionChecked(CS::Connected, "already-open");
            return true;
        case CS::Disconnecting:
            // A teardown is mid-flight; it owns the FSM. The channel is open,
            // so report connected without fighting the disconnect.
            return true;
        }
        return true;
    }

    // Defensive normalization of stale states (see header comment).
    normalizeBeforeConnectAttempt();

    const int attempts = allowReconnect ? std::max(1, config_->retries + 1) : 1;
    QString connectError;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        clearError();

        const CS attemptState = (attempt == 0) ? CS::Connecting : CS::Reconnecting;
        if (!transitionChecked(attemptState,
                               attempt == 0 ? "connect-attempt" : "reconnect-attempt")) {
            // A concurrent transition (channel-lost handler on a foreign
            // thread) raced us; retry once from the normalized state.
            normalizeBeforeConnectAttempt();
            if (!transitionChecked(attemptState, "connect-attempt-retry")) {
                lastChannelError_ = TrContext<kConnManagerCtx>::tr("State machine busy");
                return false;
            }
        }

        if (!channel_->open()) {
            connectError = TrContext<kConnManagerCtx>::tr("Failed to dispatch channel open");
        } else {
            const auto deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(config_->timeoutMs);
            if (waitForChannelState(io::ChannelState::Open, deadline, &connectError)) {
                // Channel is genuinely open; if a channel-lost handler raced
                // the success detection the FSM self-heals on the next
                // already-open fast path.
                transitionChecked(CS::Connected, "connect-success");
                return true;
            }
        }

        channel_->close();
        transitionChecked(CS::Failed, "connect-failed");
        if (attempt + 1 >= attempts) {
            break;
        }

        RetryStrategy::Config reconnectBackoffCfg;
        reconnectBackoffCfg.baseIntervalMs = config_->reconnectBaseMs;
        reconnectBackoffCfg.maxIntervalMs = config_->reconnectMaxMs;
        reconnectBackoffCfg.backoffFactor = config_->retryBackoffFactor;
        reconnectBackoffCfg.jitterPercent = config_->retryJitterPercent;
        const int reconnectDelayMs = retryStrategy_->calculateBackoffMs(reconnectBackoffCfg, attempt);
        if (!waitForAbortableDelay(aborted_, std::chrono::milliseconds(reconnectDelayMs))) {
            lastChannelError_ = TrContext<kConnManagerCtx>::tr("Aborted");
            transitionChecked(CS::Failed, "reconnect-aborted");
            return false;
        }
    }

    // Preserve the most specific error already captured by waitForChannelState()
    // (e.g. "Aborted", channel error). Only synthesize a generic timeout message
    // when no specific reason was recorded, and never overwrite an existing error.
    if (lastChannelError_.isEmpty()) {
        lastChannelError_ = connectError.isEmpty() ? TrContext<kConnManagerCtx>::tr("Connect timeout") : connectError;
    }
    SPDLOG_WARN("ModbusClient: connect failed target={}:{} reason={} channelState={}",
                 config_->ipAddress.toStdString(),
                 config_->port,
                 lastChannelError_.toStdString(),
                 static_cast<int>(channel_->state()));
    return false;
}

bool ConnectionManager::waitForChannelState(io::ChannelState expectedState,
                                            std::chrono::steady_clock::time_point deadline,
                                            QString* errorOut) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (channel_->state() == expectedState
            || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
            return true;
        }
        if (hasChannelError()) {
            if (errorOut) {
                *errorOut = lastChannelError();
            }
            clearError();
            return false;
        }
        if (channel_->state() == io::ChannelState::Error) {
            if (errorOut && errorOut->isEmpty()) {
                *errorOut = TrContext<kConnManagerCtx>::tr("Channel entered error state");
            }
            return false;
        }

        waitForCondition([this, expectedState]() {
            return !lastChannelError_.isEmpty()
                || channel_->state() == io::ChannelState::Error
                || channel_->state() == expectedState
                || (expectedState == io::ChannelState::Open && channel_->isOpen());
        }, deadline);
    }

    if (channel_->state() == expectedState
        || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
        return true;
    }
    if (errorOut && errorOut->isEmpty()) {
        *errorOut = TrContext<kConnManagerCtx>::tr("Connect timeout");
    }
    return false;
}

} // namespace modbus::session
