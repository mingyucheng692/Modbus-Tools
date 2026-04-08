#include "ChannelOperationWorker.h"
#include "AppConstants.h"
#include "TcpChannel.h"
#include "SerialChannel.h"
#include <QFileInfo>
#include <QThread>
#include <QMetaObject>
#include <QMetaType>

namespace io {

ChannelOperationWorker::ChannelOperationWorker(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<io::ChannelState>("io::ChannelState");
}

ChannelOperationWorker::~ChannelOperationWorker()
{
    cleanupChannel();
}

void ChannelOperationWorker::openTcp(const QString& ip, int port, quint64 generation)
{
    cleanupChannel();
    channelGeneration_ = generation;

    auto tcp = std::make_shared<TcpChannel>();
    tcp->setEndpoint(ip, port);
    channel_ = tcp;

    setupChannel();

    if (!channel_->open()) {
        // If open failed synchronously
        emit channelErrorOccurred("Failed to open TCP connection");
        // State changes are still forwarded through the subscribed channel handler.
    }
}

void ChannelOperationWorker::openSerial(const SerialConfig& config)
{
    cleanupChannel();

    auto serial = std::make_shared<SerialChannel>();
    serial->setConfig(config);
    channel_ = serial;

    setupChannel();

    if (!channel_->open()) {
        emit channelErrorOccurred("Failed to open serial port");
    }
}

void ChannelOperationWorker::close()
{
    cancelFileTransfer();
    if (channel_) {
        channel_->close();
    }
}

void ChannelOperationWorker::write(const QByteArray& data)
{
    if (transferInProgress_) {
        emit channelErrorOccurred("File transfer in progress");
        return;
    }
    if (channel_ && channel_->isOpen()) {
        if (channel_->write(data)) {
            emit bytesQueued(data.size());
        } else {
            emit channelErrorOccurred("Write failed");
        }
    } else {
        emit channelErrorOccurred("Channel not open");
    }
}

void ChannelOperationWorker::sendFile(const QString& filePath, int chunkSizeBytes)
{
    if (transferInProgress_) {
        failFileTransfer("File transfer already in progress");
        return;
    }

    if (!channel_ || !channel_->isOpen()) {
        failFileTransfer("Channel not open");
        return;
    }

    transferFilePath_ = QFileInfo(filePath).fileName().isEmpty() ? filePath : QFileInfo(filePath).fileName();
    transferFile_.setFileName(filePath);
    if (!transferFile_.open(QIODevice::ReadOnly)) {
        failFileTransfer("Cannot open file for transfer");
        return;
    }

    transferTotalBytes_ = transferFile_.size();
    transferSentBytes_ = 0;
    transferInFlightBytes_ = 0;
    transferChunkSizeBytes_ = qMax(1, chunkSizeBytes);
    transferAwaitingDrain_ = false;
    transferInProgress_ = true;
    emit fileTransferStarted(transferFilePath_, transferTotalBytes_);
    emit fileTransferProgress(transferFilePath_, 0, transferTotalBytes_);
    QMetaObject::invokeMethod(this, [this]() {
        sendNextFileChunk();
    }, Qt::QueuedConnection);
}

void ChannelOperationWorker::setDtr(bool set)
{
    if (auto serial = std::dynamic_pointer_cast<SerialChannel>(channel_)) {
        serial->setDtr(set);
    }
}

void ChannelOperationWorker::setRts(bool set)
{
    if (auto serial = std::dynamic_pointer_cast<SerialChannel>(channel_)) {
        serial->setRts(set);
    }
}

void ChannelOperationWorker::setupChannel()
{
    if (!channel_) return;

    channel_->setErrorHandler([this](const QString& err) {
        if (transferInProgress_) {
            failFileTransfer(err.isEmpty() ? QStringLiteral("Channel error") : err);
            return;
        }
        emit channelErrorOccurred(err);
    });

    channel_->setMonitor([this](bool isTx, const QByteArray& data) {
        emit monitor(isTx, data);
    });

    channel_->setWriteDrainedHandler([this]() {
        if (transferInProgress_ && transferAwaitingDrain_) {
            transferSentBytes_ += transferInFlightBytes_;
            emit bytesDrained(transferInFlightBytes_);
            emit bytesWritten(transferInFlightBytes_);
            transferInFlightBytes_ = 0;
            transferAwaitingDrain_ = false;
            emit fileTransferProgress(transferFilePath_, transferSentBytes_, transferTotalBytes_);
            sendNextFileChunk();
        }
    });

    stateHandlerId_ = channel_->addStateHandler([this](ChannelState state) {
        if (transferInProgress_ && (state == ChannelState::Error || state == ChannelState::Closed)) {
            failFileTransfer(state == ChannelState::Error
                                 ? QStringLiteral("Channel entered error state during file transfer")
                                 : QStringLiteral("Channel closed during file transfer"));
        }
        emit stateChanged(state);
        emit stateChangedWithGeneration(state, channelGeneration_);
    });
}

void ChannelOperationWorker::cleanupChannel()
{
    cancelFileTransfer();
    if (channel_) {
        channel_->close();
        channel_->setReadHandler(nullptr);
        channel_->setErrorHandler(nullptr);
        channel_->setWriteDrainedHandler(nullptr);
        channel_->setMonitor(nullptr);
        channel_->removeStateHandler(stateHandlerId_);
        stateHandlerId_ = 0;
        channel_.reset();
    }
}

void ChannelOperationWorker::sendNextFileChunk()
{
    if (!transferInProgress_ || transferAwaitingDrain_) {
        return;
    }

    if (!channel_ || !channel_->isOpen()) {
        failFileTransfer("Channel not open");
        return;
    }

    const QByteArray chunk = transferFile_.read(transferChunkSizeBytes_);
    if (chunk.isEmpty()) {
        if (!transferFile_.atEnd() && transferFile_.error() != QFileDevice::NoError) {
            failFileTransfer("Read file chunk failed");
            return;
        }
        finishFileTransferSuccess();
        return;
    }

    if (!channel_->write(chunk)) {
        failFileTransfer("Write failed");
        return;
    }

    transferInFlightBytes_ = chunk.size();
    transferAwaitingDrain_ = true;
    emit bytesQueued(chunk.size());
}

void ChannelOperationWorker::finishFileTransferSuccess()
{
    if (!transferInProgress_) {
        return;
    }
    emit fileTransferProgress(transferFilePath_, transferTotalBytes_, transferTotalBytes_);
    emit fileTransferFinished(transferFilePath_);
    resetFileTransferState();
}

void ChannelOperationWorker::failFileTransfer(const QString& error)
{
    if (!error.isEmpty()) {
        emit channelErrorOccurred(error);
    }
    if (transferInProgress_ || !transferFilePath_.isEmpty()) {
        emit fileTransferFailed(transferFilePath_, error);
    }
    resetFileTransferState();
}

void ChannelOperationWorker::cancelFileTransfer()
{
    if (transferInProgress_) {
        emit fileTransferCanceled(transferFilePath_);
    }
    resetFileTransferState();
}

void ChannelOperationWorker::resetFileTransferState()
{
    transferInProgress_ = false;
    transferAwaitingDrain_ = false;
    transferInFlightBytes_ = 0;
    transferTotalBytes_ = 0;
    transferSentBytes_ = 0;
    transferChunkSizeBytes_ = 0;
    if (transferFile_.isOpen()) {
        transferFile_.close();
    }
    transferFile_.setFileName(QString());
    transferFilePath_.clear();
}

} // namespace io
