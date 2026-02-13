#include "SerialChannel.h"

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

void SerialChannel::close() {
    if (serial_.isOpen()) {
        serial_.close();
    }
    setState(ChannelState::Closed);
}

bool SerialChannel::write(QByteArrayView data) {
    if (!isOpen()) return false;
    
    qint64 written = serial_.write(data.data(), data.size());
    if (written == data.size()) {
        addTx(written);
        // serial_.waitForBytesWritten(timeouts().writeMs); // Optional: blocking write
        return true;
    }
    return false;
}

void SerialChannel::setConfig(const SerialConfig& config) {
    config_ = config;
}

void SerialChannel::onReadyRead() {
    QByteArray data = serial_.readAll();
    if (!data.isEmpty()) {
        addRx(data.size());
        emitRead(data);
    }
}

void SerialChannel::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        emitError(serial_.errorString());
    }
}

}
