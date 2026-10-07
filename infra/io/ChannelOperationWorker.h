// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>
#include "IChannel.h"
#include "SerialConfig.h"

namespace io {

// Role: Single-channel I/O worker managing serial/TCP/UDP channel instances on dedicated thread.
// Thread-safety: Thread-compatible. Moved to dedicated QThread; interacted via queued connections.
// Ownership: Owned by QObject parent hierarchy (typically ChannelController).
class ChannelOperationWorker : public QObject {
    Q_OBJECT

public:
    explicit ChannelOperationWorker(QObject* parent = nullptr);
    ~ChannelOperationWorker() noexcept override;

public slots:
    void openTcp(const QString& ip, int port, quint64 generation);
    // generation: Monotonic UI-side connection attempt counter; stale
    // stateChangedWithGeneration emissions from a torn-down serial channel are dropped by the view layer.
    void openSerial(const SerialConfig& config, quint64 generation);
    void openUdp(const QString& localIp, int localPort,
                 const QString& remoteIp = {}, int remotePort = 0);
    void close();
    void write(const QByteArray& data);

    // Serial Control
    void setDtr(bool set);
    void setRts(bool set);

signals:
    void stateChanged(ChannelState state);
    void stateChangedWithGeneration(ChannelState state, quint64 generation);
    void channelErrorOccurred(const QString& deviceHint, io::ChannelErrorCode code, const QString& error);
    void monitor(bool isTx, const QByteArray& data);
    void bytesQueued(qint64 bytes);

private:
    void setupChannel();
    void cleanupChannel();
    void emitError(io::ChannelErrorCode code, const QString& error);

    std::shared_ptr<IChannel> channel_;
    quint64 channelGeneration_ = 0;
    IChannel::HandlerId stateHandlerId_ = 0;
    QString deviceHint_;
};

} // namespace io
