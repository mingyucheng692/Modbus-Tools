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
#include <unistd.h>

namespace io {

TcpServerHandle::TcpServerHandle(QObject* parent)
    : QObject(parent)
    // server_ MUST be a child of this handle: moveToThread() only relocates
    // an object together with its children. As a parentless value member it
    // stayed on the constructing (GUI) thread while ServerChannelWorker —
    // and this handle — moved to the server IO thread, so listen() and every
    // nextPendingConnection() ran cross-thread ("Cannot create children for
    // a parent that is in a different thread") and the spawned client
    // sockets were stranded on the wrong thread, killing their read
    // notifications (run6: NetworkDebuggerLoopback.TcpServer_SendReceiveAndStop).
    , server_(this)
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

        // Single-ownership hand-off: the channel adopts a PRIVATE duplicate
        // of the connection fd, then the QTcpServer-spawned socket object is
        // retired immediately (its destruction closes only the original fd).
        // Keeping both QAbstractSockets alive on one descriptor is UB in Qt:
        // both socket engines register read notifications for the same fd, so
        // peer data may be consumed by the dead-end source socket instead of
        // the channel (run6: NetworkDebuggerLoopback.TcpServer_SendReceiveAndStop
        // never received "DE AD BE EF" on Linux; a raw two-socket repro even
        // segfaults in the notification dispatch).
        const qintptr ownedFd = ::dup(socket->socketDescriptor());
        if (ownedFd < 0 || !channel->adoptSocketDescriptor(ownedFd)) {
            SPDLOG_ERROR("TcpServerHandle: failed to adopt socket for client {}", clientId);
            if (ownedFd >= 0) {
                ::close(ownedFd);
            }
            socket->close();
            socket->deleteLater();
            continue;
        }
        socket->deleteLater();

        ClientInfo info;
        info.clientId = clientId;
        info.peerAddress = socket->peerAddress().toString();
        info.peerPort = socket->peerPort();

        ClientEntry entry;
        entry.channel = channel;
        entry.info = info;
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
    // The adopted channel owns a dup()'ed descriptor since onNewConnection(),
    // so closing it above is the single, legal closesocket() for this
    // connection — no source socket to retire anymore.
    emit clientDisconnected(clientId);
}

int TcpServerHandle::nextClientId()
{
    return nextId_++;
}

} // namespace io