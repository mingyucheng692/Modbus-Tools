/**
 * @file UdpChannel.cpp
 * @brief Implementation of UdpChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UdpChannel.h"
#include <QThread>
#include <QMetaObject>
#include <spdlog/spdlog.h>

namespace io {

UdpChannel::UdpChannel()
{
    QObject::connect(&socket_, &QUdpSocket::readyRead, [this]() {
        onReadyRead();
    });
    QObject::connect(&socket_, &QUdpSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
        onSocketError(error);
    });
    QObject::connect(&socket_, &QUdpSocket::stateChanged, [this](QAbstractSocket::SocketState state) {
        onStateChanged(state);
    });
}

UdpChannel::~UdpChannel()
{
    close();
}

void UdpChannel::setBindEndpoint(const QString& localIp, int localPort)
{
    localAddress_ = QHostAddress(localIp);
    localPort_ = static_cast<quint16>(localPort);
}

void UdpChannel::setRemotePeer(const QString& remoteIp, int remotePort)
{
    if (remoteIp.isEmpty() || remotePort <= 0) {
        hasRemotePeer_ = false;
        return;
    }
    hasRemotePeer_ = true;
    remoteAddress_ = QHostAddress(remoteIp);
    remotePort_ = static_cast<quint16>(remotePort);
}

bool UdpChannel::open()
{
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        return QMetaObject::invokeMethod(&socket_, [this]() {
            open();
        }, Qt::QueuedConnection);
    }

    if (socket_.state() == QAbstractSocket::BoundState) {
        setState(ChannelState::Open);
        return true;
    }

    closing_ = false;
    setState(ChannelState::Opening);
    socket_.abort();

    if (!socket_.bind(localAddress_, localPort_, QUdpSocket::ShareAddress)) {
        spdlog::error("UdpChannel: bind failed {}:{} error={}",
                      localAddress_.toString().toStdString(),
                      localPort_,
                      socket_.errorString().toStdString());
        emitError(socket_.errorString().isEmpty()
                      ? QStringLiteral("UDP bind failed")
                      : socket_.errorString());
        return false;
    }

    setState(ChannelState::Open);
    return true;
}

void UdpChannel::moveToThread(QThread* thread)
{
    socket_.moveToThread(thread);
}

void UdpChannel::close()
{
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            closing_ = false;
            setState(ChannelState::Closed);
            return;
        }
        QMetaObject::invokeMethod(&socket_, [this]() {
            this->close();
        }, Qt::QueuedConnection);
        return;
    }

    setState(ChannelState::Closing);
    closing_ = true;
    socket_.close();
    closing_ = false;
    setState(ChannelState::Closed);
}

bool UdpChannel::write(QByteArrayView data)
{
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        QByteArray dataCopy(data.data(), data.size());
        return QMetaObject::invokeMethod(&socket_, [this, dataCopy]() {
            write(dataCopy);
        }, Qt::QueuedConnection);
    }

    if (socket_.state() != QAbstractSocket::BoundState) {
        return false;
    }

    QByteArray buffer(data.data(), data.size());
    if (buffer.isEmpty()) {
        return true;
    }

    qint64 sent = 0;
    if (hasRemotePeer_) {
        sent = socket_.writeDatagram(buffer, remoteAddress_, remotePort_);
    } else {
        sent = socket_.writeDatagram(buffer, localAddress_, localPort_);
    }

    if (sent < 0) {
        spdlog::error("UdpChannel: writeDatagram failed error={}",
                      socket_.errorString().toStdString());
        return false;
    }

    emitMonitor(true, buffer);
    addTx(buffer.size());
    return true;
}

void UdpChannel::onReadyRead()
{
    while (socket_.hasPendingDatagrams()) {
        const qint64 size = socket_.pendingDatagramSize();
        if (size <= 0) continue;

        QByteArray buffer(static_cast<qsizetype>(size), Qt::Uninitialized);
        QHostAddress senderAddr;
        quint16 senderPort = 0;
        const qint64 read = socket_.readDatagram(buffer.data(), size, &senderAddr, &senderPort);
        if (read <= 0) continue;

        buffer.resize(static_cast<qsizetype>(read));

        if (hasRemotePeer_) {
            if (senderAddr != remoteAddress_ || senderPort != remotePort_) {
                continue;
            }
        }

        emitMonitor(false, buffer);
        addRx(buffer.size());
        emitRead(buffer);
    }
}

void UdpChannel::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    if (closing_) return;

    spdlog::error("UdpChannel: socket error {}",
                  socket_.errorString().toStdString());
    emitError(socket_.errorString());
}

void UdpChannel::onStateChanged(QAbstractSocket::SocketState socketState)
{
    if (closing_) return;

    switch (socketState) {
    case QAbstractSocket::UnconnectedState:
        setState(ChannelState::Closed);
        break;
    case QAbstractSocket::BoundState:
        setState(ChannelState::Open);
        break;
    default:
        break;
    }
}

} // namespace io