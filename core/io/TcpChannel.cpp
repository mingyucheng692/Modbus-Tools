#include "TcpChannel.h"
#include <spdlog/spdlog.h>
#include <QThread>
#include <QMetaObject>

namespace io {

TcpChannel::TcpChannel() {
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
        return;
    }

    auto& frame = pendingWrites_.front();
    while (currentWriteOffset_ < frame.size()) {
        const char* dataPtr = frame.constData() + currentWriteOffset_;
        const qint64 remaining = static_cast<qint64>(frame.size() - currentWriteOffset_);
        const qint64 accepted = socket_.write(dataPtr, remaining);
        if (accepted < 0) {
            setState(ChannelState::Error);
            emitError(socket_.errorString().isEmpty() ? QStringLiteral("TCP write failed") : socket_.errorString());
            return;
        }
        if (accepted == 0) {
            break;
        }
        currentWriteOffset_ += static_cast<qsizetype>(accepted);
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
    while (!pendingWrites_.empty() &&
           currentWriteOffset_ >= pendingWrites_.front().size() &&
           socket_.bytesToWrite() == 0) {
        addTx(pendingWrites_.front().size());
        pendingWrites_.pop_front();
        currentWriteOffset_ = 0;
    }
    flushPendingWrites();
}

void TcpChannel::onSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    if (closing_) {
        return;
    }
    resetWriteState();
    setState(ChannelState::Error);
    emitError(socket_.errorString().isEmpty() ? QStringLiteral("TCP socket error") : socket_.errorString());
}

void TcpChannel::onStateChanged(QAbstractSocket::SocketState state) {
    switch (state) {
        case QAbstractSocket::ConnectedState:
            setState(ChannelState::Open);
            flushPendingWrites();
            break;
        case QAbstractSocket::UnconnectedState:
            resetWriteState();
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

void TcpChannel::resetWriteState() {
    pendingWrites_.clear();
    currentWriteOffset_ = 0;
}

}
