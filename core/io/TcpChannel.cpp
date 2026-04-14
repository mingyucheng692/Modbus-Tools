/**
 * @file TcpChannel.cpp
 * @brief Implementation of TcpChannel.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpChannel.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

namespace io {

TcpChannel::TcpChannel() {
    writeTimeoutTimer_.setSingleShot(true);
    QObject::connect(&writeTimeoutTimer_, &QTimer::timeout, [this]() {
        onWriteTimeout();
    });
    QObject::connect(&socket_, &QTcpSocket::connected, [this]() {
        onConnected();
    });
    QObject::connect(&socket_, &QTcpSocket::bytesWritten, [this](qint64 bytes) {
        onBytesWritten(bytes);
    });
    QObject::connect(&socket_, &QTcpSocket::readyRead, [this]() {
        onReadyRead();
    });
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
        return QMetaObject::invokeMethod(&socket_, [this]() {
            open();
        }, Qt::QueuedConnection);
    }

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        setState(ChannelState::Open);
        flushPendingWrites();
        return true;
    }
    if (socket_.state() == QAbstractSocket::ConnectingState) {
        setState(ChannelState::Opening);
        return true;
    }

    closing_ = false;
    resetWriteState();
    setState(ChannelState::Opening);
    socket_.abort();
    socket_.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    socket_.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    socket_.connectToHost(ip_, port_);
    return true;
}

void TcpChannel::moveToThread(QThread* thread) {
    socket_.moveToThread(thread);
    writeTimeoutTimer_.moveToThread(thread);
}

void TcpChannel::close() {
    if (QThread::currentThread() != socket_.thread()) {
        QThread* ownerThread = socket_.thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            closing_ = false;
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
    setState(ChannelState::Closing);
    closing_ = true;
    if (socket_.state() == QAbstractSocket::UnconnectedState) {
        socket_.close();
        closing_ = false;
        setState(ChannelState::Closed);
        return;
    }
    if (socket_.state() == QAbstractSocket::ConnectedState ||
        socket_.state() == QAbstractSocket::ConnectingState) {
        socket_.disconnectFromHost();
    }
    if (socket_.state() == QAbstractSocket::UnconnectedState) {
        socket_.close();
        closing_ = false;
        setState(ChannelState::Closed);
    }
}

bool TcpChannel::write(QByteArrayView data) {
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

    if (socket_.state() != QAbstractSocket::ConnectedState) {
        return false;
    }

    QByteArray dataBuffer(data.data(), data.size());
    if (dataBuffer.isEmpty()) {
        return true;
    }

    pendingWrites_.push_back(dataBuffer);
    emitMonitor(true, dataBuffer);
    armWriteTimeout();
    flushPendingWrites();
    return true;
}

void TcpChannel::setEndpoint(const QString& ip, int port) {
    ip_ = ip;
    port_ = port;
}

void TcpChannel::flushPendingWrites() {
    if (QThread::currentThread() != socket_.thread()) {
        QMetaObject::invokeMethod(&socket_, [this]() {
            flushPendingWrites();
        }, Qt::QueuedConnection);
        return;
    }

    if (socket_.state() != QAbstractSocket::ConnectedState || pendingWrites_.empty()) {
        if (pendingWrites_.empty() && socket_.bytesToWrite() == 0) {
            disarmWriteTimeout();
        }
        return;
    }

    auto& frame = pendingWrites_.front();
    while (currentWriteOffset_ < frame.size()) {
        const char* dataPtr = frame.constData() + currentWriteOffset_;
        const qint64 remaining = static_cast<qint64>(frame.size() - currentWriteOffset_);
        const qint64 accepted = socket_.write(dataPtr, remaining);
        if (accepted < 0) {
            const QString err = socket_.errorString().isEmpty() ? QStringLiteral("TCP write failed") : socket_.errorString();
            spdlog::warn("TcpChannel: write failed endpoint={}:{} error={}",
                         ip_.toStdString(), port_, err.toStdString());
            resetWriteState();
            disarmWriteTimeout();
            setState(ChannelState::Error);
            emitError(err);
            return;
        }
        if (accepted == 0) {
            break;
        }
        currentWriteOffset_ += static_cast<qsizetype>(accepted);
        armWriteTimeout();
    }
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

void TcpChannel::onConnected() {
    closing_ = false;
    setState(ChannelState::Open);
    flushPendingWrites();
}

void TcpChannel::onBytesWritten(qint64 bytes) {
    Q_UNUSED(bytes);
    bool queueDrained = false;
    while (!pendingWrites_.empty() &&
           currentWriteOffset_ >= pendingWrites_.front().size() &&
           socket_.bytesToWrite() == 0) {
        addTx(pendingWrites_.front().size());
        pendingWrites_.pop_front();
        currentWriteOffset_ = 0;
        queueDrained = pendingWrites_.empty();
    }
    if (queueDrained && socket_.bytesToWrite() == 0) {
        disarmWriteTimeout();
        emitWriteDrained();
    } else if (!pendingWrites_.empty() || socket_.bytesToWrite() > 0) {
        armWriteTimeout();
    }
    flushPendingWrites();
}

void TcpChannel::onSocketError(QAbstractSocket::SocketError error) {
    if (closing_) {
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
    switch (state) {
        case QAbstractSocket::ConnectedState:
            setState(ChannelState::Open);
            flushPendingWrites();
            break;
        case QAbstractSocket::UnconnectedState:
            resetWriteState();
            disarmWriteTimeout();
            closing_ = false;
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

void TcpChannel::onWriteTimeout() {
    if (QThread::currentThread() != socket_.thread()) {
        QMetaObject::invokeMethod(&socket_, [this]() {
            onWriteTimeout();
        }, Qt::QueuedConnection);
        return;
    }

    if (closing_ || socket_.state() != QAbstractSocket::ConnectedState) {
        disarmWriteTimeout();
        return;
    }

    if (pendingWrites_.empty() && socket_.bytesToWrite() == 0) {
        disarmWriteTimeout();
        return;
    }

    const bool draining = socket_.bytesToWrite() > 0;
    spdlog::warn("TcpChannel: write timeout endpoint={}:{} draining={} pendingWrites={}",
                 ip_.toStdString(), port_, draining, pendingWrites_.size());
    resetWriteState();
    disarmWriteTimeout();
    setState(ChannelState::Error);
    emitError(draining
        ? QStringLiteral("TCP write drain timeout")
        : QStringLiteral("TCP write timeout"));
}

void TcpChannel::armWriteTimeout() {
    const int timeoutMs = timeouts().writeMs;
    if (timeoutMs <= 0) {
        return;
    }
    writeTimeoutTimer_.start(timeoutMs);
}

void TcpChannel::disarmWriteTimeout() {
    if (writeTimeoutTimer_.isActive()) {
        writeTimeoutTimer_.stop();
    }
}

void TcpChannel::resetWriteState() {
    pendingWrites_.clear();
    currentWriteOffset_ = 0;
}

}
