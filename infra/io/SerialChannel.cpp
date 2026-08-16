/**
 * @file SerialChannel.cpp
 * @brief Implementation of SerialChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SerialChannel.h"
#include "infra/logging/Logger.h"
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
        setState(ChannelState::Open);
        flushPendingWrites();
        return true;
    }

    setClosing(false);
    resetWriteState();

    QString errorMsg;
    if (!config_.isValid(&errorMsg)) {
        SPDLOG_WARN("SerialChannel: invalid config: {}", errorMsg.toStdString());
        setState(ChannelState::Error);
        emitError(errorMsg);
        return false;
    }

    setState(ChannelState::Opening);

    serial_.setPortName(config_.portName);
    serial_.setBaudRate(config_.baudRate);
    serial_.setDataBits(static_cast<QSerialPort::DataBits>(config_.dataBits));
    serial_.setStopBits(static_cast<QSerialPort::StopBits>(config_.stopBits));
    serial_.setParity(config_.parity);
    serial_.setFlowControl(config_.flowControl);
    // 移除 setReadBufferSize(1)，恢复系统级块读取，极大降低高波特率下的 CPU 开销。
    // 在非 RTOS 系统上，块内字符间隙已由驱动保证，无需应用层逐字节判断 t1.5。

    if (serial_.open(QIODevice::ReadWrite)) {
        setState(ChannelState::Open);
        return true;
    } else {
        const QString err = serial_.errorString();
        SPDLOG_WARN("SerialChannel: open failed port={} baud={} error={}",
                     config_.portName.toStdString(), config_.baudRate, err.toStdString());
        setState(ChannelState::Error);
        emitError(err);
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

void SerialChannel::setDtr(bool set) {
    if (marshalToOwner(serial_, [this, set]() { this->setDtr(set); })) return;
    if (isOpen()) {
        serial_.setDataTerminalReady(set);
    }
}

void SerialChannel::setRts(bool set) {
    if (marshalToOwner(serial_, [this, set]() { this->setRts(set); })) return;
    if (isOpen()) {
        serial_.setRequestToSend(set);
    }
}

bool SerialChannel::setSerialControl(SerialSignal signal, bool value) {
    switch (signal) {
    case SerialSignal::Dtr: setDtr(value); return true;
    case SerialSignal::Rts: setRts(value); return true;
    }
    return false;
}

void SerialChannel::onReadyRead() {
    if (isClosing()) return;

    logThreadContextOnce("SerialChannel::onReadyRead", ioThreadLoggedFlag());

    QByteArray data = serial_.readAll();
    if (!data.isEmpty()) {
        // OS 驱动层推送上来的一批数据视为连续到达，分配统一时间戳
        // 这在 PC 平台上是唯一合理且高性能的做法
        SPDLOG_DEBUG("SerialChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
        emitRead(data);
    }
}

void SerialChannel::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        if (isClosing()) {
            return;
        }
        const QString errorText = serial_.errorString().isEmpty()
            ? QStringLiteral("Serial port error")
            : serial_.errorString();
        SPDLOG_WARN("SerialChannel: error code={} port={} message={}",
                     static_cast<int>(error),
                     config_.portName.toStdString(),
                     errorText.toStdString());
        resetWriteState();
        disarmWriteTimeout();
        setState(ChannelState::Error);
        emitError(errorText);
    }
}

}
