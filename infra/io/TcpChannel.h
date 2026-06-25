/**
 * @file TcpChannel.h
 * @brief Header file for TcpChannel.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once
#include "core/AppConstants.h"
#include "ChannelBase.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <deque>

namespace io {

/**
 * @brief TCP channel with explicit IO-thread affinity.
 *
 * Lifecycle contract:
 *   TcpChannel MUST be destroyed on the same thread that owns socket_ (the IO
 *   thread). Cross-thread destruction is a use-after-free: ~TcpChannel() calls
 *   close(), which on a cross-thread call queues a lambda to the IO thread and
 *   returns immediately; the destructor then destroys socket_/pendingWrites_
 *   while the IO thread may still access them. Callers (e.g. Presenter) must
 *   ensure ioThread quit()+wait() completes before releasing the channel.
 *
 * Thread affinity of handlers:
 *   All private slot methods (onReadyRead/onConnected/onBytesWritten/
 *   onSocketError/onStateChanged/onWriteTimeout) and write-state helpers
 *   (flushPendingWrites/resetWriteState) assume execution on the IO thread
 *   (socket_.thread()). After moveToThread(ioThread), signal handlers are
 *   delivered via Qt::QueuedConnection on the IO thread. Do not call private
 *   methods from the GUI thread.
 */
class TcpChannel : public ChannelBase {
public:
    TcpChannel();
    ~TcpChannel() noexcept override;

    ChannelKind kind() const override { return ChannelKind::Tcp; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;
    bool write(QByteArrayView data) override;
    
    void setEndpoint(const QString& ip, int port);
    bool adoptSocketDescriptor(qintptr socketDescriptor);

private:
    void logThreadContextOnce(const char* scope, bool& loggedFlag);
    void flushPendingWrites();
    void onReadyRead();
    void onConnected();
    void onBytesWritten(qint64 bytes);
    void onSocketError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onWriteTimeout();
    void armWriteTimeout();
    void disarmWriteTimeout();
    void resetWriteState();

    QTcpSocket socket_;
    QTimer writeTimeoutTimer_;
    QString ip_;
    int port_ = app::constants::Values::Network::kDefaultModbusTcpPort;
    std::deque<QByteArray> pendingWrites_;
    qsizetype currentWriteOffset_ = 0;
    bool closing_ = false;
    bool openThreadLogged_ = false;
    bool ioThreadLogged_ = false;
};

}
