/**
 * @file ServerChannelWorker.h
 * @brief Header file for ServerChannelWorker.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include "IChannel.h"

namespace io {

/**
 * @brief Multi-channel I/O worker for TCP server mode.
 *
 * Responsibility boundary:
 *   - Manages a TCP server that accepts multiple concurrent client
 *     connections.
 *   - Routes incoming data per-client so the UI can distinguish
 *     traffic from different peers.
 *   - Provides per-client write and disconnect operations.
 *
 * Public slots:
 *   openTcpServer, closeClient, closeAllClients, writeToClient.
 *
 * NOT in scope:
 *   - Single-channel operations (see ChannelOperationWorker).
 *   - UDP or serial communication.
 *   - Modbus protocol logic.
 */
class ServerChannelWorker : public QObject {
    Q_OBJECT

public:
    explicit ServerChannelWorker(QObject* parent = nullptr);
    ~ServerChannelWorker() override;

public slots:
    void openTcpServer(const QString& listenIp, int port, int maxClients);
    void closeClient(int clientId);
    void closeAllClients();
    void writeToClient(int clientId, const QByteArray& data);

signals:
    void monitorWithClient(bool isTx, const QByteArray& data, int clientId);
    void clientConnected(int clientId, const QString& peerInfo);
    void clientDisconnected(int clientId);
    void stateChanged(ChannelState state);
    void channelErrorOccurred(const QString& deviceHint, const QString& error);
};

} // namespace io