/**
 * @file TcpChannel.h
 * @brief Header file for TcpChannel.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once
#include "AppConstants.h"
#include "ChannelBase.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <deque>

namespace io {

class TcpChannel : public ChannelBase {
public:
    TcpChannel();
    ~TcpChannel() override;

    ChannelKind kind() const override { return ChannelKind::Tcp; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;
    bool write(QByteArrayView data) override;
    
    void setEndpoint(const QString& ip, int port);

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
