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
#include <QFile>
#include <memory>
#include "IChannel.h"
#include "SerialChannel.h" // For SerialConfig

namespace io {

/**
 * @brief Single-channel I/O worker.
 *
 * Responsibility boundary:
 *   - Manages exactly ONE IChannel at a time (TCP client, serial, or UDP).
 *   - All open*() methods implicitly close the previous channel.
 *   - File transfer is handled as a sequence of chunked writes with
 *     backpressure via write-drained notifications.
 *   - Serial control signals (DTR / RTS) are forwarded to the underlying
 *     SerialChannel if active; silently ignored for other channel kinds.
 *
 * Public slots (8 total):
 *   openTcp, openSerial, openUdp, close, write, sendFile, setDtr, setRts.
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
    void sendFile(const QString& filePath, int chunkSizeBytes);
    
    // Serial Control
    void setDtr(bool set);
    void setRts(bool set);

signals:
    void stateChanged(ChannelState state);
    void stateChangedWithGeneration(ChannelState state, quint64 generation);
    void channelErrorOccurred(const QString& deviceHint, const QString& error);
    void monitor(bool isTx, const QByteArray& data);
    void bytesQueued(qint64 bytes);
    void bytesDrained(qint64 bytes);
    void bytesWritten(qint64 bytes);
    void fileTransferStarted(const QString& filePath, qint64 totalBytes);
    void fileTransferProgress(const QString& filePath, qint64 sentBytes, qint64 totalBytes);
    void fileTransferFinished(const QString& filePath);
    void fileTransferFailed(const QString& filePath, const QString& error);
    void fileTransferCanceled(const QString& filePath);

private:
    void setupChannel();
    void cleanupChannel();
    void sendNextFileChunk();
    void finishFileTransferSuccess();
    void failFileTransfer(const QString& error);
    void cancelFileTransfer();
    void resetFileTransferState();
    void emitError(const QString& error);

    std::shared_ptr<IChannel> channel_;
    quint64 channelGeneration_ = 0;
    IChannel::HandlerId stateHandlerId_ = 0;
    QString deviceHint_;
    QFile transferFile_;
    QString transferFilePath_;
    qint64 transferTotalBytes_ = 0;
    qint64 transferSentBytes_ = 0;
    qint64 transferInFlightBytes_ = 0;
    int transferChunkSizeBytes_ = 0;
    bool transferAwaitingDrain_ = false;
    bool transferInProgress_ = false;
};

} // namespace io
