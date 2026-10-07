// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once
#include "core/Config.h"
#include "BufferedWritingChannel.h"
#include <QTcpSocket>
#include <QHostAddress>

class QIODevice;

namespace io {

// Role: Buffered, non-blocking TCP communication channel wrapping QTcpSocket.
// Thread-safety: Thread-compatible. All operations must run on its IO-thread affinity.
// Ownership: Manages private QTcpSocket instance; must be destroyed on owning IO-thread.
class TcpChannel : public BufferedWritingChannel {
public:
    explicit TcpChannel(int closeLingerMs = kDefaultCloseLingerMs);
    ~TcpChannel() noexcept override;

    /// Production close-linger fallback (see constructor parameter).
    static constexpr int kDefaultCloseLingerMs = 2000;

    ChannelKind kind() const override { return ChannelKind::Tcp; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;

    void setEndpoint(const QString& ip, int port);
    /// Takes ownership of an externally created, already-connected socket
    /// (e.g. QTcpServer::nextPendingConnection()). The channel becomes the
    /// single owner of the object and its native descriptor — one socket
    /// engine per fd, no descriptor duplication (which is POSIX-only).
    bool adoptSocket(QTcpSocket* socket);

protected:
    QIODevice* device() override { return socket_; }
    bool isWritable() const override { return socket_->state() == QAbstractSocket::ConnectedState; }
    QString logContext() const override;
    QString errorPrefix() const override { return QStringLiteral("TCP"); }

private:
    void onReadyRead();
    void onConnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onLingerTimeout();
    /// Connects the five socket signal handlers to the current socket_.
    /// Called from the constructor and after adoptSocket() swaps the object.
    void wireSocketSignals();

    /// Owned raw pointer (deleted in ~TcpChannel), deliberately NOT a QObject
    /// child: the channel manages its affinity manually via moveToThread(),
    /// matching the historic value-member semantics. For server-accepted
    /// connections the object is adopted externally via adoptSocket().
    QTcpSocket* socket_;
    QTimer connectTimer_;
    /// Close fallback: force-aborts the socket if disconnectFromHost() cannot
    /// complete within closeLingerMs_ (stale kernel buffers, unresponsive peer)
    /// so the channel can never be stranded in Closing forever.
    QTimer lingerTimer_;
    int closeLingerMs_ = kDefaultCloseLingerMs;
    QString ip_;
    int port_ = config::Network::kDefaultModbusTcpPort;
    /// True while an unsolicited socket drop must be surfaced as an error:
    /// armed when a connect attempt starts or the session is established,
    /// cleared on user teardown (close/linger), connect timeout and error
    /// consumption. Linux delivers stateChanged(Unconnected) BEFORE
    /// errorOccurred, so the FSM can already show Closed when the error
    /// arrives — this flag distinguishes that genuine drop from a stale
    /// error after teardown (see onSocketError).
    bool socketDropIsError_ = false;
};

}
