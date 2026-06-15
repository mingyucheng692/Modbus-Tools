/**
 * @file ModbusRtuTransport.h
 * @brief Header file for ModbusRtuTransport.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ITransport.h"
#include <QByteArray>
#include <atomic>

namespace modbus::transport {

/**
 * @brief Modbus RTU transport layer (ADU build/parse).
 *
 * @thread buildRequest() and parseResponse() are safe to call from any thread.
 *         Internal state is protected by std::atomic members.
 */
class ModbusRtuTransport : public ITransport {
public:
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;

private:
    std::atomic<uint8_t> expectedResponseSlaveId_{0};
    std::atomic<bool> hasPendingRequest_{false};
};

} // namespace modbus::transport
