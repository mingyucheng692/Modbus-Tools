// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

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
