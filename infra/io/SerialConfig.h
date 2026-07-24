/**
 * @file SerialConfig.h
 * @brief Serial port configuration struct.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "core/Config.h"
#include "core/modbus/base/ModbusConfig.h"
#include <QSerialPort>
#include <QString>

namespace io {

struct SerialConfig {
    QString portName;
    qint32 baudRate = config::Serial::kDefaultBaudRate;
    int dataBits = config::Serial::kDefaultDataBits;
    int stopBits = config::Serial::kDefaultStopBits;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;

    [[nodiscard]] bool isValid(QString* errorOut = nullptr) const;
};

/// Convert ModbusConfig to SerialConfig, centralizing parity mapping.
/// ModbusConfig parity: 0=None, 2=Even, 3=Odd → QSerialPort::Parity.
/// flowControl is left at default (NoFlowControl).
[[nodiscard]] SerialConfig toSerialConfig(const modbus::base::ModbusConfig& config);

} // namespace io