#include "SerialChannel.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

namespace io {

SerialChannel::SerialChannel() {
    writeTimeoutTimer_.setSingleShot(true);
    QObject::connect(&writeTimeoutTimer_, &QTimer::timeout, [this]() {
        onWriteTimeout();
    });
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
    // 移除 setReadBufferSize(1)，恢复系统级块读取，极大降低高波特率下的 CPU 开销。
    // 在非 RTOS 系统上，块内字符间隙已由驱动保证，无需应用层逐字节判断 t1.5。
    
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
    writeTimeoutTimer_.moveToThread(thread);
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
    disarmWriteTimeout();
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
    armWriteTimeout();
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
        if (pendingWrites_.empty() && serial_.bytesToWrite() == 0) {
            disarmWriteTimeout();
        }
        return;
    }

    auto& frame = pendingWrites_.front();
    while (currentWriteOffset_ < frame.size()) {
        const char* dataPtr = frame.constData() + currentWriteOffset_;
        const qint64 remaining = static_cast<qint64>(frame.size() - currentWriteOffset_);
        const qint64 accepted = serial_.write(dataPtr, remaining);
        if (accepted < 0) {
            resetWriteState();
            disarmWriteTimeout();
            setState(ChannelState::Error);
            emitError(serial_.errorString().isEmpty() ? QStringLiteral("Serial write failed") : serial_.errorString());
            return;
        }
        if (accepted == 0) {
            break;
        }
        currentWriteOffset_ += static_cast<qsizetype>(accepted);
        armWriteTimeout();
    }
}

void SerialChannel::onReadyRead() {
    if (closing_) return;

    QByteArray data = serial_.readAll();
    if (!data.isEmpty()) {
        // OS 驱动层推送上来的一批数据视为连续到达，分配统一时间戳
        // 这在 PC 平台上是唯一合理且高性能的做法
        spdlog::info("SerialChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
        emitRead(data);
    }
}

void SerialChannel::onBytesWritten(qint64 bytes) {
    Q_UNUSED(bytes);
    bool queueDrained = false;
    while (!pendingWrites_.empty() &&
           currentWriteOffset_ >= pendingWrites_.front().size() &&
           serial_.bytesToWrite() == 0) {
        addTx(pendingWrites_.front().size());
        pendingWrites_.pop_front();
        currentWriteOffset_ = 0;
        queueDrained = pendingWrites_.empty();
    }
    if (queueDrained && serial_.bytesToWrite() == 0) {
        disarmWriteTimeout();
        emitWriteDrained();
    } else if (!pendingWrites_.empty() || serial_.bytesToWrite() > 0) {
        armWriteTimeout();
    }
    flushPendingWrites();
}

void SerialChannel::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        if (closing_) {
            return;
        }
        const QString errorText = serial_.errorString().isEmpty()
            ? QStringLiteral("Serial port error")
            : serial_.errorString();
        spdlog::warn("SerialChannel: error code={} port={} message={}",
                     static_cast<int>(error),
                     config_.portName.toStdString(),
                     errorText.toStdString());
        resetWriteState();
        disarmWriteTimeout();
        setState(ChannelState::Error);
        emitError(errorText);
    }
}

void SerialChannel::onWriteTimeout() {
    if (QThread::currentThread() != serial_.thread()) {
        QMetaObject::invokeMethod(&serial_, [this]() {
            onWriteTimeout();
        }, Qt::QueuedConnection);
        return;
    }

    if (closing_ || !serial_.isOpen()) {
        disarmWriteTimeout();
        return;
    }

    if (pendingWrites_.empty() && serial_.bytesToWrite() == 0) {
        disarmWriteTimeout();
        return;
    }

    const bool draining = serial_.bytesToWrite() > 0;
    resetWriteState();
    disarmWriteTimeout();
    setState(ChannelState::Error);
    emitError(draining
        ? QStringLiteral("Serial write drain timeout")
        : QStringLiteral("Serial write timeout"));
}

void SerialChannel::armWriteTimeout() {
    const int timeoutMs = timeouts().writeMs;
    if (timeoutMs <= 0) {
        return;
    }
    writeTimeoutTimer_.start(timeoutMs);
}

void SerialChannel::disarmWriteTimeout() {
    if (writeTimeoutTimer_.isActive()) {
        writeTimeoutTimer_.stop();
    }
}

void SerialChannel::resetWriteState() {
    pendingWrites_.clear();
    currentWriteOffset_ = 0;
}

}
