/**
 * @file TcpChannel.cpp
 * @brief Implementation of TcpChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpChannel.h"
#include "infra/logging/Logger.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <QThread>
#include <QMetaObject>

namespace io {

namespace {

unsigned long long threadToken(QThread* thread)
{
    return static_cast<unsigned long long>(reinterpret_cast<quintptr>(thread));
}

} // namespace

TcpChannel::TcpChannel() {
    connectTimer_.setSingleShot(true);
    connectTimer_.callOnTimeout([this]() {
        spdlog::warn("TcpChannel: connect timeout to {}:{}", ip_.toStdString(), port_);
        socket_.abort();
        setState(ChannelState::Error);
        emitError(QStringLiteral("TCP connect timeout (%1:%2)").arg(ip_).arg(port_));
    });

    QObject::connect(&socket_, &QTcpSocket::connected, &socket_, [this]() {
        onConnected();
    }, Qt::QueuedConnection);
    QObject::connect(&socket_, &QTcpSocket::bytesWritten, &socket_, [this](qint64 bytes) {
        onBytesWritten(bytes);
    }, Qt::QueuedConnection);
    QObject::connect(&socket_, &QTcpSocket::readyRead, &socket_, [this]() {
        onReadyRead();
    }, Qt::QueuedConnection);
    QObject::connect(&socket_, &QTcpSocket::errorOccurred, &socket_, [this](QAbstractSocket::SocketError error) {
        onSocketError(error);
    }, Qt::QueuedConnection);
    QObject::connect(&socket_, &QTcpSocket::stateChanged, &socket_, [this](QAbstractSocket::SocketState state) {
        onStateChanged(state);
    }, Qt::QueuedConnection);
}

TcpChannel::~TcpChannel() {
    assertOwnerThreadForDestruction("TcpChannel");
    close();
}

QString TcpChannel::logContext() const {
    return QStringLiteral("endpoint=%1:%2").arg(ip_).arg(port_);
}

bool TcpChannel::open() {
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        return QMetaObject::invokeMethod(&socket_, [this]() {
            open();
        }, Qt::QueuedConnection);
    }

    logThreadContextOnce("TcpChannel::open", openThreadLoggedFlag());

    connectTimer_.stop();

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        setState(ChannelState::Open);
        flushPendingWrites();
        return true;
    }
    if (socket_.state() == QAbstractSocket::ConnectingState) {
        setState(ChannelState::Opening);
        return true;
    }

    setClosing(false);
    resetWriteState();

    QHostAddress addr(ip_);
    if (addr.isNull() || addr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol) {
        QString err = QCoreApplication::translate("TcpChannel",
            "Invalid IP address: %1").arg(ip_);
        spdlog::warn("TcpChannel: {}", err.toStdString());
        setState(ChannelState::Error);
        emitError(err);
        return false;
    }
    if (port_ < 1 || port_ > 65535) {
        QString err = QCoreApplication::translate("TcpChannel",
            "Invalid port: %1 (expected 1-65535)").arg(port_);
        spdlog::warn("TcpChannel: {}", err.toStdString());
        setState(ChannelState::Error);
        emitError(err);
        return false;
    }

    setState(ChannelState::Opening);
    socket_.abort();
    socket_.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    socket_.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    socket_.connectToHost(ip_, port_);
    connectTimer_.start(std::max(3000, timeouts().readMs));
    return true;
}

void TcpChannel::moveToThread(QThread* thread) {
    spdlog::debug("TcpChannel: moveToThread current={} target={}",
                              threadToken(socket_.thread()),
                              threadToken(thread));
    ChannelBase::moveToThread(thread);
    socket_.moveToThread(thread);
    connectTimer_.moveToThread(thread);
    moveWriteInfrastructureToThread(thread);
    // Invariant: socket_ and writeTimeoutTimer_ now both live on @p thread.
    // moveWriteInfrastructureToThread moves the timer to the same target, so
    // the invariant is guaranteed by construction.
}

void TcpChannel::close() {
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            setClosing(false);
            resetWriteState();
            setState(ChannelState::Closed);
            return;
        }
        QMetaObject::invokeMethod(&socket_, [this]() {
            this->close();
        }, Qt::QueuedConnection);
        return;
    }

    resetWriteState();
    disarmWriteTimeout();
    connectTimer_.stop();
    setState(ChannelState::Closing);
    setClosing(true);
    if (socket_.state() == QAbstractSocket::UnconnectedState) {
        socket_.close();
        setClosing(false);
        setState(ChannelState::Closed);
        return;
    }
    if (socket_.state() == QAbstractSocket::ConnectedState ||
        socket_.state() == QAbstractSocket::ConnectingState) {
        socket_.disconnectFromHost();
    }
    if (socket_.state() == QAbstractSocket::UnconnectedState) {
        socket_.close();
        setClosing(false);
        setState(ChannelState::Closed);
    }
}

void TcpChannel::setEndpoint(const QString& ip, int port) {
    // Defensive: if the socket is in an active state, tear down the old
    // connection before changing the endpoint to prevent stale connections.
    if (socket_.state() == QAbstractSocket::ConnectedState ||
        socket_.state() == QAbstractSocket::ConnectingState) {
        close();
    }
    ip_ = ip;
    port_ = port;
}

void TcpChannel::onReadyRead() {
    assertOwnerThread(socket_, __func__);
    logThreadContextOnce("TcpChannel::onReadyRead", ioThreadLoggedFlag());
    QByteArray data = socket_.readAll();
    if (!data.isEmpty()) {
        spdlog::debug("TcpChannel: Received {} bytes", data.size());
        addRx(data.size());
        emitMonitor(false, data);
        emitRead(data);
    }
}

void TcpChannel::onConnected() {
    assertOwnerThread(socket_, __func__);
    logThreadContextOnce("TcpChannel::onConnected", ioThreadLoggedFlag());
    connectTimer_.stop();
    setClosing(false);
    setState(ChannelState::Open);
    flushPendingWrites();
}

void TcpChannel::onSocketError(QAbstractSocket::SocketError error) {
    assertOwnerThread(socket_, __func__);
    connectTimer_.stop();
    if (isClosing()) {
        return;
    }
    const QString endpoint = QStringLiteral("%1:%2")
        .arg(socket_.peerAddress().toString())
        .arg(socket_.peerPort());
    const QString errorText = socket_.errorString().isEmpty()
        ? QStringLiteral("TCP socket error")
        : socket_.errorString();
    spdlog::warn("TcpChannel: socket error code={} endpoint={} message={}",
                 static_cast<int>(error),
                 endpoint.toStdString(),
                 errorText.toStdString());
    resetWriteState();
    disarmWriteTimeout();
    setState(ChannelState::Error);
    emitError(QStringLiteral("TCP socket error (%1): %2")
                  .arg(static_cast<int>(error))
                  .arg(errorText));
}

void TcpChannel::onStateChanged(QAbstractSocket::SocketState state) {
    assertOwnerThread(socket_, __func__);
    switch (state) {
        case QAbstractSocket::ConnectedState:
            setState(ChannelState::Open);
            flushPendingWrites();
            break;
        case QAbstractSocket::UnconnectedState:
            resetWriteState();
            disarmWriteTimeout();
            setClosing(false);
            setState(ChannelState::Closed);
            break;
        case QAbstractSocket::ConnectingState:
            setState(ChannelState::Opening);
            break;
        case QAbstractSocket::ClosingState:
            setState(ChannelState::Closing);
            break;
        default:
            break;
    }
}

bool TcpChannel::adoptSocketDescriptor(qintptr socketDescriptor) {
    if (socketDescriptor < 0) {
        return false;
    }
    if (!socket_.setSocketDescriptor(socketDescriptor)) {
        spdlog::error("TcpChannel: setSocketDescriptor failed: {}",
                      socket_.errorString().toStdString());
        return false;
    }
    ip_ = socket_.peerAddress().toString();
    port_ = socket_.peerPort();
    setState(ChannelState::Open);
    return true;
}

}
