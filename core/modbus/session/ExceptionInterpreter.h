/**
 * @file ExceptionInterpreter.h
 * @brief Header file for ExceptionInterpreter.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include "../base/ModbusTypes.h"

namespace modbus::session {

class ExceptionInterpreter {
public:
    static QString exceptionName(base::ExceptionCode code);
    static QString buildMessage(int slaveId,
        base::FunctionCode requestFc,
        base::ExceptionCode exceptionCode);
};

} // namespace modbus::session