/**
 * @file SerialConfig.h
 * @brief Serial port configuration struct.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../AppConstants.h"
#include <QSerialPort>
#include <QString>

namespace io {

struct SerialConfig {
    QString portName;
    qint32 baudRate = app::constants::Values::Serial::kDefaultBaudRate;
    int dataBits = app::constants::Values::Serial::kDefaultDataBits;
    int stopBits = app::constants::Values::Serial::kDefaultStopBits;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};

} // namespace io