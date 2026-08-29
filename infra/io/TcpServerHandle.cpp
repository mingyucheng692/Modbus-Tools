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
#include <QMetaObject>
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
        // Synchronous failure: reported through the return value so the
        // caller (ServerChannelWorker::openTcpServer) can emit ONE coherent
        // error + terminal stateChanged(Closed) pair. Emitting errorOccurred
        // here as well used to produce a duplicate channelErrorOccurred and
        // an Error state that left the UI stranded on "Connecting".
        SPDLOG_ERROR("TcpServerHandle: listen failed {}:{} error={}",
                      listenIp.toStdString(), port,
                      server_.errorString().toStdString());
        return false;
    }

    SPDLOG_INFO("TcpServerHandle: listening on {}:{} maxClients={}",
                 listenIp.toStdString(), port, maxClients);
    return true;
}

void TcpServerHandle::stop()
{
    server_.close();
    // removeClient() erases each entry (and detaches its state handler),
    // so iterating a snapshot of the ids is safe; clients_ ends up empty.
    const auto clientIds = clients_.keys();
    for (int id : clientIds) {
        removeClient(id);
    }
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
    // The socket from nextPendingConnection() and the TcpChannel share
    // one native descriptor after adoptSocketDescriptor(). The source
    // QTcpSocket is retained in ClientEntry (NOT deleteLater'd here):
    // destroying it would close the shared descriptor out from under
    // the channel and kill all passive-loss notifications. Teardown
    // order lives in removeClient().
    while (server_.hasPendingConnections()) {
        QTcpSocket* socket = server_.nextPendingConnection();
        if (!socket) continue;

        if (maxClients_ > 0 && clients_.size() >= maxClients_) {
            SPDLOG_WARN("TcpServerHandle: rejecting connection from {}, max clients ({}) reached",
                         socket->peerAddress().toString().toStdString(),
                         maxClients_);
            socket->close();
            socket->deleteLater();
            continue;
        }

        const int clientId = nextClientId();
        auto channel = std::make_shared<TcpChannel>();

        if (!channel->adoptSocketDescriptor(socket->socketDescriptor())) {
            SPDLOG_ERROR("TcpServerHandle: failed to adopt socket for client {}", clientId);
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
        entry.sourceSocket = socket;
        // Passive-loss subscription: when the peer goes away (RST, cable
        // pull, orderly FIN) the adopted TcpChannel reports Closed/Error
        // and the client is removed here. Active removals (removeClient)
        // erase the entry BEFORE closing the channel, so the contains()
        // check inside the handler distinguishes user-initiated closes
        // from passive losses and cannot re-enter.
        //
        // The removal itself MUST be deferred: this handler runs inside
        // the channel socket's own error/state notification dispatch, and
        // removeClient() destroys the TcpChannel (with its member
        // QTcpSocket) at scope exit — tearing the socket down in the
        // middle of its own signal dispatch wedges the Windows event
        // dispatcher (processEvents never returns). Queuing to the next
        // event-loop turn lets the current socket dispatch complete
        // first.
        entry.stateHandlerId = channel->addStateHandler(
            [this, clientId](ChannelState state) {
                if (state != ChannelState::Closed
                    && state != ChannelState::Error) {
                    return;
                }
                if (!clients_.contains(clientId)) {
                    return; // user-initiated removal path
                }
                QMetaObject::invokeMethod(this, [this, clientId]() {
                    if (!clients_.contains(clientId)) {
                        return; // removed by stop()/removeClient meanwhile
                    }
                    removeClient(clientId);
                }, Qt::QueuedConnection);
            });
        clients_.insert(clientId, std::move(entry));

        SPDLOG_INFO("TcpServerHandle: client {} connected from {}:{}",
                     clientId,
                     info.peerAddress.toStdString(),
                     info.peerPort);
        emit clientConnected(clientId, info.peerAddress, info.peerPort);
    }
}

void TcpServerHandle::removeClient(int clientId)
{
    auto it = clients_.find(clientId);
    if (it == clients_.end()) return;

    SPDLOG_INFO("TcpServerHandle: client {} disconnected", clientId);

    // Erase BEFORE closing: the channel state handler installed in
    // onNewConnection() treats a close of a still-registered client as a
    // passive loss and would re-enter removeClient; erasing first makes the
    // contains() check fail so the close is treated as user-initiated.
    auto entry = std::move(it.value());
    clients_.erase(it);

    if (auto* ch = entry.channel.get()) {
        ch->removeStateHandler(entry.stateHandlerId);
        ch->close();
    }
    if (entry.sourceSocket) {
        // The source socket and the channel share one OS socket handle.
        // The channel's close() above is the FIRST (and only legal)
        // closesocket(); closing the source socket afterwards makes the
        // second closesocket() hit an already-dead descriptor on this same
        // thread, which Windows safely reports as an error instead of
        // closing a recycled handle. Deleting the source socket here also
        // retires the QTcpServer child ownership.
        entry.sourceSocket->close();
        delete entry.sourceSocket;
    }
    emit clientDisconnected(clientId);
}

int TcpServerHandle::nextClientId()
{
    return nextId_++;
}

} // namespace io