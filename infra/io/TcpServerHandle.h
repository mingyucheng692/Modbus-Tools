/**
 * @file TcpServerHandle.h
 * @brief Header file for TcpServerHandle.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "IChannel.h"
#include <QObject>
#include <QTcpServer>
#include <QHash>
#include <memory>

namespace io {

class TcpChannel;

struct ClientInfo {
    int clientId = 0;
    QString peerAddress;
    quint16 peerPort = 0;
};

class TcpServerHandle : public QObject {
    Q_OBJECT

public:
    explicit TcpServerHandle(QObject* parent = nullptr);
    ~TcpServerHandle() noexcept override;

    bool start(const QString& listenIp, int port, int maxClients);
    void stop();
    bool isListening() const;

    /// Port the listener is bound to (valid after a successful start()).
    /// Mainly for tests that pass 0 to request an ephemeral port.
    quint16 listenPort() const { return server_.serverPort(); }

    IChannel* clientChannel(int clientId) const;
    QList<ClientInfo> clientList() const;
    int clientCount() const;

signals:
    void clientConnected(int clientId, const QString& peerAddress, quint16 peerPort);
    void clientDisconnected(int clientId);
    void errorOccurred(const QString& error);

private slots:
    void onNewConnection();

private:
    int nextClientId();
    void removeClient(int clientId);

    struct ClientEntry {
        std::shared_ptr<TcpChannel> channel;
        ClientInfo info;
        IChannel::HandlerId stateHandlerId = 0;
        /// The QTcpServer-spawned socket whose native descriptor was adopted
        /// by @p channel. Ownership stays HERE (never deleteLater in
        /// onNewConnection): both objects reference the same OS socket
        /// handle, and destroying the source socket first closes the
        /// descriptor out from under the channel, silently killing all
        /// passive-loss notifications. Teardown order lives in removeClient().
        QTcpSocket* sourceSocket = nullptr;
    };

    QTcpServer server_;
    QHash<int, ClientEntry> clients_;
    int nextId_ = 1;
    int maxClients_ = 0;
};

} // namespace io
