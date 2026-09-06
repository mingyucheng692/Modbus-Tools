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

void ConnectionManager::onRequestTimeout() {
    using CS = ConnectionStateMachine::State;
    // Only timeouts of an established session count toward eviction: while
    // connecting/reconnecting the connect attempt loop already owns the FSM
    // and its own failure handling.
    if (stateMachine_->currentState() != CS::Connected) {
        return;
    }
    const int threshold = config_->unresponsiveThreshold;
    if (threshold <= 0) {
        return; // feature disabled
    }
    ++consecutiveTimeoutCount_;
    if (consecutiveTimeoutCount_ < threshold) {
        return;
    }

    // Evict the half-open session: the peer is a black hole (no RST, no
    // response), so the socket outlives its usefulness even though the
    // kernel still reports it connected. Transition first so the channel
    // -lost handler (fired by close() on the channel owner thread) sees a
    // terminal state and stays a no-op.
    consecutiveTimeoutCount_ = 0;
    SPDLOG_WARN(
        "ConnectionManager: {} consecutive request timeouts — evicting "
        "half-open session target={}:{}",
        threshold,
        config_->ipAddress.toStdString(),
        config_->port);
    if (!transitionChecked(CS::Failed, "unresponsive")) {
        // A concurrent passive-loss transition already left Connected, which
        // by definition means the channel already reported Closed/Error —
        // the teardown this eviction would perform has happened.
        return;
    }
    channel_->close();
}

void ConnectionManager::onRequestSuccess() {
    consecutiveTimeoutCount_ = 0;
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

void ConnectionManager::assertCleanEntryState() {
    using CS = ConnectionStateMachine::State;
    const auto current = stateMachine_->currentState();
    switch (current) {
    case CS::Disconnected:
    case CS::Failed:
        // Clean attempt-loop entry states: zero correction needed.
        return;
    case CS::Connecting:
    case CS::Reconnecting:
        // A previous attempt was abandoned mid-flight without proper abort cleanup.
        // In normal execution, abort() guarantees terminal Failed state.
        SPDLOG_ERROR("ConnectionManager::assertCleanEntryState: unexpected in-flight state {}, forcing self-heal to Failed",
                     ConnectionStateMachine::toString(current));
        transitionChecked(CS::Failed, "assert-clean-heal-abandoned");
        return;
    case CS::Connected:
        // Channel is closed but the FSM still claims Connected: the passive
        // loss the state handler normally reports was missed. Route through
        // Failed so the retry can legally proceed.
        SPDLOG_ERROR("ConnectionManager::assertCleanEntryState: channel closed but state is Connected, forcing self-heal to Failed");
        transitionChecked(CS::Failed, "assert-clean-heal-stale");
        return;
    case CS::Disconnecting:
        SPDLOG_ERROR("ConnectionManager::assertCleanEntryState: unexpected Disconnecting state, forcing self-heal to Disconnected");
        transitionChecked(CS::Disconnected, "assert-clean-heal-disconnecting");
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

    // Defensive check of entry state (assert clean state or self-heal).
    assertCleanEntryState();

    const int attempts = allowReconnect ? std::max(1, config_->retries + 1) : 1;
    QString connectError;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        clearError();

        const CS attemptState = (attempt == 0) ? CS::Connecting : CS::Reconnecting;
        if (!transitionChecked(attemptState,
                               attempt == 0 ? "connect-attempt" : "reconnect-attempt")) {
            // A concurrent transition (channel-lost handler on a foreign
            // thread) raced us; retry once from the normalized state.
            assertCleanEntryState();
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
                consecutiveTimeoutCount_ = 0; // fresh session
                transitionChecked(CS::Connected, "connect-success");
                connectFailureLogged_ = false; // 成功建立连接，重置 Latch
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
    if (!connectFailureLogged_) {
        if (channel_ && channel_->kind() == io::ChannelKind::Serial) {
            SPDLOG_WARN("ModbusClient: connect failed port={} reason={} channelState={}",
                        config_->portName.toStdString(),
                        lastChannelError_.toStdString(),
                        static_cast<int>(channel_->state()));
        } else {
            SPDLOG_WARN("ModbusClient: connect failed target={}:{} reason={} channelState={}",
                        config_->ipAddress.toStdString(),
                        config_->port,
                        lastChannelError_.toStdString(),
                        static_cast<int>(channel_ ? channel_->state() : io::ChannelState::Closed));
        }
        connectFailureLogged_ = true; // 锁存：轮询高频失败时完全静默，消除刷盘与 fmt 格式化 CPU 开销
    }
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
