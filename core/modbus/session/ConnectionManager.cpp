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

bool ConnectionManager::ensureConnected(bool allowReconnect) {
    if (!channel_) {
        lastChannelError_ = TrContext<kConnManagerCtx>::tr("No channel attached");
        stateMachine_->tryTransition(ConnectionStateMachine::State::Failed, "no-channel");
        return false;
    }

    io::Timeouts timeouts;
    timeouts.readMs = config_->timeoutMs;
    timeouts.writeMs = config_->timeoutMs;
    channel_->setTimeouts(timeouts);

    if (channel_->isOpen()) {
        stateMachine_->tryTransition(ConnectionStateMachine::State::Connected, "already-open");
        return true;
    }

    const int attempts = allowReconnect ? std::max(1, config_->retries + 1) : 1;
    QString connectError;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        clearError();

        stateMachine_->tryTransition(
            attempt == 0 ? ConnectionStateMachine::State::Connecting
                         : ConnectionStateMachine::State::Reconnecting,
            attempt == 0 ? "connect-attempt" : "reconnect-attempt");

        if (!channel_->open()) {
            connectError = TrContext<kConnManagerCtx>::tr("Failed to dispatch channel open");
        } else {
            const auto deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(config_->timeoutMs);
            if (waitForChannelState(io::ChannelState::Open, deadline, &connectError)) {
                stateMachine_->tryTransition(ConnectionStateMachine::State::Connected,
                                             "connect-success");
                return true;
            }
        }

        channel_->close();
        stateMachine_->tryTransition(ConnectionStateMachine::State::Failed, "connect-failed");
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
            stateMachine_->tryTransition(ConnectionStateMachine::State::Failed,
                                         "reconnect-aborted");
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
