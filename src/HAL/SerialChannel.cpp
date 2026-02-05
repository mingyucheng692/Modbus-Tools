#include "SerialChannel.h"
#include <spdlog/spdlog.h>

SerialChannel::SerialChannel(QObject* parent) : IChannel(parent) {
    serial_ = new QSerialPort(this);
    silenceTimer_ = new QTimer(this);
    silenceTimer_->setSingleShot(true);
    
    connect(serial_, &QSerialPort::readyRead, this, &SerialChannel::onSerialReadyRead);
    connect(serial_, &QSerialPort::errorOccurred, this, &SerialChannel::onSerialError);
    connect(silenceTimer_, &QTimer::timeout, this, &SerialChannel::onSilenceTimeout);
}

SerialChannel::~SerialChannel() {
    close();
}

void SerialChannel::setConnectionSettings(const QString& portName, int baudRate, int dataBits, int stopBits, int parity) {
    portName_ = portName;
    baudRate_ = baudRate;
    dataBits_ = dataBits;
    stopBits_ = stopBits;
    parity_ = parity;
}

void SerialChannel::open() {
    if (state_ == ChannelState::Open) return;
    
    serial_->setPortName(portName_);
    serial_->setBaudRate(baudRate_);
    
    // Map settings
    serial_->setDataBits(static_cast<QSerialPort::DataBits>(dataBits_));
    serial_->setStopBits(static_cast<QSerialPort::StopBits>(stopBits_));
    serial_->setParity(static_cast<QSerialPort::Parity>(parity_));
    serial_->setFlowControl(QSerialPort::NoFlowControl);
    
    if (serial_->open(QIODevice::ReadWrite)) {
        state_ = ChannelState::Open;
        emit opened();
        spdlog::info("Serial Port {} Opened", portName_.toStdString());
    } else {
        state_ = ChannelState::Error;
        emit errorOccurred(serial_->errorString());
        spdlog::error("Failed to open Serial Port {}: {}", portName_.toStdString(), serial_->errorString().toStdString());
    }
}

void SerialChannel::close() {
    if (state_ == ChannelState::Closed) return;
    serial_->close();
    state_ = ChannelState::Closed;
    emit closed();
}

void SerialChannel::write(const char* data, size_t size) {
    if (state_ != ChannelState::Open) return;
    serial_->write(data, size);
    
    std::vector<uint8_t> vec(data, data + size);
    emit dataSent(vec);
}

void SerialChannel::write(const std::vector<uint8_t>& data) {
    write(reinterpret_cast<const char*>(data.data()), data.size());
}

ChannelState SerialChannel::state() const { return state_; }
QString SerialChannel::errorString() const { return serial_->errorString(); }

void SerialChannel::onSerialReadyRead() {
    QByteArray data = serial_->readAll();
    if (data.isEmpty()) return;
    
    rxBuffer_.insert(rxBuffer_.end(), data.begin(), data.end());
    
    // Restart silence timer (heuristic: 10ms for now)
    silenceTimer_->start(10);
}

void SerialChannel::onSilenceTimeout() {
    if (!rxBuffer_.empty()) {
        emit dataReceived(rxBuffer_);
        rxBuffer_.clear();
    }
}

void SerialChannel::onSerialError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) return;
    // Don't change state to Error on simple read errors if port is still open? 
    // QSerialPort closes on ResourceError.
    if (!serial_->isOpen()) {
        state_ = ChannelState::Error;
        emit errorOccurred(serial_->errorString());
    }
}
