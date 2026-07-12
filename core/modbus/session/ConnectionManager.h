/**
 * @file ConnectionManager.h
 * @brief Manages Modbus channel connection lifecycle and state.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ConnectionStateMachine.h"
#include "TimeoutController.h"
#include "infra/io/IChannel.h"
#include "../base/ModbusConfig.h"
#include <QString>
#include <mutex>
#include <condition_variable>

namespace modbus::session {

/**
 * @brief Encapsulates connection lifecycle: connect, disconnect, reconnect, and
 *        channel state waiting.
 *
 * @note This class must not outlive the referenced objects (channel, stateMachine,
 *       timeoutController, config, mutex, cv). The owning ModbusClient is
 *       responsible for lifetime ordering.
 */
class ConnectionManager {
public:
    ConnectionManager(io::IChannel* channel,
                      ConnectionStateMachine* stateMachine,
                      TimeoutController* timeoutController,
                      const base::ModbusConfig* config,
                      std::mutex& mutex,
                      std::condition_variable& cv);

    /**
     * @brief Attempt to establish a connection with retry support.
     * @param allowReconnect Whether to retry on failure
     * @return true if connected successfully
     */
    bool ensureConnected(bool allowReconnect);

    /**
     * @brief Check if the underlying channel is open.
     */
    bool isConnected() const;

    /**
     * @brief Get the last channel error message.
     * @note Thread-safe via internal mutex.
     */
    QString lastChannelError() const;
    QString lastChannelErrorLocked() const;
    bool hasChannelErrorLocked() const;

    /**
     * @brief Clear the last channel error.
     */
    void clearError();
    void clearErrorLocked();

    /**
     * @brief Set the last channel error (called from error callback).
     */
    void setError(const QString& error);
    void setErrorLocked(const QString& error);

    /**
     * @brief Wait for the channel to reach a specific state.
     * @param expectedState The desired channel state
     * @param deadline Absolute deadline for the wait
     * @param errorOut If non-null, receives the error message on failure
     * @return true if the channel reached the expected state
     */
    bool waitForChannelState(io::ChannelState expectedState,
                             std::chrono::steady_clock::time_point deadline,
                             QString* errorOut);

private:
    io::IChannel* channel_;
    ConnectionStateMachine* stateMachine_;
    TimeoutController* timeoutController_;
    const base::ModbusConfig* config_;
    std::mutex& mutex_;
    std::condition_variable& cv_;
    QString lastChannelError_;
};

} // namespace modbus::session
