// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "SessionTypes.h"
#include "FrameExtractor.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "ConnectionManager.h"
#include "../transport/ITransport.h"
#include "../base/ModbusConfig.h"
#include "common/LogDedupe.h"
#include "infra/io/IChannel.h"
#include <atomic>
#include <deque>
#include <chrono>
#include <optional>
#include <tuple>
#include <functional>
#include <memory>
#include <QTimer>

namespace modbus::session {

// Role: Executes Modbus requests with retry, timeout, async reactor, and response parsing.
// Thread-safety: Thread-compatible. Driven by owning ModbusClient on worker thread.
// Ownership: Owned by ModbusClient; holds non-owning references to protocol stack collaborators.
class RequestExecutor {
public:
    using AsyncResponseCallback = std::function<void(const ModbusResponse&)>;

    struct PendingRequest {
        int requestId = 0;
        int slaveId = 0;
        int timeoutMs = 0;
        int retries = 0;
        base::FunctionCode functionCode = base::FunctionCode::ReadHoldingRegisters;
        std::chrono::steady_clock::time_point enqueueAt{};
    };

    /// Aggregated dependencies for RequestExecutor.
    ///
    /// Group 1 (protocol stack): channel through config — non-owning
    /// references to the protocol stack collaborators owned by ModbusClient.
    /// Group 2 (sync primitives): aborted — owned by ModbusClient.
    struct Dependencies {
        // 1. Protocol stack references (non-owning)
        io::IChannel* channel = nullptr;
        transport::ITransport* transport = nullptr;
        FrameExtractor* frameExtractor = nullptr;
        RetryStrategy* retryStrategy = nullptr;
        ConnectionStateMachine* connStateMachine = nullptr;
        RequestStateMachine* reqStateMachine = nullptr;
        ConnectionManager* connectionManager = nullptr;
        const base::ModbusConfig* config = nullptr;

        // 2. Synchronization primitives (owned by ModbusClient)
        std::atomic<bool>& aborted;
    };

    explicit RequestExecutor(const Dependencies& deps);
    ~RequestExecutor();

    [[nodiscard]] common::Result<base::Pdu, base::ModbusError> executeRequest(const base::Pdu& request, int slaveId);
    [[nodiscard]] ModbusResponse execute(const base::Pdu& request, int slaveId);
    void executeAsync(const base::Pdu& request, int slaveId, quint64 traceId, AsyncResponseCallback callback);

    void sendRaw(const QByteArray& data);
    void abort();
    void onDataReceived(QByteArrayView data);
    void onChannelError(const QString& error);
    void onWriteDrained();
    void resetState(bool clearPendingQueue);

private:
    struct RequestLockGuard {
        bool& flag;
        explicit RequestLockGuard(bool& f) : flag(f) {}
        ~RequestLockGuard() { flag = false; }
    };

    [[nodiscard]] bool tryAcquireRequestLock();

    void sendCurrentRequest();
    void performWrite(const QByteArray& adu);
    void checkPendingResponse();
    void handleTimeout();
    void completeTransaction(ModbusResponse response);

    std::optional<ModbusResponse> handleParsedFrame(
        const QByteArray& frame,
        const base::Pdu& request,
        int slaveId,
        std::chrono::steady_clock::time_point start);
    ModbusResponse handleExceptionResponse(const base::Pdu& responsePdu, int slaveId,
                                           const base::Pdu& requestPdu);
    bool isBroadcastRequest(int slaveId, base::FunctionCode functionCode) const;
    bool shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const;

    [[nodiscard]] int determineMaxRetries(base::FunctionCode fc) const;
    int enqueuePendingRequest(const base::Pdu& request, int slaveId, int maxRetries);
    void finishPendingRequest(int requestId, bool success, const QString& error);

    // --- Non-owning references to ModbusClient-owned resources ---
    // 1. Protocol stack
    io::IChannel* channel_;
    transport::ITransport* transport_;
    FrameExtractor* frameExtractor_;
    RetryStrategy* retryStrategy_;
    ConnectionStateMachine* connStateMachine_;
    RequestStateMachine* reqStateMachine_;
    ConnectionManager* connectionManager_;
    const base::ModbusConfig* config_;
    // 2. Synchronization primitives
    std::atomic<bool>& aborted_;

    // --- Inlined Flow Control (replaces FlowController) ---
    bool writeDrained_{true};
    std::chrono::steady_clock::time_point lastWriteDrainedAt_{};
    std::chrono::steady_clock::time_point rtuSendWindowOpensAt_{};

    // --- Active transaction state ---
    base::Pdu activeRequest_;
    int activeSlaveId_ = 0;
    quint64 activeTraceId_ = 0;
    int activeRequestId_ = 0;
    std::chrono::steady_clock::time_point transactionStartTime_{};
    std::unique_ptr<QTimer> transactionTimer_;
    std::unique_ptr<QTimer> rtuSilenceTimer_;
    AsyncResponseCallback activeCallback_;

    // --- Owned members ---
    std::deque<PendingRequest> pendingRequests_;
    int nextRequestId_ = 1;

    bool requestLocked_{false};
    // Deduplication keys are integer tuples: (slave, fc, errorKind)
    using DedupeKey = std::tuple<uint8_t, uint8_t, uint8_t>;
    common::LogDedupe<DedupeKey> exceptionDedupe_;
    common::LogDedupe<DedupeKey> failureDedupe_;
    bool responseReady_ = false;
};

} // namespace modbus::session
