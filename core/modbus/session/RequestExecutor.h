/**
 * @file RequestExecutor.h
 * @brief Handles Modbus request execution, response parsing, and retry logic.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "IModbusClient.h"
#include "RequestValidator.h"
#include "FrameExtractor.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "FlowController.h"
#include "TimeoutController.h"
#include "ConnectionManager.h"
#include "../transport/ITransport.h"
#include "../base/ModbusConfig.h"
#include "infra/io/IChannel.h"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <map>
#include <chrono>

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

    struct Dependencies {
        io::IChannel* channel;
        transport::ITransport* transport;
        FrameExtractor* frameExtractor;
        FlowController* flowController;
        RetryStrategy* retryStrategy;
        RequestValidator* requestValidator;
        ConnectionStateMachine* connStateMachine;
        RequestStateMachine* reqStateMachine;
        TimeoutController* timeoutController;
        ConnectionManager* connectionManager;
        const base::ModbusConfig* config;
        std::mutex& mutex;
        std::condition_variable& cv;
        std::atomic<bool>& aborted;
        std::mutex& pendingMutex;
        std::deque<PendingRequest>& pendingRequests;
        int& nextRequestId;
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
    ModbusResponse handleExceptionResponse(const base::Pdu& responsePdu, int slaveId,
                                           const base::Pdu& requestPdu);
    bool isRtuBroadcastRequest(int slaveId, base::FunctionCode functionCode) const;
    bool shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const;
    bool waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                           std::chrono::steady_clock::time_point* drainedAt);
    bool waitForEventOrTimeout(std::chrono::steady_clock::time_point deadline);
    int enqueuePendingRequest(const base::Pdu& request, int slaveId);
    void finishPendingRequest(int requestId, bool success, const QString& error);

    // --- Non-owning references to ModbusClient-owned resources ---
    io::IChannel* channel_;
    transport::ITransport* transport_;
    FrameExtractor* frameExtractor_;
    FlowController* flowController_;
    RetryStrategy* retryStrategy_;
    RequestValidator* requestValidator_;
    ConnectionStateMachine* connStateMachine_;
    RequestStateMachine* reqStateMachine_;
    TimeoutController* timeoutController_;
    ConnectionManager* connectionManager_;
    const base::ModbusConfig* config_;
    std::mutex& mutex_;
    std::condition_variable& cv_;
    std::atomic<bool>& aborted_;
    std::mutex& pendingMutex_;
    std::deque<PendingRequest>& pendingRequests_;
    int& nextRequestId_;

    // --- Owned member ---
    std::mutex requestMutex_;
    std::atomic<bool> requestLocked_{false};
    std::map<std::tuple<uint8_t, uint8_t, uint8_t>,
             std::chrono::steady_clock::time_point> dupeTracker_;
    bool responseReady_ = false;
};

} // namespace modbus::session
