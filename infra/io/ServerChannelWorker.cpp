/**
 * @file ServerChannelWorker.cpp
 * @brief Implementation of ServerChannelWorker.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ServerChannelWorker.h"
#include "TcpServerHandle.h"
#include "TcpChannel.h"
#include <QMetaObject>
#include <spdlog/spdlog.h>

namespace io {

ServerChannelWorker::ServerChannelWorker(QObject* parent)
    : QObject(parent)
    , serverHandle_(new TcpServerHandle(this))
{
    QObject::connect(serverHandle_, &TcpServerHandle::clientConnected,
                     this, [this](int clientId, const QString& peerAddress, quint16 peerPort) {
        auto* channel = serverHandle_->clientChannel(clientId);
        if (!channel) return;

        channel->setReadHandler([this, clientId](QByteArrayView data) {
            QByteArray copy(data.data(), data.size());
            emit monitorWithClient(false, copy, clientId);
        });

        channel->setMonitor([this, clientId](bool isTx, const QByteArray& data) {
            emit monitorWithClient(isTx, data, clientId);
        });

        channel->setErrorHandler([this](const ChannelError& err) {
            emit channelErrorOccurred(QStringLiteral("TCP Server"), err.code, err.message);
        });

        const QString peerInfo = QStringLiteral("%1:%2").arg(peerAddress).arg(peerPort);
        emit clientConnected(clientId, peerInfo);
    });

    QObject::connect(serverHandle_, &TcpServerHandle::clientDisconnected,
                     this, &ServerChannelWorker::clientDisconnected);

    QObject::connect(serverHandle_, &TcpServerHandle::errorOccurred,
                     this, [this](const QString& error) {
        emit channelErrorOccurred(QStringLiteral("TCP Server"), io::ChannelErrorCode::ConnectionFailed, error);
        emit stateChanged(ChannelState::Error);
    });
}

ServerChannelWorker::~ServerChannelWorker()
{
    closeAllClients();
}

void ServerChannelWorker::openTcpServer(const QString& listenIp, int port, int maxClients)
{
    if (!serverHandle_->start(listenIp, port, maxClients)) {
        emit channelErrorOccurred(
            QStringLiteral("TCP Server"),
            io::ChannelErrorCode::ConnectionFailed,
            QStringLiteral("Failed to start TCP server"));
        // Terminal state after a failed listen: without this emission the
        // view never hears back from the worker and stays stuck on the
        // "Connecting" display state set when Start was clicked.
        emit stateChanged(ChannelState::Closed);
        return;
    }

    emit stateChanged(ChannelState::Open);
}

void ServerChannelWorker::closeClient(int clientId)
{
    auto* channel = serverHandle_->clientChannel(clientId);
    if (channel) {
        // Closing the channel fires its Closed state; TcpServerHandle's
        // passive-loss subscription then removes the client and emits
        // clientDisconnected exactly once. Do NOT emit here as well —
        // that would double-report the disconnect.
        channel->close();
    }
}

void ServerChannelWorker::closeAllClients()
{
    serverHandle_->stop();
    emit stateChanged(ChannelState::Closed);
}

void ServerChannelWorker::writeToClient(int clientId, const QByteArray& data)
{
    auto* channel = serverHandle_->clientChannel(clientId);
    if (!channel) {
        emit channelErrorOccurred(
            QStringLiteral("TCP Server"),
            io::ChannelErrorCode::Unknown,
            QStringLiteral("Client not found"));
        return;
    }
    if (!channel->isOpen()) {
        emit channelErrorOccurred(
            QStringLiteral("TCP Server"),
            io::ChannelErrorCode::Unknown,
            QStringLiteral("Client channel not open"));
        return;
    }
    if (!channel->write(data)) {
        emit channelErrorOccurred(
            QStringLiteral("TCP Server"),
            io::ChannelErrorCode::WriteFailed,
            QStringLiteral("Write to client failed"));
        return;
    }
    emit monitorWithClient(true, data, clientId);
}

} // namespace io