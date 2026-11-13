// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SerialConfig.h"
#include <QCoreApplication>
#include <QSet>

namespace io {

namespace {

bool failValidation(QString* errorOut, const QString& message)
{
    if (errorOut) {
        *errorOut = message;
    }
    return false;
}

bool isValidParity(QSerialPort::Parity parity)
{
    switch (parity) {
    case QSerialPort::NoParity:
    case QSerialPort::EvenParity:
    case QSerialPort::OddParity:
    case QSerialPort::SpaceParity:
    case QSerialPort::MarkParity:
        return true;
    default:
        return false;
    }
}

bool isValidFlowControl(QSerialPort::FlowControl flowControl)
{
    switch (flowControl) {
    case QSerialPort::NoFlowControl:
    case QSerialPort::HardwareControl:
    case QSerialPort::SoftwareControl:
        return true;
    default:
        return false;
    }
}

} // namespace

static const QSet<qint32> kStandardBaudRates = {
    1200, 2400, 4800, 9600, 19200, 38400, 57600,
    115200, 230400, 460800, 921600
};

bool SerialConfig::isValid(QString* errorOut) const
{
    if (portName.trimmed().isEmpty()) {
        return failValidation(errorOut,
            QCoreApplication::translate("SerialConfig", "Port name is empty"));
    }
    if (!kStandardBaudRates.contains(baudRate)) {
        return failValidation(errorOut,
            QCoreApplication::translate("SerialConfig", "Unsupported baud rate: %1").arg(baudRate));
    }
    if (dataBits < 5 || dataBits > 8) {
        return failValidation(errorOut,
            QCoreApplication::translate("SerialConfig",
                "Invalid data bits: %1 (expected 5, 6, 7, or 8)").arg(dataBits));
    }
    // QSerialPort represents 1, 2, and 1.5 stop bits as 1, 2, and 3 respectively
    if (stopBits != 1 && stopBits != 2 && stopBits != 3) {
        return failValidation(errorOut,
            QCoreApplication::translate("SerialConfig",
                "Invalid stop bits: %1 (expected 1, 1.5, or 2)").arg(stopBits));
    }
    if (!isValidParity(parity)) {
        return failValidation(errorOut,
            QCoreApplication::translate("SerialConfig",
                "Unsupported parity setting: %1").arg(static_cast<int>(parity)));
    }
    if (!isValidFlowControl(flowControl)) {
        return failValidation(errorOut,
            QCoreApplication::translate("SerialConfig",
                "Unsupported flow control setting: %1").arg(static_cast<int>(flowControl)));
    }
    return true;
}

} // namespace io
