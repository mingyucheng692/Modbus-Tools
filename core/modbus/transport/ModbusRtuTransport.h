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
#include "PendingSlaveTracker.h"
#include <QByteArray>

namespace modbus::transport {

/**
 * @brief Modbus RTU transport layer (ADU build/parse).
 *
 * @thread buildRequest() and parseResponse() are safe to call from any thread.
 *         Internal pending-response state is protected by PendingSlaveTracker.
 */
class ModbusRtuTransport : public ITransport {
public:
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;

private:
    PendingSlaveTracker tracker_;
};

} // namespace modbus::transport
