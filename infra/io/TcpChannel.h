/**
 * @file TcpChannel.h
 * @brief Header file for TcpChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once
#include "core/Config.h"
#include "BufferedWritingChannel.h"
#include <QTcpSocket>
#include <QHostAddress>

class QIODevice;

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
class TcpChannel : public BufferedWritingChannel {
public:
    /// @param closeLingerMs  Fallback timer that force-aborts a socket stuck
    ///                       in Closing (e.g. disconnectFromHost blocked on a
    ///                       full kernel send buffer). Production default is
    ///                       kDefaultCloseLingerMs; tests inject a short value.
    explicit TcpChannel(int closeLingerMs = kDefaultCloseLingerMs);
    ~TcpChannel() noexcept override;

    /// Production close-linger fallback (see constructor parameter).
    static constexpr int kDefaultCloseLingerMs = 2000;

    ChannelKind kind() const override { return ChannelKind::Tcp; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;

    void setEndpoint(const QString& ip, int port);
    bool adoptSocketDescriptor(qintptr socketDescriptor);

protected:
    QIODevice* device() override { return &socket_; }
    bool isWritable() const override { return socket_.state() == QAbstractSocket::ConnectedState; }
    QString logContext() const override;
    QString errorPrefix() const override { return QStringLiteral("TCP"); }

private:
    void onReadyRead();
    void onConnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onLingerTimeout();

    QTcpSocket socket_;
    QTimer connectTimer_;
    /// Close fallback: force-aborts the socket if disconnectFromHost() cannot
    /// complete within closeLingerMs_ (stale kernel buffers, unresponsive peer)
    /// so the channel can never be stranded in Closing forever.
    QTimer lingerTimer_;
    int closeLingerMs_ = kDefaultCloseLingerMs;
    QString ip_;
    int port_ = config::Network::kDefaultModbusTcpPort;
};

}
