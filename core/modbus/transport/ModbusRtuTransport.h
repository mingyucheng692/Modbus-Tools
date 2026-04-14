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

namespace modbus::transport {

class ModbusRtuTransport : public ITransport {
public:
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;

private:
    uint8_t expectedResponseSlaveId_ = 0;
    bool hasPendingRequest_ = false;
};

} // namespace modbus::transport
