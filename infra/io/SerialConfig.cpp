/**
 * @file SerialConfig.cpp
 * @brief SerialConfig validation implementation.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SerialConfig.h"
#include <QCoreApplication>
#include <QSet>

namespace io {

static const QSet<qint32> kStandardBaudRates = {
    1200, 2400, 4800, 9600, 19200, 38400, 57600,
    115200, 230400, 460800, 921600
};

bool SerialConfig::isValid(QString* errorOut) const
{
    if (portName.trimmed().isEmpty()) {
        if (errorOut) {
            *errorOut = QCoreApplication::translate("SerialConfig",
                "Port name is empty");
        }
        return false;
    }

    if (!kStandardBaudRates.contains(baudRate)) {
        if (errorOut) {
            *errorOut = QCoreApplication::translate("SerialConfig",
                "Unsupported baud rate: %1").arg(baudRate);
        }
        return false;
    }

    if (dataBits < 5 || dataBits > 8) {
        if (errorOut) {
            *errorOut = QCoreApplication::translate("SerialConfig",
                "Invalid data bits: %1 (expected 5, 6, 7, or 8)").arg(dataBits);
        }
        return false;
    }

    if (stopBits != 1 && stopBits != 2) {
        if (errorOut) {
            // QSerialPort supports 1.5 stop bits as well
            if (stopBits == 3) {
                // 1.5 stop bits is represented as 3 in QSerialPort
                return true;
            }
            *errorOut = QCoreApplication::translate("SerialConfig",
                "Invalid stop bits: %1 (expected 1, 1.5, or 2)").arg(stopBits);
        }
        return false;
    }

    return true;
}

} // namespace io