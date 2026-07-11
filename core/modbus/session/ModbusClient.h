/**
 * @file ModbusClient.h
 * @brief Header file for ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "Config.h"
#include "IModbusClient.h"
#include "RequestValidator.h"
#include "FrameExtractor.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "FlowController.h"
#include "TimeoutController.h"
#include "ConnectionManager.h"
#include "RequestExecutor.h"
#include "../transport/ITransport.h"
#include "infra/io/IChannel.h"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <chrono>

namespace modbus::session {

/**
 * @brief Core Modbus client implementing IModbusClient with full session management.
 *
 * @thread Objects of this class can be called from any thread. Internal state
 *         is protected by mutex_ (connection/response state) and pendingMutex_
 *         (request queue). The underlying channel_/transport_ operations are
 *         serialized through queued slot invocations or synchronous calls.
 *
 * @note sendRequest() uses a separate requestMutex_ to prevent concurrent
 *       request execution, enforcing strict serialization of the submit path.
 */
class ModbusClient : public IModbusClient {
public:
    using ConnectionState = ConnectionStateMachine::State;
    using RequestState = RequestStateMachine::State;

    ModbusClient(std::shared_ptr<io::IChannel> channel, 
                 std::shared_ptr<transport::ITransport> transport);
    ~ModbusClient() noexcept override;

    ModbusResponse sendRequest(const base::Pdu& request, int slaveId = -1) override;
    void sendRaw(const QByteArray& data) override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString lastChannelError() override;
    void abort() override;
    void setConfig(const base::ModbusConfig& config) override;
    ConnectionState connectionState() const;
    RequestState requestState() const;

private:
    using PendingRequest = RequestExecutor::PendingRequest;

    bool ensureConnected(bool allowReconnect);
    bool waitForChannelState(io::ChannelState expectedState,
                             std::chrono::steady_clock::time_point deadline,
                             QString* errorOut);
    void clearRuntimeState(bool clearPendingQueue);

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<transport::ITransport> transport_;
    base::ModbusConfig config_;
    RequestValidator requestValidator_;

    // 同步机制：等待响应
    std::mutex mutex_;
    std::condition_variable cv_;
    io::IChannel::HandlerId stateHandlerId_ = 0;
    
    std::atomic<bool> aborted_ {false};
    TimeoutController timeoutController_;
    ConnectionStateMachine connectionStateMachine_;
    ConnectionManager connectionManager_;
    RequestStateMachine requestStateMachine_;

    // @guarded_by pendingMutex_ — pendingRequests_, nextRequestId_
    std::mutex pendingMutex_;
    FrameExtractor frameExtractor_;
    RetryStrategy retryStrategy_;
    FlowController flowController_;
    int nextRequestId_ = 1;
    std::deque<PendingRequest> pendingRequests_;

    RequestExecutor requestExecutor_;
};

} // namespace modbus::session
