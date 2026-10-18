// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <cstdint>
#include "ModbusTypes.h"
#include "../../Config.h"

namespace modbus::base::function_traits {

/// Role: Compile-time traits and metadata queries for Modbus function codes per IEC 61158.
/// Thread-safety: Thread-safe (stateless pure constexpr queries).
/// Ownership: Header-only traits library; no instances required.

[[nodiscard]] constexpr bool isReadOperation(FunctionCode fc) noexcept {
    switch (fc) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs:
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] constexpr bool isReadOperation(uint8_t fc) noexcept {
    return isReadOperation(static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr bool isWriteOperation(FunctionCode fc) noexcept {
    switch (fc) {
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] constexpr bool isWriteOperation(uint8_t fc) noexcept {
    return isWriteOperation(static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr bool isSingleWrite(FunctionCode fc) noexcept {
    return fc == FunctionCode::WriteSingleCoil || fc == FunctionCode::WriteSingleRegister;
}

[[nodiscard]] constexpr bool isSingleWrite(uint8_t fc) noexcept {
    return isSingleWrite(static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr bool isMultipleWrite(FunctionCode fc) noexcept {
    return fc == FunctionCode::WriteMultipleCoils || fc == FunctionCode::WriteMultipleRegisters;
}

[[nodiscard]] constexpr bool isMultipleWrite(uint8_t fc) noexcept {
    return isMultipleWrite(static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr uint16_t maxAllowedQuantity(FunctionCode fc) noexcept {
    switch (fc) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs:
        return config::Modbus::kMaxReadBitsQuantity;
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters:
        return config::Modbus::kMaxReadQuantity;
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
        return 1;
    case FunctionCode::WriteMultipleCoils:
        return config::Modbus::kMaxWriteCoilsQuantity;
    case FunctionCode::WriteMultipleRegisters:
        return config::Modbus::kMaxWriteRegistersQuantity;
    default:
        return 0;
    }
}

[[nodiscard]] constexpr uint16_t maxAllowedQuantity(uint8_t fc) noexcept {
    return maxAllowedQuantity(static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr bool isBroadcastAllowed(uint8_t slaveId, FunctionCode fc) noexcept {
    if (slaveId != 0) {
        return true;
    }
    return isWriteOperation(fc);
}

[[nodiscard]] constexpr bool isBroadcastAllowed(uint8_t slaveId, uint8_t fc) noexcept {
    return isBroadcastAllowed(slaveId, static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr const char* functionCodeName(FunctionCode fc) noexcept {
    switch (fc) {
    case FunctionCode::ReadCoils:              return "Read Coils";
    case FunctionCode::ReadDiscreteInputs:     return "Read Discrete Inputs";
    case FunctionCode::ReadHoldingRegisters:   return "Read Holding Registers";
    case FunctionCode::ReadInputRegisters:     return "Read Input Registers";
    case FunctionCode::WriteSingleCoil:        return "Write Single Coil";
    case FunctionCode::WriteSingleRegister:    return "Write Single Register";
    case FunctionCode::WriteMultipleCoils:     return "Write Multiple Coils";
    case FunctionCode::WriteMultipleRegisters: return "Write Multiple Registers";
    default:                                   return "Unknown";
    }
}

[[nodiscard]] constexpr const char* functionCodeName(uint8_t fc) noexcept {
    return functionCodeName(static_cast<FunctionCode>(fc));
}

[[nodiscard]] constexpr const char* exceptionCodeName(ExceptionCode ec) noexcept {
    switch (ec) {
    case ExceptionCode::IllegalFunction:           return "Illegal Function";
    case ExceptionCode::IllegalDataAddress:        return "Illegal Data Address";
    case ExceptionCode::IllegalDataValue:          return "Illegal Data Value";
    case ExceptionCode::ServerDeviceFailure:       return "Server Device Failure";
    case ExceptionCode::Acknowledge:               return "Acknowledge";
    case ExceptionCode::ServerDeviceBusy:          return "Server Device Busy";
    case ExceptionCode::NegativeAcknowledge:       return "Negative Acknowledge";
    case ExceptionCode::MemoryParityError:         return "Memory Parity Error";
    case ExceptionCode::GatewayPathUnavailable:    return "Gateway Path Unavailable";
    case ExceptionCode::GatewayTargetDeviceFailed: return "Gateway Target Device Failed";
    default:                                       return "Unknown Exception";
    }
}

[[nodiscard]] constexpr const char* exceptionCodeName(uint8_t ec) noexcept {
    return exceptionCodeName(static_cast<ExceptionCode>(ec));
}

} // namespace modbus::base::function_traits

