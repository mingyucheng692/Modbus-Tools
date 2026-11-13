// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "UdpChannel.h"
#include <QThread>
#include <QMetaObject>
#include <spdlog/spdlog.h>

namespace io {

UdpChannel::UdpChannel()
{
    // Functor-based connect with an explicit connection type requires a context
    // QObject (5-arg overload). UdpChannel itself is not a QObject, so the
    // signal sender is passed as the context. Because the sender lives on the
    // IO thread (after moveToThread), Qt::QueuedConnection guarantees the
    // functor executes on the IO thread regardless of which thread emits.
    QObject::connect(&socket_, &QUdpSocket::readyRead, &socket_, [this]() {
        onReadyRead();
    }, Qt::QueuedConnection);
    QObject::connect(&socket_, &QUdpSocket::errorOccurred, &socket_, [this](QAbstractSocket::SocketError error) {
        onSocketError(error);
    }, Qt::QueuedConnection);
    QObject::connect(&socket_, &QUdpSocket::stateChanged, &socket_, [this](QAbstractSocket::SocketState state) {
        onStateChanged(state);
    }, Qt::QueuedConnection);
}

UdpChannel::~UdpChannel()
{
    assertOwnerThreadForDestruction("UdpChannel");
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
        if (state() == ChannelState::Open) {
            return true;
        }
        socket_.abort();
    }

    closing_ = false;
    setState(ChannelState::Opening);
    socket_.abort();

    if (!socket_.bind(localAddress_, localPort_, QUdpSocket::ShareAddress)) {
        const QString err = socket_.errorString().isEmpty()
            ? QStringLiteral("UDP bind failed")
            : socket_.errorString();
        SPDLOG_ERROR("UdpChannel: bind failed {}:{} error={}",
                      localAddress_.toString().toStdString(),
                      localPort_,
                      err.toStdString());
        setState(ChannelState::Error);
        emitError(ChannelErrorCode::ConnectionFailed, err);
        return false;
    }

    setState(ChannelState::Open);
    return true;
}

void UdpChannel::moveToThread(QThread* thread)
{
    ChannelBase::moveToThread(thread);
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

    if (state() != ChannelState::Open || socket_.state() != QAbstractSocket::BoundState) {
        return false;
    }

    QByteArray buffer(data.data(), data.size());
    if (buffer.isEmpty()) {
        return true;
    }

    if (!hasRemotePeer_ || remotePort_ == 0 || remoteAddress_.isNull()) {
        SPDLOG_WARN("UdpChannel: write rejected because remote peer is not configured");
        emitError(ChannelErrorCode::WriteFailed,
                  QStringLiteral("UDP remote peer is not configured"));
        return false;
    }
    const qint64 sent = socket_.writeDatagram(buffer, remoteAddress_, remotePort_);

    if (sent < 0) {
        SPDLOG_ERROR("UdpChannel: writeDatagram failed error={}",
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
        if (size < 0) {
            break;
        }
        if (size == 0) {
            socket_.readDatagram(nullptr, 0);
            continue;
        }

        QByteArray buffer(static_cast<qsizetype>(size), Qt::Uninitialized);
        QHostAddress senderAddr;
        quint16 senderPort = 0;
        const qint64 read = socket_.readDatagram(buffer.data(), size, &senderAddr, &senderPort);
        if (read < 0) {
            break;
        }
        if (read == 0) {
            continue;
        }

        buffer.resize(static_cast<qsizetype>(read));

        if (hasRemotePeer_) {
            const bool broadcastTarget = remoteAddress_.isBroadcast() ||
                                         remoteAddress_ == QHostAddress::Broadcast;
            if (broadcastTarget) {
                if (senderPort != remotePort_) {
                    continue;
                }
            } else if (!senderAddr.isEqual(remoteAddress_, QHostAddress::TolerantConversion) ||
                       senderPort != remotePort_) {
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

    const QString err = socket_.errorString().isEmpty()
        ? QStringLiteral("UDP socket error")
        : socket_.errorString();
    SPDLOG_ERROR("UdpChannel: socket error {}", err.toStdString());
    setState(ChannelState::Error);
    emitError(ChannelErrorCode::ConnectionFailed, err);
}

void UdpChannel::onStateChanged(QAbstractSocket::SocketState socketState)
{
    if (closing_ || socketState != socket_.state() || state() == ChannelState::Error) {
        return;
    }

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
