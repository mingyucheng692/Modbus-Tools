// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include "IChannel.h"

namespace io {

class TcpServerHandle;

// Role: Multi-client TCP server worker managing listener and adopted client channels.
// Thread-safety: Thread-compatible. Moved to dedicated QThread; interacted via queued connections.
// Ownership: Owned by QObject parent hierarchy (typically ChannelController); owns TcpServerHandle.
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
