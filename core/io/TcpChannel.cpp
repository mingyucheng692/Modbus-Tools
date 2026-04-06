#include "TcpChannel.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

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
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        bool result = false;
        QMetaObject::invokeMethod(&socket_, [this, &result]() {
            result = this->open();
        }, Qt::BlockingQueuedConnection);
        return result;
    }

    if (isOpen()) return true;

    setState(ChannelState::Opening);
    socket_.connectToHost(ip_, port_);
    
    if (socket_.waitForConnected(timeouts().readMs)) {
        setState(ChannelState::Open);
        return true;
    } else {
        setState(ChannelState::Error);
        emitError(socket_.errorString());
        return false;
    }
}

void TcpChannel::moveToThread(QThread* thread) {
    socket_.moveToThread(thread);
}

void TcpChannel::close() {
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            setState(ChannelState::Closed);
            return;
        }
        QMetaObject::invokeMethod(&socket_, [this]() {
            this->close();
        }, Qt::BlockingQueuedConnection);
        return;
    }

    socket_.disconnectFromHost();
    if (socket_.state() != QAbstractSocket::UnconnectedState) {
        socket_.waitForDisconnected(1000);
    }
    socket_.close();
    setState(ChannelState::Closed);
}

bool TcpChannel::write(QByteArrayView data) {
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        bool result = false;
        QByteArray dataCopy(data.data(), data.size());
        QMetaObject::invokeMethod(&socket_, [this, dataCopy, &result]() {
            result = this->write(dataCopy);
        }, Qt::BlockingQueuedConnection);
        return result;
    }

    if (!isOpen()) return false;
    
    QByteArray dataBuffer(data.data(), data.size());
    qint64 written = socket_.write(dataBuffer);
    if (written != dataBuffer.size()) {
        return false;
    }

    // 确认字节已交给 OS 套接字发送队列，避免仅写入 Qt 缓冲区就判定成功。
    const bool flushed = socket_.bytesToWrite() == 0 || socket_.waitForBytesWritten(timeouts().writeMs);
    if (!flushed && socket_.bytesToWrite() > 0) {
        emitError(QStringLiteral("TCP write timeout before all bytes were sent"));
        return false;
    }

    addTx(written);
    emitMonitor(true, dataBuffer);
    return true;
}

void TcpChannel::setEndpoint(const QString& ip, int port) {
    ip_ = ip;
    port_ = port;
}

void TcpChannel::onReadyRead() {
    QByteArray data = socket_.readAll();
    if (!data.isEmpty()) {
        spdlog::info("TcpChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
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
