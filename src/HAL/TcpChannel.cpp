#include "TcpChannel.h"
#include <spdlog/spdlog.h>
#include <QTimer>
#include <QRandomGenerator>

TcpChannel::TcpChannel(QObject* parent) : IChannel(parent) {
    socket_ = new QTcpSocket(this);
    connect(socket_, &QTcpSocket::connected, this, &TcpChannel::onSocketConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &TcpChannel::onSocketDisconnected);
    connect(socket_, &QTcpSocket::errorOccurred, this, &TcpChannel::onSocketError);
    connect(socket_, &QTcpSocket::readyRead, this, &TcpChannel::onSocketReadyRead);
}

TcpChannel::~TcpChannel() {
    close();
}

void TcpChannel::setConnectionSettings(const QString& ip, uint16_t port) {
    ip_ = ip;
    port_ = port;
}

void TcpChannel::open() {
    if (state_ == ChannelState::Open || state_ == ChannelState::Opening) return;
    
    state_ = ChannelState::Opening;
    spdlog::info("TcpChannel connecting to {}:{}", ip_.toStdString(), port_);
    socket_->connectToHost(ip_, port_);
}

void TcpChannel::close() {
    if (state_ == ChannelState::Closed) return;
    
    state_ = ChannelState::Closing;
    socket_->disconnectFromHost();
}

void TcpChannel::write(const std::vector<uint8_t>& data) {
    if (state_ != ChannelState::Open) return;

    if (shouldDrop()) {
        // Simulating packet loss (TX)
        return; 
    }
    
    int delay = getDelay();
    if (delay > 0) {
        QTimer::singleShot(delay, this, [this, data]() {
            if (socket_->state() == QAbstractSocket::ConnectedState) {
                socket_->write(reinterpret_cast<const char*>(data.data()), data.size());
            }
        });
    } else {
        socket_->write(reinterpret_cast<const char*>(data.data()), data.size());
    }
}

void TcpChannel::write(const char* data, size_t size) {
    std::vector<uint8_t> vec(data, data + size);
    write(vec);
}

ChannelState TcpChannel::state() const {
    return state_;
}

QString TcpChannel::errorString() const {
    return socket_->errorString();
}

void TcpChannel::onSocketConnected() {
    state_ = ChannelState::Open;
    spdlog::info("TcpChannel Connected");
    emit opened();
}

void TcpChannel::onSocketDisconnected() {
    state_ = ChannelState::Closed;
    spdlog::info("TcpChannel Disconnected");
    emit closed();
}

void TcpChannel::onSocketError(QAbstractSocket::SocketError error) {
    state_ = ChannelState::Error;
    spdlog::error("TcpChannel Error: {}", socket_->errorString().toStdString());
    emit errorOccurred(socket_->errorString());
}

void TcpChannel::onSocketReadyRead() {
    QByteArray bytes = socket_->readAll();
    if (bytes.isEmpty()) return;
    
    std::vector<uint8_t> data(bytes.begin(), bytes.end());
    
    if (shouldDrop()) {
        // Simulating packet loss (RX)
        return;
    }
    
    int delay = getDelay();
    if (delay > 0) {
        QTimer::singleShot(delay, this, [this, data]() {
            emit dataReceived(data);
        });
    } else {
        emit dataReceived(data);
    }
}
