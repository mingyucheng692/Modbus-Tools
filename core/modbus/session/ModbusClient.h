// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "Config.h"
#include "SessionTypes.h"
#include "FrameExtractor.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "ConnectionManager.h"
#include "RequestExecutor.h"
#include "../transport/ITransport.h"
#include "infra/io/IChannel.h"
#include <atomic>
#include <deque>
#include <chrono>

class QThread;

namespace modbus::session {

// Role: Coordinates connection management, frame extraction, and async request execution.
// Thread-safety: Thread-compatible. All driving methods must run on its owning worker thread.
// Ownership: Owns connection/request state machines, FrameExtractor, RequestExecutor; shared ownership of IChannel and ITransport.
//
// Concurrency model:
// Driven exclusively by ModbusWorker via Qt::QueuedConnection serialized events.
// Cross-thread reads (sessionHealth, connectionState, isConnected, abort) are safely decoupled.
//
// Destruction order:
// Worker thread must be fully joined (quit()+wait()) before ~ModbusClient() destroys state.
class ModbusClient {
public:
    using ConnectionState = ConnectionStateMachine::State;
    using RequestState = RequestStateMachine::State;

    ModbusClient(std::shared_ptr<io::IChannel> channel,
                 std::shared_ptr<transport::ITransport> transport);
    ~ModbusClient() noexcept;

    ModbusResponse sendRequest(const base::Pdu& request, int slaveId = -1);
    void sendRequestAsync(const base::Pdu& request, int slaveId, quint64 traceId,
                          std::function<void(const ModbusResponse&)> callback);
    [[nodiscard]] common::Result<base::Pdu, base::ModbusError> sendRequestResult(
        const base::Pdu& request, int slaveId = -1);
    void sendRequestAsyncResult(
        const base::Pdu& request, int slaveId, quint64 traceId,
        std::function<void(const common::Result<base::Pdu, base::ModbusError>&)> callback);
    void sendRaw(const QByteArray& data);
    bool connect();
    void disconnect();
    /// Transport-level fact (delegates to ConnectionManager ->
    /// IChannel::isOpen()). NOT the session-level truth: a half-open socket
    /// reports connected here while ConnectionState already says Failed, and
    /// a session declared dead by the unresponsive eviction reports false
    /// here. For session truth use connectionState() == Connected.
    bool isConnected() const;
    QString lastChannelError() const;
    void abort();
    void setConfig(const base::ModbusConfig& config);
    ConnectionState connectionState() const;
    RequestState requestState() const;

    /// Session health: whether the Modbus device is actually responding,
    /// orthogonal to the transport-layer connection state.
    SessionHealth sessionHealth() const {
        return sessionHealth_.load(std::memory_order_acquire);
    }

    /// Debug-only contract arming: ModbusWorker invokes this on its own
    /// thread (from every queued handler that drives the client) to declare
    /// itself the owning thread. From then on, Debug builds assert that all
    /// session-driving methods run on that thread (see thread contract above).
    /// No-op in Release builds.
#ifdef NDEBUG
    void claimSessionOwnershipForCurrentThread() {}
#else
    void claimSessionOwnershipForCurrentThread();
#endif

private:
    using PendingRequest = RequestExecutor::PendingRequest;

#ifdef NDEBUG
    // Release builds: the affinity contract is enforced by documentation and
    // ModbusWorker's QueuedConnection serialization; guards compile to nothing.
    void assertSessionAffinity() {}
#else
    // Debug-only owner-thread guard (see thread contract above). Armed
    // by claimSessionOwnershipForCurrentThread().
    void assertSessionAffinity();
    QThread* sessionOwnerThread_ = nullptr;
#endif

    void clearRuntimeState(bool clearPendingQueue);

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<transport::ITransport> transport_;
    base::ModbusConfig config_;

    // Channel state callback handler ID.
    io::IChannel::HandlerId stateHandlerId_ = 0;
    
    std::atomic<bool> aborted_ {false};
    ConnectionStateMachine connectionStateMachine_;
    RetryStrategy retryStrategy_;
    ConnectionManager connectionManager_;
    RequestStateMachine requestStateMachine_;
    std::atomic<SessionHealth> sessionHealth_{SessionHealth::Unknown};

    FrameExtractor frameExtractor_;
    RequestExecutor requestExecutor_;
};

} // namespace modbus::session
