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

class TcpServerHandle;

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
 * State contract (channel-level states emitted via stateChanged):
 *   - openTcpServer success: Closed -> Open.
 *   - openTcpServer failure: channelErrorOccurred + Closed (terminal — the
 *     view must fall back to its disconnected display, never stay on
 *     "Connecting").
 *   - closeAllClients: Closed (after every client channel and the listener
 *     have been torn down).
 *   Opening/Closing are never emitted; there is no intermediate listen state.
 *
 * Public slots:
 *   openTcpServer, closeClient, closeAllClients, writeToClient.
 *
 * @thread This worker must be moved to a dedicated server thread via
 *         moveToThread(); all slots execute on that thread. The worker and
 *         its TcpServerHandle (including every adopted client channel) share
 *         that single thread, so no cross-thread locking is required.
 *         Signals are emitted to the GUI thread via Qt::AutoConnection.
 *         Shutdown order: the owner queues closeAllClients() on the server
 *         thread (explicit teardown) BEFORE deleteLater(); relying on the
 *         destructor alone leaves the listener running until the thread
 *         drains its remaining events.
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
    ~ServerChannelWorker() noexcept override;

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
    void channelErrorOccurred(const QString& deviceHint, io::ChannelErrorCode code, const QString& error);

private:
    TcpServerHandle* serverHandle_ = nullptr;
};

} // namespace io
