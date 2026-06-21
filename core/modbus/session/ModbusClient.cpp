/**
 * @file ModbusClient.cpp
 * @brief Implementation of ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusClient.h"
#include "AppConstants.h"
#include "infra/logging/Logger.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <QtEndian>
#include <QCoreApplication>

namespace modbus::session {

ModbusClient::ConnectionState ModbusClient::connectionState() const {
    return connectionStateMachine_.currentState();
}

ModbusClient::RequestState ModbusClient::requestState() const {
    return requestStateMachine_.currentState();
}

void ModbusClient::clearRuntimeState(bool clearPendingQueue) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        frameExtractor_.reset();
        flowController_.reset();
    }
    connectionManager_.clearError();
    requestExecutor_.resetState(clearPendingQueue);
    if (transport_) {
        transport_->resetPendingState();
    }
}

ModbusClient::ModbusClient(std::shared_ptr<io::IChannel> channel, 
                           std::shared_ptr<transport::ITransport> transport)
    : channel_(std::move(channel)), transport_(std::move(transport))
    , frameExtractor_(base::ModbusMode::TCP, 9600)
    , retryStrategy_(RetryStrategy::Config{})
    , flowController_(base::ModbusMode::TCP)
    , timeoutController_(mutex_, cv_, aborted_)
    , connectionManager_(channel_.get(), &connectionStateMachine_,
                         &timeoutController_, &config_, mutex_, cv_)
    , requestExecutor_(channel_.get(), transport_.get(), &frameExtractor_,
                       &flowController_, &retryStrategy_, &requestValidator_,
                       &connectionStateMachine_, &requestStateMachine_,
                       &timeoutController_, &connectionManager_,
                       &config_, mutex_, cv_, aborted_,
                       pendingMutex_, pendingRequests_, nextRequestId_) {
    
    // Channel callbacks
    channel_->setReadHandler([this](QByteArrayView data) {
        requestExecutor_.onDataReceived(data);
    });
    
    channel_->setErrorHandler([this](const QString& error) {
        requestExecutor_.onChannelError(error);
    });

    channel_->setWriteDrainedHandler([this]() {
        std::lock_guard<std::mutex> lock(mutex_);
        flowController_.markWriteDrained(std::chrono::steady_clock::now());
        cv_.notify_one();
    });
    stateHandlerId_ = channel_->addStateHandler([this](io::ChannelState) {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    });
}

ModbusClient::~ModbusClient() {
    abort();
    if (channel_) {
        channel_->setReadHandler({});
        channel_->setErrorHandler({});
        channel_->setWriteDrainedHandler({});
        channel_->removeStateHandler(stateHandlerId_);
    }
    clearRuntimeState(true);
}

bool ModbusClient::connect() {
    if (!channel_) return false;

    aborted_ = false;
    const bool connected = ensureConnected(config_.autoReconnect);
    if (connected) {
        clearRuntimeState(false);
        requestStateMachine_.tryTransition(RequestState::Idle, "connect");
    }
    return connected;
}

void ModbusClient::disconnect() {
    aborted_ = true;
    connectionStateMachine_.tryTransition(ConnectionState::Disconnecting, "disconnect");
    if (channel_) {
        channel_->close();
    }
    clearRuntimeState(true);
    aborted_ = false;
    if (!connectionStateMachine_.tryTransition(ConnectionState::Disconnected, "disconnect")) {
        connectionStateMachine_.forceReset(ConnectionState::Disconnected);
    }
    requestStateMachine_.tryTransition(RequestState::Idle, "disconnect");
}

void ModbusClient::abort() {
    requestExecutor_.abort();
}

bool ModbusClient::isConnected() const {
    return connectionManager_.isConnected();
}

QString ModbusClient::lastChannelError() {
    return connectionManager_.lastChannelError();
}

void ModbusClient::setConfig(const base::ModbusConfig& config) {
    config_ = config;
    frameExtractor_.setConfig(config);
    flowController_.setMode(config.mode);
    retryStrategy_.reconfigure(RetryStrategy::Config{
        config.retries,
        config.retryIntervalMs,
        config.maxRetryIntervalMs,
        config.retryBackoffFactor,
        config.retryJitterPercent});
    if (isConnected()) {
        io::Timeouts timeouts;
        timeouts.readMs = config_.timeoutMs;
        timeouts.writeMs = config_.timeoutMs;
        channel_->setTimeouts(timeouts);
    }
}

bool ModbusClient::ensureConnected(bool allowReconnect) {
    return connectionManager_.ensureConnected(allowReconnect);
}

bool ModbusClient::waitForChannelState(io::ChannelState expectedState,
                                       std::chrono::steady_clock::time_point deadline,
                                       QString* errorOut) {
    return connectionManager_.waitForChannelState(expectedState, deadline, errorOut);
}

ModbusResponse ModbusClient::sendRequest(const base::Pdu& request, int slaveId) {
    return requestExecutor_.execute(request, slaveId);
}

void ModbusClient::sendRaw(const QByteArray& data) {
    requestExecutor_.sendRaw(data);
}

} // namespace modbus::session
