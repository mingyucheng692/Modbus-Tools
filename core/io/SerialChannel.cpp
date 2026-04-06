#include "SerialChannel.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

namespace io {

SerialChannel::SerialChannel() {
    QObject::connect(&serial_, &QSerialPort::bytesWritten, [this](qint64 bytes) {
        onBytesWritten(bytes);
    });
    QObject::connect(&serial_, &QSerialPort::readyRead, [this]() {
        onReadyRead();
    });
    QObject::connect(&serial_, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError error) {
        onErrorOccurred(error);
    });
}

SerialChannel::~SerialChannel() {
    close();
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

    if (serial_.isOpen()) {
        setState(ChannelState::Open);
        flushPendingWrites();
        return true;
    }

    closing_ = false;
    resetWriteState();
    setState(ChannelState::Opening);
    
    serial_.setPortName(config_.portName);
    serial_.setBaudRate(config_.baudRate);
    serial_.setDataBits(static_cast<QSerialPort::DataBits>(config_.dataBits));
    serial_.setStopBits(static_cast<QSerialPort::StopBits>(config_.stopBits));
    serial_.setParity(config_.parity);
    
    if (serial_.open(QIODevice::ReadWrite)) {
        setState(ChannelState::Open);
        return true;
    } else {
        setState(ChannelState::Error);
        emitError(serial_.errorString());
        return false;
    }
}

void SerialChannel::moveToThread(QThread* thread) {
    serial_.moveToThread(thread);
}

void SerialChannel::close() {
    if (QThread::currentThread() != serial_.thread()) {
        QThread* ownerThread = serial_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            closing_ = false;
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
    setState(ChannelState::Closing);
    closing_ = true;
    if (serial_.isOpen()) {
        serial_.close();
    }
    closing_ = false;
    setState(ChannelState::Closed);
}

bool SerialChannel::write(QByteArrayView data) {
    if (QThread::currentThread() != serial_.thread()) {
        QThread* ownerThread = serial_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        QByteArray dataCopy(data.data(), data.size());
        return QMetaObject::invokeMethod(&serial_, [this, dataCopy]() {
            write(dataCopy);
        }, Qt::QueuedConnection);
    }

    if (!serial_.isOpen()) {
        return false;
    }

    QByteArray dataBuffer(data.data(), data.size());
    if (dataBuffer.isEmpty()) {
        return true;
    }

    pendingWrites_.push_back(dataBuffer);
    emitMonitor(true, dataBuffer);
    flushPendingWrites();
    return true;
}

void SerialChannel::setConfig(const SerialConfig& config) {
    config_ = config;
}

void SerialChannel::setDtr(bool set) {
    if (QThread::currentThread() != serial_.thread()) {
        QMetaObject::invokeMethod(&serial_, [this, set]() {
            this->setDtr(set);
        }, Qt::QueuedConnection);
        return;
    }
    
    if (isOpen()) {
        serial_.setDataTerminalReady(set);
    }
}

void SerialChannel::setRts(bool set) {
    if (QThread::currentThread() != serial_.thread()) {
        QMetaObject::invokeMethod(&serial_, [this, set]() {
            this->setRts(set);
        }, Qt::QueuedConnection);
        return;
    }
    
    if (isOpen()) {
        serial_.setRequestToSend(set);
    }
}

void SerialChannel::flushPendingWrites() {
    if (QThread::currentThread() != serial_.thread()) {
        QMetaObject::invokeMethod(&serial_, [this]() {
            flushPendingWrites();
        }, Qt::QueuedConnection);
        return;
    }

    if (!serial_.isOpen() || pendingWrites_.empty()) {
        return;
    }

    auto& frame = pendingWrites_.front();
    while (currentWriteOffset_ < frame.size()) {
        const char* dataPtr = frame.constData() + currentWriteOffset_;
        const qint64 remaining = static_cast<qint64>(frame.size() - currentWriteOffset_);
        const qint64 accepted = serial_.write(dataPtr, remaining);
        if (accepted < 0) {
            resetWriteState();
            setState(ChannelState::Error);
            emitError(serial_.errorString().isEmpty() ? QStringLiteral("Serial write failed") : serial_.errorString());
            return;
        }
        if (accepted == 0) {
            break;
        }
        currentWriteOffset_ += static_cast<qsizetype>(accepted);
    }
}

void SerialChannel::onReadyRead() {
    QByteArray data = serial_.readAll();
    if (!data.isEmpty()) {
        spdlog::info("SerialChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
        emitRead(data);
    }
}

void SerialChannel::onBytesWritten(qint64 bytes) {
    Q_UNUSED(bytes);
    while (!pendingWrites_.empty() &&
           currentWriteOffset_ >= pendingWrites_.front().size() &&
           serial_.bytesToWrite() == 0) {
        addTx(pendingWrites_.front().size());
        pendingWrites_.pop_front();
        currentWriteOffset_ = 0;
    }
    flushPendingWrites();
}

void SerialChannel::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        if (closing_) {
            return;
        }
        resetWriteState();
        setState(ChannelState::Error);
        emitError(serial_.errorString().isEmpty() ? QStringLiteral("Serial port error") : serial_.errorString());
    }
}

void SerialChannel::resetWriteState() {
    pendingWrites_.clear();
    currentWriteOffset_ = 0;
}

}
