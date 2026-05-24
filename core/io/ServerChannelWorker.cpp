/**
 * @file ServerChannelWorker.cpp
 * @brief Implementation of ServerChannelWorker.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ServerChannelWorker.h"

namespace io {

ServerChannelWorker::ServerChannelWorker(QObject* parent)
    : QObject(parent)
{
}

ServerChannelWorker::~ServerChannelWorker()
{
    closeAllClients();
}

void ServerChannelWorker::openTcpServer(const QString& listenIp, int port, int maxClients)
{
    Q_UNUSED(listenIp);
    Q_UNUSED(port);
    Q_UNUSED(maxClients);
    emit channelErrorOccurred(
        QStringLiteral("TCP Server"),
        QStringLiteral("TCP Server mode not yet implemented"));
}

void ServerChannelWorker::closeClient(int clientId)
{
    Q_UNUSED(clientId);
}

void ServerChannelWorker::closeAllClients()
{
}

void ServerChannelWorker::writeToClient(int clientId, const QByteArray& data)
{
    Q_UNUSED(clientId);
    Q_UNUSED(data);
    emit channelErrorOccurred(
        QStringLiteral("TCP Server"),
        QStringLiteral("Write to client not yet implemented"));
}

} // namespace io