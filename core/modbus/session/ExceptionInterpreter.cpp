/**
 * @file ExceptionInterpreter.cpp
 * @brief Implementation of ExceptionInterpreter.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ExceptionInterpreter.h"
#include <QCoreApplication>

namespace modbus::session {

namespace {

QString trClient(const char* text)
{
    return QCoreApplication::translate("ModbusClient", text);
}

} // namespace

QString ExceptionInterpreter::exceptionName(base::ExceptionCode code)
{
    using base::ExceptionCode;
    switch (code) {
    case ExceptionCode::IllegalFunction:
        return trClient("Illegal Function");
    case ExceptionCode::IllegalDataAddress:
        return trClient("Illegal Data Address");
    case ExceptionCode::IllegalDataValue:
        return trClient("Illegal Data Value");
    case ExceptionCode::ServerDeviceFailure:
        return trClient("Server Device Failure");
    case ExceptionCode::Acknowledge:
        return trClient("Acknowledge");
    case ExceptionCode::ServerDeviceBusy:
        return trClient("Server Device Busy");
    case ExceptionCode::NegativeAcknowledge:
        return trClient("Negative Acknowledge");
    case ExceptionCode::MemoryParityError:
        return trClient("Memory Parity Error");
    case ExceptionCode::GatewayPathUnavailable:
        return trClient("Gateway Path Unavailable");
    case ExceptionCode::GatewayTargetDeviceFailed:
        return trClient("Gateway Target Device Failed To Respond");
    default:
        return trClient("Unknown Exception");
    }
}

QString ExceptionInterpreter::buildMessage(int slaveId,
    base::FunctionCode requestFc,
    base::ExceptionCode exceptionCode)
{
    return trClient("Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)")
        .arg(slaveId)
        .arg(static_cast<int>(requestFc), 2, 16, QChar('0'))
        .arg(static_cast<int>(exceptionCode), 2, 16, QChar('0'))
        .arg(exceptionName(exceptionCode));
}

} // namespace modbus::session