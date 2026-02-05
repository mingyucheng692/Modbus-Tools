#include "TcpChannel.h"
#include <spdlog/spdlog.h>

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
    write(reinterpret_cast<const char*>(data.data()), data.size());
}

void TcpChannel::write(const char* data, size_t size) {
    if (state_ != ChannelState::Open) return;
    socket_->write(data, size);
    socket_->flush();
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
    QByteArray data = socket_->readAll();
    if (data.isEmpty()) return;

    std::vector<uint8_t> vec(data.begin(), data.end());
    emit dataReceived(vec);
}
