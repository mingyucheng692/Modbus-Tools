#include "TcpChannel.h"

namespace io {

TcpChannel::TcpChannel() {
    QObject::connect(&socket_, &QTcpSocket::readyRead, [this]() {
        onReadyRead();
    });
    // Qt6 signature change for error signal? Use errorOccurred if available or static cast
    QObject::connect(&socket_, &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
        onSocketError(error);
    });
    QObject::connect(&socket_, &QTcpSocket::stateChanged, [this](QAbstractSocket::SocketState state) {
        onStateChanged(state);
    });
}

TcpChannel::~TcpChannel() {
    close();
}

bool TcpChannel::open() {
    if (isOpen()) return true;

    setState(ChannelState::Opening);
    socket_.connectToHost(ip_, port_);
    
    if (socket_.waitForConnected(timeouts().readMs)) {
        // state changed signal will handle setState(Open)
        return true;
    } else {
        setState(ChannelState::Error);
        emitError(socket_.errorString());
        return false;
    }
}

void TcpChannel::close() {
    socket_.disconnectFromHost();
    if (socket_.state() != QAbstractSocket::UnconnectedState) {
        socket_.waitForDisconnected(1000);
    }
    socket_.close();
    setState(ChannelState::Closed);
}

bool TcpChannel::write(QByteArrayView data) {
    if (!isOpen()) return false;
    
    qint64 written = socket_.write(data.data(), data.size());
    if (written == data.size()) {
        addTx(written);
        // socket_.waitForBytesWritten(timeouts().writeMs);
        return true;
    }
    return false;
}

void TcpChannel::setEndpoint(const QString& ip, int port) {
    ip_ = ip;
    port_ = port;
}

void TcpChannel::onReadyRead() {
    QByteArray data = socket_.readAll();
    if (!data.isEmpty()) {
        addRx(data.size());
        emitRead(data);
    }
}

void TcpChannel::onSocketError(QAbstractSocket::SocketError error) {
    emitError(socket_.errorString());
}

void TcpChannel::onStateChanged(QAbstractSocket::SocketState state) {
    switch (state) {
        case QAbstractSocket::ConnectedState:
            setState(ChannelState::Open);
            break;
        case QAbstractSocket::UnconnectedState:
            setState(ChannelState::Closed);
            break;
        default:
            break;
    }
}

}
