/**
 * @file ModbusClient.cpp
 * @brief Implementation of ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusClient.h"
#include "Config.h"
#include "infra/logging/Logger.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <QtEndian>
#include <QCoreApplication>
#include <QThread>

namespace modbus::session {

#ifndef NDEBUG
void ModbusClient::assertSessionAffinity() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!sessionOwnerThread_) {
        // Guard not armed yet: pre-live factory configuration or direct
        // unit-test driving (ad-hoc threads exercising Busy/abort defense
        // paths). Nothing to enforce.
        return;
    }
    Q_ASSERT_X(sessionOwnerThread_ == QThread::currentThread(),
               "ModbusClient",
               "session-driving methods must run on the owning worker thread; "
               "route the call through ModbusWorker (see @thread in ModbusClient.h)");
}

void ModbusClient::claimSessionOwnershipForCurrentThread() {
    std::lock_guard<std::mutex> lock(mutex_);
    sessionOwnerThread_ = QThread::currentThread();
}
#endif

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
    , connectionManager_(channel_.get(), &connectionStateMachine_,
                         aborted_, &retryStrategy_, &config_, mutex_, cv_)
    , requestExecutor_(RequestExecutor::Dependencies{
          channel_.get(),
          transport_.get(),
          &frameExtractor_,
          &flowController_,
          &retryStrategy_,
          &connectionStateMachine_,
          &requestStateMachine_,
          &connectionManager_,
          &config_,
          mutex_,
          cv_,
          aborted_,
          pendingMutex_,
          pendingRequests_,
          nextRequestId_}) {

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

    // Destruction-order safety: if the worker thread is still executing a
    // request (RequestStateMachine in Sending or Waiting), destroying
    // requestExecutor_ and its collaborators below is a use-after-free.
    // The caller (ModbusWorker / WorkerReleaseCoordinator) must ensure the
    // worker thread has quit()+wait() before releasing the client.
    // Release-mode assert: UAF is unrecoverable, must crash.
    const auto reqState = requestStateMachine_.currentState();
    if (reqState == RequestStateMachine::State::Sending ||
        reqState == RequestStateMachine::State::Waiting) {
        // Double-write to stderr: this abort path bypasses QtMessageHandler,
        // and the async queue may not flush the critical line in time.
        std::fprintf(stderr,
                     "ModbusClient::~ModbusClient: destroyed while request is "
                     "in-flight (state=%s). The worker thread must be joined "
                     "before destruction. This is a use-after-free.\n",
                     RequestStateMachine::toString(reqState));
        SPDLOG_CRITICAL("ModbusClient::~ModbusClient: destroyed while request is "
                         "in-flight (state={}). The worker thread must be joined "
                         "before destruction. This is a use-after-free.",
                         RequestStateMachine::toString(reqState));
        spdlog::default_logger()->flush();
        std::abort();
    }

    if (channel_) {
        channel_->setReadHandler({});
        channel_->setErrorHandler({});
        channel_->setWriteDrainedHandler({});
        channel_->removeStateHandler(stateHandlerId_);
    }
    clearRuntimeState(true);
}

bool ModbusClient::connect() {
    assertSessionAffinity();
    if (!channel_) return false;

    aborted_ = false;
    sessionHealth_.store(SessionHealth::Unknown, std::memory_order_release);
    const bool connected = ensureConnected(config_.autoReconnect);
    if (connected) {
        clearRuntimeState(false);
        requestStateMachine_.tryTransition(RequestState::Idle, "connect");
    }
    return connected;
}

void ModbusClient::disconnect() {
    assertSessionAffinity();
    // Lifecycle closure: pairs with the connect-side info logs so every
    // session has a visible start AND end in the log file.
    SPDLOG_INFO("ModbusClient: disconnect requested, reason=user-request");
    aborted_ = true;
    sessionHealth_.store(SessionHealth::Unknown, std::memory_order_release);
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
    SPDLOG_INFO("ModbusClient: session disconnected");
}

void ModbusClient::abort() {
    requestExecutor_.abort();
}

bool ModbusClient::isConnected() const {
    return connectionManager_.isConnected();
}

QString ModbusClient::lastChannelError() const {
    return connectionManager_.lastChannelError();
}

void ModbusClient::setConfig(const base::ModbusConfig& config) {
    assertSessionAffinity();
    // Detect endpoint change before applying config so we can tear down
    // the stale connection before the new endpoint takes effect.
    const bool endpointChanged = (config_.ipAddress != config.ipAddress ||
                                   config_.port != config.port);
    const auto oldIp = config_.ipAddress;
    const auto oldPort = config_.port;

    config_ = config;
    frameExtractor_.setConfig(config);
    flowController_.setMode(config.mode);
    retryStrategy_.reconfigure(RetryStrategy::Config{
        config.retries,
        config.retryIntervalMs,
        config.maxRetryIntervalMs,
        config.retryBackoffFactor,
        config.retryJitterPercent});

    if (endpointChanged && isConnected()) {
        SPDLOG_INFO("ModbusClient::setConfig: endpoint changed while connected "
                     "(old={}:{} new={}:{}), disconnecting",
                     oldIp.toStdString(), oldPort,
                     config.ipAddress.toStdString(), config.port);
        disconnect();
    } else if (isConnected()) {
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
    assertSessionAffinity();
    auto response = requestExecutor_.execute(request, slaveId);
    // Update session health based on the outcome of this request.
    // Busy (lock contention) is not a transport failure — don't downgrade.
    if (response.kind == ModbusResponseKind::Success) {
        sessionHealth_.store(SessionHealth::Healthy, std::memory_order_release);
    } else if (!response.isBusy()) {
        sessionHealth_.store(SessionHealth::Unresponsive, std::memory_order_release);
    }
    return response;
}

void ModbusClient::sendRaw(const QByteArray& data) {
    assertSessionAffinity();
    requestExecutor_.sendRaw(data);
}

} // namespace modbus::session
