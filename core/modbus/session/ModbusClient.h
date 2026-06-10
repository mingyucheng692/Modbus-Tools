/**
 * @file ModbusClient.h
 * @brief Header file for ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "AppConstants.h"
#include "IModbusClient.h"
#include "RequestValidator.h"
#include "FrameExtractor.h"
#include "BackoffCalculator.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "FlowController.h"
#include "../transport/ITransport.h"
#include "infra/io/IChannel.h"
#include <cassert>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <chrono>
#include <optional>

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
    struct PendingRequest {
        int requestId = 0;
        int slaveId = app::constants::Values::Modbus::kDefaultSlaveId;
        int timeoutMs = app::constants::Values::Modbus::kDefaultTimeoutMs;
        int retries = 0;
        base::FunctionCode functionCode = base::FunctionCode::ReadHoldingRegisters;
        std::chrono::steady_clock::time_point enqueueAt{};
    };

    ModbusResponse sendRequestInternal(const base::Pdu& request, int slaveId);
    void onDataReceived(QByteArrayView data);
    void onChannelError(const QString& error);
    bool ensureConnected(bool allowReconnect);
    bool waitForChannelState(io::ChannelState expectedState,
                             std::chrono::steady_clock::time_point deadline,
                             QString* errorOut);
    bool waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                           std::chrono::steady_clock::time_point* drainedAt);
    bool waitForEventOrTimeout(std::chrono::steady_clock::time_point deadline);
    bool isRtuBroadcastRequest(int slaveId, base::FunctionCode functionCode) const;
    bool shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const;
    bool waitForAbortableDelay(std::chrono::steady_clock::duration delay);
    int enqueuePendingRequest(const base::Pdu& request, int slaveId);
    void finishPendingRequest(int requestId, bool success, const QString& error);
    void clearRuntimeState(bool clearPendingQueue);

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<transport::ITransport> transport_;
    base::ModbusConfig config_;
    RequestValidator requestValidator_;

    // 同步机制：等待响应
    // @guarded_by mutex_ — responseReady_, lastChannelError_
    std::mutex mutex_;
    std::condition_variable cv_;
    bool responseReady_ = false;
    QString lastChannelError_;
    io::IChannel::HandlerId stateHandlerId_ = 0;
    
    // 保护 sendRequest/sendRaw 串行执行，防止请求路径发生重入
    // 断言在 Debug 下用于检测重入调用链，Release 下被编译移除
    std::mutex requestMutex_;
    std::atomic<bool> requestLocked_{false};
    std::atomic<bool> aborted_ {false};
    ConnectionStateMachine connectionStateMachine_;
    RequestStateMachine requestStateMachine_;

    // @guarded_by pendingMutex_ — pendingRequests_, nextRequestId_
    std::mutex pendingMutex_;
    FrameExtractor frameExtractor_;
    RetryStrategy retryStrategy_;
    FlowController flowController_;
    int nextRequestId_ = 1;
    std::deque<PendingRequest> pendingRequests_;
};

} // namespace modbus::session
