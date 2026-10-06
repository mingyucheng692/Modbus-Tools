// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <cstdint>

namespace modbus::base {

enum class FunctionCode : uint8_t {
    ReadCoils = 0x01,
    ReadDiscreteInputs = 0x02,
    ReadHoldingRegisters = 0x03,
    ReadInputRegisters = 0x04,
    WriteSingleCoil = 0x05,
    WriteSingleRegister = 0x06,
    WriteMultipleCoils = 0x0F,
    WriteMultipleRegisters = 0x10,

    // Exception response function codes set the high-order error mask bit (0x80)
    ErrorMask = 0x80
};

enum class ExceptionCode : uint8_t {
    None = 0x00,
    IllegalFunction = 0x01,
    IllegalDataAddress = 0x02,
    IllegalDataValue = 0x03,
    ServerDeviceFailure = 0x04,
    Acknowledge = 0x05,
    ServerDeviceBusy = 0x06,
    NegativeAcknowledge = 0x07,
    MemoryParityError = 0x08,
    GatewayPathUnavailable = 0x0A,
    GatewayTargetDeviceFailed = 0x0B
};

/**
 * @brief Structured error codes for Modbus operations.
 * Decouples domain error states from user-facing localization.
 */
enum class ModbusError : uint8_t {
    None = 0,
    Timeout,
    ConnectionFailed,
    NotConnected,
    Busy,
    InvalidAdu,
    InvalidPdu,
    CrcMismatch,
    LrcMismatch,
    ExceptionResponse,
    WriteFailed,
    Aborted,
    TooManyInvalidBytes,
    Unknown
};

[[nodiscard]] constexpr const char* toString(ModbusError err) noexcept {
    switch (err) {
    case ModbusError::None: return "None";
    case ModbusError::Timeout: return "Timeout";
    case ModbusError::ConnectionFailed: return "ConnectionFailed";
    case ModbusError::NotConnected: return "NotConnected";
    case ModbusError::Busy: return "Busy";
    case ModbusError::InvalidAdu: return "InvalidAdu";
    case ModbusError::InvalidPdu: return "InvalidPdu";
    case ModbusError::CrcMismatch: return "CrcMismatch";
    case ModbusError::LrcMismatch: return "LrcMismatch";
    case ModbusError::ExceptionResponse: return "ExceptionResponse";
    case ModbusError::WriteFailed: return "WriteFailed";
    case ModbusError::Aborted: return "Aborted";
    case ModbusError::TooManyInvalidBytes: return "TooManyInvalidBytes";
    case ModbusError::Unknown: return "Unknown";
    }
    return "Unknown";
}

/**
 * @brief Register byte/word ordering for 16-bit registers.
 * Used primarily for diagnostic analysis of firmware byte order issues.
 */
enum class RegisterOrder {
    ABCD, ///< Standard Big-Endian: R1(High/Low), R2(High/Low)
    BADC, ///< Byte Swapped: R1(Low/High), R2(Low/High)
    CDAB, ///< Word Swapped: R2(High/Low), R1(High/Low)
    DCBA  ///< Total Swapped: R2(Low/High), R1(Low/High)
};

} // namespace modbus::base

namespace modbus {
using base::ModbusError;
} // namespace modbus
