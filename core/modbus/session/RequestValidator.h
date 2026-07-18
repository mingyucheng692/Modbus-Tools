/**
 * @file RequestValidator.h
 * @brief Header file for RequestValidator.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include "../base/ModbusTypes.h"
#include "../base/ModbusConfig.h"

namespace modbus::base {
class Pdu;
}

namespace modbus::session::request_validator {

struct Result {
    bool valid = false;
    QString error;
};

Result validate(const base::Pdu& request, int slaveId, base::ModbusMode mode);

} // namespace modbus::session::request_validator
