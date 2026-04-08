#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <memory>
#include "IChannel.h"
#include "SerialChannel.h" // For SerialConfig

namespace io {

class GenericIoWorker : public QObject {
    Q_OBJECT

public:
    explicit GenericIoWorker(QObject* parent = nullptr);
    ~GenericIoWorker() override;

public slots:
    void openTcp(const QString& ip, int port, quint64 generation);
    void openSerial(const SerialConfig& config);
    void close();
    void write(const QByteArray& data);
    void sendFile(const QString& filePath, int chunkSizeBytes);
    
    // Serial Control
    void setDtr(bool set);
    void setRts(bool set);

signals:
    void stateChanged(ChannelState state);
    void stateChangedWithGeneration(ChannelState state, quint64 generation);
    void channelErrorOccurred(const QString& error);
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

    std::shared_ptr<IChannel> channel_;
    quint64 channelGeneration_ = 0;
    IChannel::HandlerId stateHandlerId_ = 0;
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
