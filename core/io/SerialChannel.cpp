#include "SerialChannel.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

namespace io {

SerialChannel::SerialChannel() {
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
        bool result = false;
        QMetaObject::invokeMethod(&serial_, [this, &result]() {
            result = this->open();
        }, Qt::BlockingQueuedConnection);
        return result;
    }

    if (isOpen()) return true;

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
        QMetaObject::invokeMethod(&serial_, [this]() {
            this->close();
        }, Qt::BlockingQueuedConnection);
        return;
    }

    if (serial_.isOpen()) {
        serial_.close();
    }
    setState(ChannelState::Closed);
}

bool SerialChannel::write(QByteArrayView data) {
    if (QThread::currentThread() != serial_.thread()) {
        bool result = false;
        QByteArray dataCopy(data.data(), data.size());
        QMetaObject::invokeMethod(&serial_, [this, dataCopy, &result]() {
            result = this->write(dataCopy);
        }, Qt::BlockingQueuedConnection);
        return result;
    }

    if (!isOpen()) return false;
    
    QByteArray dataBuffer(data.data(), data.size());
    qint64 written = serial_.write(dataBuffer);
    if (written == dataBuffer.size()) {
        addTx(written);
        emitMonitor(true, dataBuffer);
        // 关键：确保数据已发送出缓冲区，提高 RTT 测量和超时判断的准确性
        serial_.waitForBytesWritten(timeouts().writeMs);
        return true;
    }
    return false;
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

void SerialChannel::onReadyRead() {
    QByteArray data = serial_.readAll();
    if (!data.isEmpty()) {
        spdlog::info("SerialChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
        emitRead(data);
    }
}

void SerialChannel::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        emitError(serial_.errorString());
    }
}

}
