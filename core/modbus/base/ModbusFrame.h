// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <cstdint>
#include "ModbusTypes.h"

namespace modbus::base {

// Role: Protocol Data Unit (PDU) representing function code and data payload independent of transport.
// Thread-safety: Thread-compatible (value type).
// Ownership: Value semantics; internal buffer managed via Qt copy-on-write QByteArray.
class Pdu {
public:
    Pdu() = default;

    explicit Pdu(FunctionCode functionCode)
        : functionCode_(functionCode), data_{} {}

    Pdu(FunctionCode functionCode, const QByteArray& data) 
        : functionCode_(functionCode), data_(data) {}

    Pdu(FunctionCode functionCode, QByteArrayView data) 
        : functionCode_(functionCode), data_(data.toByteArray()) {}

    Pdu(FunctionCode functionCode, ExceptionCode exceptionCode)
        : functionCode_(static_cast<FunctionCode>(static_cast<uint8_t>(functionCode) | 0x80)) {
        data_.append(static_cast<char>(exceptionCode));
    }

    [[nodiscard]] FunctionCode functionCode() const noexcept {
        return functionCode_;
    }

    [[nodiscard]] const QByteArray& data() const noexcept {
        return data_;
    }

    [[nodiscard]] bool isException() const noexcept {
        return (static_cast<uint8_t>(functionCode_) & static_cast<uint8_t>(FunctionCode::ErrorMask)) != 0;
    }

    [[nodiscard]] ExceptionCode exceptionCode() const noexcept {
        if (!isException() || data_.isEmpty()) {
            return ExceptionCode::None;
        }
        return static_cast<ExceptionCode>(data_[0]);
    }

    [[nodiscard]] FunctionCode originalFunctionCode() const noexcept {
        return static_cast<FunctionCode>(static_cast<uint8_t>(functionCode_) & 0x7F);
    }

    // Serializes PDU into raw byte stream without transport encapsulation (MBAP/CRC).
    [[nodiscard]] QByteArray toByteArray() const {
        QByteArray buffer;
        buffer.append(static_cast<char>(functionCode_));
        buffer.append(data_);
        return buffer;
    }

private:
    FunctionCode functionCode_ = FunctionCode::ReadHoldingRegisters; // Default
    QByteArray data_;
};

} // namespace modbus::base
