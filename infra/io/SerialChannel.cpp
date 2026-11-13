// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SerialChannel.h"
#include "infra/logging/Logger.h"
#include "infra/logging/TraceContext.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

namespace io {

namespace {
unsigned long long threadToken(QThread* thread)
{
    return static_cast<unsigned long long>(reinterpret_cast<quintptr>(thread));
}
}

SerialChannel::SerialChannel() {
    QObject::connect(&serial_, &QSerialPort::bytesWritten, &serial_, [this](qint64 bytes) {
        onBytesWritten(bytes);
    }, Qt::QueuedConnection);
    QObject::connect(&serial_, &QSerialPort::readyRead, &serial_, [this]() {
        onReadyRead();
    }, Qt::QueuedConnection);
    QObject::connect(&serial_, &QSerialPort::errorOccurred, &serial_, [this](QSerialPort::SerialPortError error) {
        onErrorOccurred(error);
    }, Qt::QueuedConnection);
}

SerialChannel::~SerialChannel() {
    assertOwnerThreadForDestruction("SerialChannel");
    close();
}

QString SerialChannel::logContext() const {
    return QStringLiteral("port=%1").arg(config_.portName);
}

bool SerialChannel::open() {
    if (QThread::currentThread() != serial_.thread()) {
        QThread* ownerThread = serial_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        return QMetaObject::invokeMethod(&serial_, [this]() {
            open();
        }, Qt::QueuedConnection);
    }

    logThreadContextOnce("SerialChannel::open", openThreadLoggedFlag());

    if (serial_.isOpen()) {
        if (state() == ChannelState::Open) {
            flushPendingWrites();
            return true;
        }
        serial_.close();
        serial_.clearError();
    }

    setClosing(false);
    resetWriteState();

    QString errorMsg;
    if (!config_.isValid(&errorMsg)) {
        SPDLOG_WARN("SerialChannel: invalid config: {}", errorMsg.toStdString());
        setState(ChannelState::Error);
        emitError(ChannelErrorCode::ConnectionFailed, errorMsg);
        return false;
    }

    setState(ChannelState::Opening);

    const QString portName = config_.portName.trimmed();
    serial_.setPortName(portName);
    serial_.setBaudRate(config_.baudRate);
    serial_.setDataBits(static_cast<QSerialPort::DataBits>(config_.dataBits));
    serial_.setStopBits(static_cast<QSerialPort::StopBits>(config_.stopBits));
    serial_.setParity(config_.parity);
    serial_.setFlowControl(config_.flowControl);
    // Block-oriented reading delegates intra-character (t1.5) inter-frame timing
    // to kernel/UART drivers, minimizing context switches and CPU consumption at high baud rates.

    opening_ = true;
    const bool opened = serial_.open(QIODevice::ReadWrite);
    opening_ = false;
    if (opened) {
        serial_.clear(QSerialPort::Input);
        setState(ChannelState::Open);
        return true;
    } else {
        const auto serialErr = serial_.error();
        QString err = serial_.errorString();
        serial_.clearError();
        ChannelErrorCode errCode = ChannelErrorCode::ConnectionFailed;
        if (serialErr == QSerialPort::PermissionError) {
            errCode = ChannelErrorCode::PermissionDenied;
            err = QObject::tr("%1 is busy or in use, unable to open").arg(portName);
        } else if (serialErr == QSerialPort::DeviceNotFoundError) {
            errCode = ChannelErrorCode::PortNotFound;
        }
        SPDLOG_WARN("SerialChannel: open failed port={} baud={} error={}",
                     portName.toStdString(), config_.baudRate, err.toStdString());
        setState(ChannelState::Error);
        emitError(errCode, err);
        return false;
    }
}

void SerialChannel::moveToThread(QThread* thread) {
    SPDLOG_DEBUG("SerialChannel: moveToThread current={} target={}",
                              threadToken(serial_.thread()),
                              threadToken(thread));
    ChannelBase::moveToThread(thread);
    serial_.moveToThread(thread);
    moveWriteInfrastructureToThread(thread);
}

void SerialChannel::close() {
    if (QThread::currentThread() != serial_.thread()) {
        QThread* ownerThread = serial_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            setClosing(false);
            resetWriteState();
            setState(ChannelState::Closed);
            return;
        }
        QMetaObject::invokeMethod(&serial_, [this]() {
            this->close();
        }, Qt::QueuedConnection);
        return;
    }

    resetWriteState();
    disarmWriteTimeout();
    setState(ChannelState::Closing);
    setClosing(true);
    if (serial_.isOpen()) {
        serial_.close();
    }
    setClosing(false);
    setState(ChannelState::Closed);
}

void SerialChannel::setConfig(const SerialConfig& config) {
    config_ = config;
}

bool SerialChannel::setDtr(bool set) {
    if (QThread::currentThread() != serial_.thread()) {
        QThread* ownerThread = serial_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        return QMetaObject::invokeMethod(&serial_, [this, set]() {
            this->setDtr(set);
        }, Qt::QueuedConnection);
    }
    if (!serial_.isOpen()) {
        return false;
    }
    return serial_.setDataTerminalReady(set);
}

bool SerialChannel::setRts(bool set) {
    if (QThread::currentThread() != serial_.thread()) {
        QThread* ownerThread = serial_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        return QMetaObject::invokeMethod(&serial_, [this, set]() {
            this->setRts(set);
        }, Qt::QueuedConnection);
    }
    if (!serial_.isOpen()) {
        return false;
    }
    return serial_.setRequestToSend(set);
}

bool SerialChannel::setSerialControl(SerialSignal signal, bool value) {
    switch (signal) {
    case SerialSignal::Dtr: return setDtr(value);
    case SerialSignal::Rts: return setRts(value);
    }
    return false;
}

void SerialChannel::onReadyRead() {
    if (isClosing()) return;

    logThreadContextOnce("SerialChannel::onReadyRead", ioThreadLoggedFlag());

    QByteArray data = serial_.readAll();
    if (!data.isEmpty()) {
        // A batch delivered by the OS driver is treated as contiguous with a unified timestamp.
        SPDLOG_DEBUG("SerialChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
        emitRead(data);
    }
}

void SerialChannel::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        if (isClosing() || opening_ || state() == ChannelState::Error) {
            return;
        }
        QString errorText = serial_.errorString().isEmpty()
            ? QStringLiteral("Serial port error")
            : serial_.errorString();
        ChannelErrorCode errCode = ChannelErrorCode::ConnectionFailed;
        if (error == QSerialPort::PermissionError) {
            errCode = ChannelErrorCode::PermissionDenied;
            errorText = QObject::tr("%1 is busy or in use, unable to open").arg(config_.portName);
        } else if (error == QSerialPort::DeviceNotFoundError) {
            errCode = ChannelErrorCode::PortNotFound;
        }
        SPDLOG_WARN("SerialChannel: error trace_id={} code={} port={} message={}",
                     static_cast<unsigned long long>(modbus::trace::currentTraceId),
                     static_cast<int>(error),
                     config_.portName.toStdString(),
                     errorText.toStdString());
        resetWriteState();
        disarmWriteTimeout();
        setState(ChannelState::Error);
        onChannelError();
        emitError(errCode, errorText);
    }
}

void SerialChannel::onChannelError() {
    if (serial_.isOpen()) {
        serial_.close();
    }
    serial_.clearError();
}

}
