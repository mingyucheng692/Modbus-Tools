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
    ~TcpServerHandle() override;

    bool start(const QString& listenIp, int port, int maxClients);
    void stop();
    bool isListening() const;

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
    };

    QTcpServer server_;
    QHash<int, ClientEntry> clients_;
    int nextId_ = 1;
    int maxClients_ = 0;
};

} // namespace io