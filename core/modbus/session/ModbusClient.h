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
#include "SessionTypes.h"
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
 * @brief Core Modbus client with full session management.
 *
 * @thread Objects of this class can be called from any thread. Internal state
 *         is protected by mutex_ (connection/response state) and pendingMutex_
 *         (request queue). The underlying channel_/transport_ operations are
 *         serialized through queued slot invocations or synchronous calls.
 *
 * @note sendRequest() uses a separate requestMutex_ to prevent concurrent
 *       request execution, enforcing strict serialization of the submit path.
 *
 * @par Architecture decision: ModbusClient / RequestExecutor split
 *      ModbusClient has 7 public methods that forward to RequestExecutor
 *      (sendRequest, sendRaw, abort) and ConnectionManager (connect,
 *      disconnect, isConnected, lastChannelError). The split is intentional:
 *      RequestExecutor (~830 LOC) handles request lifecycle, retry, response
 *      parsing; ConnectionManager handles connection lifecycle. Merging them
 *      into a single ~1000 LOC class would hurt readability without reducing
 *      complexity. The shared synchronization primitives (mutex_, cv_,
 *      aborted_) are passed by reference into RequestExecutor::Dependencies
 *      rather than extracted into a SessionSynchronizationContext wrapper —
 *      3 primitives do not justify a wrapper class.
 *
 * @par Destruction order
 *      The caller (typically ModbusWorker / WorkerReleaseCoordinator) MUST
 *      ensure the worker thread has fully joined (quit()+wait()) before
 *      ~ModbusClient() runs. If RequestExecutor::execute() is still running
 *      on the worker thread when the destructor fires, destroying
 *      requestExecutor_ and its collaborators is a use-after-free. The
 *      destructor asserts (release-mode, std::abort) that no request is
 *      in-flight (RequestStateMachine is Idle/Completed/Failed/Aborted).
 */
class ModbusClient {
public:
    using ConnectionState = ConnectionStateMachine::State;
    using RequestState = RequestStateMachine::State;

    ModbusClient(std::shared_ptr<io::IChannel> channel,
                 std::shared_ptr<transport::ITransport> transport);
    ~ModbusClient() noexcept;

    ModbusResponse sendRequest(const base::Pdu& request, int slaveId = -1);
    void sendRaw(const QByteArray& data);
    bool connect();
    void disconnect();
    bool isConnected() const;
    QString lastChannelError() const;
    void abort();
    void setConfig(const base::ModbusConfig& config);
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
