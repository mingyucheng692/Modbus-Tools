/**
 * @file UdpChannel.h
 * @brief Header file for UdpChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ChannelBase.h"
#include <QUdpSocket>
#include <QHostAddress>

namespace io {

class UdpChannel : public ChannelBase {
public:
    UdpChannel();
    ~UdpChannel() override;

    ChannelKind kind() const override { return ChannelKind::Udp; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;
    bool write(QByteArrayView data) override;

    void setBindEndpoint(const QString& localIp, int localPort);
    void setRemotePeer(const QString& remoteIp, int remotePort);

private:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);

    QUdpSocket socket_;
    QHostAddress localAddress_;
    quint16 localPort_ = 0;
    bool hasRemotePeer_ = false;
    QHostAddress remoteAddress_;
    quint16 remotePort_ = 0;
    bool closing_ = false;
};

} // namespace io