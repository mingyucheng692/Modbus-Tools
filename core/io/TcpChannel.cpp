#include "TcpChannel.h"
#include <spdlog/spdlog.h>
#include <QEventLoop>
#include <QThread>
#include <QMetaObject>
#include <QTimer>

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
    socket_.abort();
    socket_.connectToHost(ip_, port_);
    socket_.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    socket_.setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        setState(ChannelState::Open);
        return true;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(&socket_, &QTcpSocket::connected, &loop, &QEventLoop::quit);
    QObject::connect(&socket_, &QTcpSocket::errorOccurred, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(std::max(1, timeouts().readMs));
    loop.exec();

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        setState(ChannelState::Open);
        return true;
    }

    socket_.abort();
    setState(ChannelState::Error);
    emitError(socket_.errorString().isEmpty() ? QStringLiteral("TCP connect timeout") : socket_.errorString());
    return false;
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

    setState(ChannelState::Closing);
    if (socket_.state() == QAbstractSocket::ConnectedState) {
        socket_.disconnectFromHost();
    }
    socket_.abort();
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
    socket_.flush();

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
    Q_UNUSED(error);
    setState(ChannelState::Error);
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
