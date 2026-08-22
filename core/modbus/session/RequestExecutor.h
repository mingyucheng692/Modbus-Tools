/**
 * @file RequestExecutor.h
 * @brief Handles Modbus request execution, response parsing, and retry logic.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "SessionTypes.h"
#include "FrameExtractor.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "FlowController.h"
#include "TimeoutHelper.h"
#include "ConnectionManager.h"
#include "../transport/ITransport.h"
#include "../base/ModbusConfig.h"
#include "common/LogDedupe.h"
#include "infra/io/IChannel.h"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <chrono>
#include <optional>
#include <tuple>

namespace modbus::session {

/**
 * @brief Executes Modbus requests with retry, timeout, and response parsing.
 *
 * Owns the request serialization mutex (requestMutex_), the request-in-flight
 * flag (requestLocked_), and the duplicate exception tracker (dupeTracker_).
 * All other resources are non-owning references to the owning ModbusClient.
 *
 * @note This class must not outlive the referenced objects. The owning
 *       ModbusClient is responsible for lifetime ordering.
 */
class RequestExecutor {
public:
    struct PendingRequest {
        int requestId = 0;
        int slaveId = 0;
        int timeoutMs = 0;
        int retries = 0;
        base::FunctionCode functionCode = base::FunctionCode::ReadHoldingRegisters;
        std::chrono::steady_clock::time_point enqueueAt{};
    };

    /// @brief Aggregated dependencies for RequestExecutor.
    ///
    /// Group 1 (protocol stack): channel through config — non-owning
    /// references to the protocol stack collaborators owned by ModbusClient.
    /// Group 2 (sync primitives): mutex/cv/aborted — owned by ModbusClient.
    struct Dependencies {
        // 1. Protocol stack references (non-owning)
        io::IChannel* channel = nullptr;
        transport::ITransport* transport = nullptr;
        FrameExtractor* frameExtractor = nullptr;
        FlowController* flowController = nullptr;
        RetryStrategy* retryStrategy = nullptr;
        ConnectionStateMachine* connStateMachine = nullptr;
        RequestStateMachine* reqStateMachine = nullptr;
        ConnectionManager* connectionManager = nullptr;
        const base::ModbusConfig* config = nullptr;

        // 2. Synchronization primitives (owned by ModbusClient)
        std::mutex& mutex;
        std::condition_variable& cv;
        std::atomic<bool>& aborted;
    };

    explicit RequestExecutor(const Dependencies& deps);

    /**
     * @brief Execute a complete Modbus request with retry logic.
     * @param request The PDU to send
     * @param slaveId The target slave ID (-1 to use config default)
     * @return ModbusResponse with success/error status
     */
    ModbusResponse execute(const base::Pdu& request, int slaveId);

    /**
     * @brief Send raw data without waiting for a response.
     */
    void sendRaw(const QByteArray& data);

    /**
     * @brief Abort the current in-flight request.
     */
    void abort();

    /**
     * @brief Callback: data received from the channel.
     */
    void onDataReceived(QByteArrayView data);

    /**
     * @brief Callback: channel error occurred.
     */
    void onChannelError(const QString& error);

    /**
     * @brief Reset pre-request state (responseReady, dupeTracker, etc.).
     * @param clearPendingQueue If true, also clears the pending request queue.
     */
    void resetState(bool clearPendingQueue);

private:
    struct RequestLockGuard {
        std::atomic<bool>& flag;
        explicit RequestLockGuard(std::atomic<bool>& f) : flag(f) {}
        ~RequestLockGuard() { flag.store(false, std::memory_order_release); }
    };

    [[nodiscard]] bool tryAcquireRequestLock();

    ModbusResponse sendRequestInternal(const base::Pdu& request, int slaveId);
    // Handles an already-extracted frame: parseResponse -> validate -> return.
    // Returns std::nullopt to signal "continue waiting" (Unmatched frame or
    // originalFunctionCode mismatch); returns ModbusResponse for terminal outcomes.
    // Caller must release mutex_ before invoking (parsing may block).
    std::optional<ModbusResponse> handleParsedFrame(
        const QByteArray& frame,
        const base::Pdu& request,
        int slaveId,
        std::chrono::steady_clock::time_point start);
    ModbusResponse handleExceptionResponse(const base::Pdu& responsePdu, int slaveId,
                                           const base::Pdu& requestPdu);
    bool isBroadcastRequest(int slaveId, base::FunctionCode functionCode) const;
    bool shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const;
    bool waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                           std::chrono::steady_clock::time_point* drainedAt);
    bool waitForEventOrTimeout(std::chrono::steady_clock::time_point deadline);
    // Writes the ADU and drains the RTU write buffer. For TCP, only the write
    // is performed. Returns true on success; *drainedAt receives the time the
    // drain completed (unset for TCP / non-RTU).
    bool writeRtuFrameWithDrain(const QByteArray& adu,
                                std::chrono::steady_clock::time_point* drainedAt);
    int enqueuePendingRequest(const base::Pdu& request, int slaveId);
    void finishPendingRequest(int requestId, bool success, const QString& error);

    // --- Non-owning references to ModbusClient-owned resources ---
    // 1. Protocol stack
    io::IChannel* channel_;
    transport::ITransport* transport_;
    FrameExtractor* frameExtractor_;
    FlowController* flowController_;
    RetryStrategy* retryStrategy_;
    ConnectionStateMachine* connStateMachine_;
    RequestStateMachine* reqStateMachine_;
    ConnectionManager* connectionManager_;
    const base::ModbusConfig* config_;
    // 2. Synchronization primitives
    std::mutex& mutex_;
    std::condition_variable& cv_;
    std::atomic<bool>& aborted_;

    // --- Owned members ---
    std::mutex pendingMutex_;
    std::deque<PendingRequest> pendingRequests_;
    int nextRequestId_ = 1;

    std::mutex requestMutex_;
    std::atomic<bool> requestLocked_{false};
    // Deduplication keys are integer tuples by design: shouldLog() is called
    // while holding mutex_, so keys must never involve QString hashing/copy.
    // exceptionDedupe_: (slave, fc, exceptionCode) for Modbus exception responses.
    // failureDedupe_:   (slave, fc, errorKind) for timeout/retry log sites.
    using DedupeKey = std::tuple<uint8_t, uint8_t, uint8_t>;
    common::LogDedupe<DedupeKey> exceptionDedupe_;
    common::LogDedupe<DedupeKey> failureDedupe_;
    bool responseReady_ = false;
};

} // namespace modbus::session
