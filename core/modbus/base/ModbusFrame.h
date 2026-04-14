/**
 * @file ModbusFrame.h
 * @brief Header file for ModbusFrame.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArray>
#include <cstdint>
#include "ModbusTypes.h"

namespace modbus::base {

class Pdu {
public:
    Pdu() = default;
    
    // 构造普通请求/响应 PDU
    Pdu(FunctionCode functionCode, const QByteArray& data = {}) 
        : functionCode_(functionCode), data_(data) {}

    // 构造异常响应 PDU
    Pdu(FunctionCode functionCode, ExceptionCode exceptionCode)
        : functionCode_(static_cast<FunctionCode>(static_cast<uint8_t>(functionCode) | 0x80)) {
        data_.append(static_cast<char>(exceptionCode));
    }

    FunctionCode functionCode() const {
        return functionCode_;
    }

    QByteArray data() const {
        return data_;
    }

    bool isException() const {
        return (static_cast<uint8_t>(functionCode_) & static_cast<uint8_t>(FunctionCode::ErrorMask)) != 0;
    }

    ExceptionCode exceptionCode() const {
        if (!isException() || data_.isEmpty()) {
            return ExceptionCode::None;
        }
        return static_cast<ExceptionCode>(data_[0]);
    }

    // 获取原始的功能码（去除异常位）
    FunctionCode originalFunctionCode() const {
        return static_cast<FunctionCode>(static_cast<uint8_t>(functionCode_) & 0x7F);
    }

    // 序列化为字节流（不含 CRC/MBAP）
    QByteArray toByteArray() const {
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
