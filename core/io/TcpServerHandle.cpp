/**
 * @file TcpServerHandle.cpp
 * @brief Implementation of TcpServerHandle.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpServerHandle.h"
#include "TcpChannel.h"
#include <QTcpSocket>
#include <spdlog/spdlog.h>

namespace io {

TcpServerHandle::TcpServerHandle(QObject* parent)
    : QObject(parent)
{
    QObject::connect(&server_, &QTcpServer::newConnection, this, &TcpServerHandle::onNewConnection);
}

TcpServerHandle::~TcpServerHandle()
{
    stop();
}

bool TcpServerHandle::start(const QString& listenIp, int port, int maxClients)
{
    stop();
    maxClients_ = maxClients;

    const QHostAddress addr(listenIp);
    if (!server_.listen(addr, static_cast<quint16>(port))) {
        spdlog::error("TcpServerHandle: listen failed {}:{} error={}",
                      listenIp.toStdString(), port,
                      server_.errorString().toStdString());
        emit errorOccurred(server_.errorString().isEmpty()
                               ? QStringLiteral("TCP server listen failed")
                               : server_.errorString());
        return false;
    }

    spdlog::info("TcpServerHandle: listening on {}:{} maxClients={}",
                 listenIp.toStdString(), port, maxClients);
    return true;
}

void TcpServerHandle::stop()
{
    server_.close();
    const auto clientIds = clients_.keys();
    for (int id : clientIds) {
        removeClient(id);
    }
    clients_.clear();
}

bool TcpServerHandle::isListening() const
{
    return server_.isListening();
}

IChannel* TcpServerHandle::clientChannel(int clientId) const
{
    auto it = clients_.find(clientId);
    return it != clients_.end() ? it->channel.get() : nullptr;
}

QList<ClientInfo> TcpServerHandle::clientList() const
{
    QList<ClientInfo> list;
    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        list.append(it->info);
    }
    return list;
}

int TcpServerHandle::clientCount() const
{
    return clients_.size();
}

void TcpServerHandle::onNewConnection()
{
    while (server_.hasPendingConnections()) {
        QTcpSocket* socket = server_.nextPendingConnection();
        if (!socket) continue;

        if (maxClients_ > 0 && clients_.size() >= maxClients_) {
            spdlog::warn("TcpServerHandle: rejecting connection from {}, max clients ({}) reached",
                         socket->peerAddress().toString().toStdString(),
                         maxClients_);
            socket->close();
            socket->deleteLater();
            continue;
        }

        const int clientId = nextClientId();
        auto channel = std::make_shared<TcpChannel>();

        if (!channel->adoptSocketDescriptor(socket->socketDescriptor())) {
            spdlog::error("TcpServerHandle: failed to adopt socket for client {}", clientId);
            socket->close();
            socket->deleteLater();
            continue;
        }

        ClientInfo info;
        info.clientId = clientId;
        info.peerAddress = socket->peerAddress().toString();
        info.peerPort = socket->peerPort();

        ClientEntry entry;
        entry.channel = channel;
        entry.info = info;
        clients_.insert(clientId, entry);

        spdlog::info("TcpServerHandle: client {} connected from {}:{}",
                     clientId,
                     info.peerAddress.toStdString(),
                     info.peerPort);
        emit clientConnected(clientId, info.peerAddress, info.peerPort);

        socket->deleteLater();
    }
}

void TcpServerHandle::removeClient(int clientId)
{
    auto it = clients_.find(clientId);
    if (it == clients_.end()) return;

    if (auto* ch = it->channel.get()) {
        ch->close();
    }
    clients_.erase(it);
    emit clientDisconnected(clientId);
}

int TcpServerHandle::nextClientId()
{
    return nextId_++;
}

} // namespace io