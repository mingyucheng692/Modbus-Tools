/**
 * @file ChannelOperationWorker.h
 * @brief Header file for ChannelOperationWorker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>
#include "IChannel.h"
#include "SerialConfig.h"

namespace io {

/**
 * @brief Single-channel I/O worker.
 *
 * Responsibility boundary:
 *   - Manages exactly ONE IChannel at a time (TCP client, serial, or UDP).
 *   - All open*() methods implicitly close the previous channel.
 *   - Serial control signals (DTR / RTS) are forwarded to the underlying
 *     SerialChannel if active; silently ignored for other channel kinds.
 *
 * @thread This worker must be moved to a dedicated IO thread via
 *         moveToThread(). All slot invocations happen on that thread.
 *         Signals are emitted to the GUI thread via Qt::AutoConnection
 *         (no Qt::DirectConnection allowed).
 *
 * Public slots (7 total):
 *   openTcp, openSerial, openUdp, close, write, setDtr, setRts.
 *
 * NOT in scope:
 *   - Multi-client management (see ServerChannelWorker).
 *   - Modbus protocol parsing or transaction logic.
 *   - Connection retry or keep-alive (handled by View layer).
 */
class ChannelOperationWorker : public QObject {
    Q_OBJECT

public:
    explicit ChannelOperationWorker(QObject* parent = nullptr);
    ~ChannelOperationWorker() noexcept override;

public slots:
    void openTcp(const QString& ip, int port, quint64 generation);
    void openSerial(const SerialConfig& config);
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
    void channelErrorOccurred(const QString& deviceHint, const QString& error);
    void monitor(bool isTx, const QByteArray& data);
    void bytesQueued(qint64 bytes);

private:
    void setupChannel();
    void cleanupChannel();
    void emitError(const QString& error);

    std::shared_ptr<IChannel> channel_;
    quint64 channelGeneration_ = 0;
    IChannel::HandlerId stateHandlerId_ = 0;
    QString deviceHint_;
};

} // namespace io
