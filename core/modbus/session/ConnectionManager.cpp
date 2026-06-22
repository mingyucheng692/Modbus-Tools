/**
 * @file ConnectionManager.cpp
 * @brief Implementation of ConnectionManager.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ConnectionManager.h"
#include "BackoffCalculator.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <algorithm>

namespace modbus::session {
namespace {
    // i18n helper — delegates to the project-wide translation function
    QString trConn(const char* text) {
        return QObject::tr(text);
    }
} // namespace

ConnectionManager::ConnectionManager(io::IChannel* channel,
                                     ConnectionStateMachine* stateMachine,
                                     TimeoutController* timeoutController,
                                     const base::ModbusConfig* config,
                                     std::mutex& mutex,
                                     std::condition_variable& cv)
    : channel_(channel)
    , stateMachine_(stateMachine)
    , timeoutController_(timeoutController)
    , config_(config)
    , mutex_(mutex)
    , cv_(cv) {}

bool ConnectionManager::isConnected() const {
    return channel_ && channel_->isOpen();
}

QString ConnectionManager::lastChannelError() {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastChannelError_;
}

QString ConnectionManager::lastChannelErrorLocked() const {
    return lastChannelError_;
}

bool ConnectionManager::hasChannelErrorLocked() const {
    return !lastChannelError_.isEmpty();
}

void ConnectionManager::clearError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastChannelError_.clear();
}

void ConnectionManager::clearErrorLocked() {
    lastChannelError_.clear();
}

void ConnectionManager::setError(const QString& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastChannelError_ = error;
}

void ConnectionManager::setErrorLocked(const QString& error) {
    lastChannelError_ = error;
}

bool ConnectionManager::ensureConnected(bool allowReconnect) {
    if (!channel_) {
        std::lock_guard<std::mutex> lock(mutex_);
        lastChannelError_ = trConn("No channel attached");
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
        {
            std::lock_guard<std::mutex> lock(mutex_);
            clearErrorLocked();
        }

        stateMachine_->tryTransition(
            attempt == 0 ? ConnectionStateMachine::State::Connecting
                         : ConnectionStateMachine::State::Reconnecting,
            attempt == 0 ? "connect-attempt" : "reconnect-attempt");

        if (!channel_->open()) {
            connectError = trConn("Failed to dispatch channel open");
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

        BackoffCalculator::Config reconnectBackoffCfg;
        reconnectBackoffCfg.baseIntervalMs = config_->reconnectBaseMs;
        reconnectBackoffCfg.maxIntervalMs = config_->reconnectMaxMs;
        reconnectBackoffCfg.backoffFactor = config_->retryBackoffFactor;
        reconnectBackoffCfg.jitterPercent = config_->retryJitterPercent;
        BackoffCalculator reconnectBackoff(reconnectBackoffCfg);
        const int reconnectDelayMs = reconnectBackoff.calculateMs(attempt);
        if (!timeoutController_->waitForAbortableDelay(
                std::chrono::milliseconds(reconnectDelayMs))) {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_ = trConn("Aborted");
            stateMachine_->tryTransition(ConnectionStateMachine::State::Failed,
                                         "reconnect-aborted");
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    lastChannelError_ = connectError.isEmpty() ? trConn("Connect timeout") : connectError;
    spdlog::warn("ModbusClient: connect failed target={}:{} reason={} channelState={}",
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
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (hasChannelErrorLocked()) {
                if (errorOut) {
                    *errorOut = lastChannelErrorLocked();
                }
                clearErrorLocked();
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            if (errorOut && errorOut->isEmpty()) {
                *errorOut = trConn("Channel entered error state");
            }
            return false;
        }

        timeoutController_->waitForCondition([this, expectedState]() {
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
        *errorOut = trConn("Connect timeout");
    }
    return false;
}

} // namespace modbus::session
