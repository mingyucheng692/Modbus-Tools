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

} // namespace io