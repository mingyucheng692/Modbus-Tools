// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "BufferedWritingChannel.h"
#include "infra/logging/Logger.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <QIODevice>
#include <QMetaObject>
#include <QThread>

namespace io {

namespace {

unsigned long long threadToken(QThread* thread)
{
    return static_cast<unsigned long long>(reinterpret_cast<quintptr>(thread));
}

} // namespace

BufferedWritingChannel::BufferedWritingChannel() {
    writeTimeoutTimer_.setSingleShot(true);
    // Functor-based connect with an explicit connection type requires a context
    // QObject (5-arg overload). ChannelBase itself is not a QObject, so the
    // timer is passed as the context. Because the timer lives on the IO thread
    // (after moveToThread), Qt::QueuedConnection guarantees the functor executes
    // on the IO thread regardless of which thread emits.
    QObject::connect(&writeTimeoutTimer_, &QTimer::timeout, &writeTimeoutTimer_, [this]() {
        onWriteTimeout();
    }, Qt::QueuedConnection);
}

BufferedWritingChannel::~BufferedWritingChannel() noexcept = default;

bool BufferedWritingChannel::write(QByteArrayView data) {
    QIODevice* dev = device();
    if (QThread::currentThread() != dev->thread()) {
        QThread* ownerThread = dev->thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        QByteArray dataCopy(data.data(), data.size());
        return QMetaObject::invokeMethod(dev, [this, dataCopy]() {
            write(dataCopy);
        }, Qt::QueuedConnection);
    }

    if (!isWritable()) {
        return false;
    }

    QByteArray dataBuffer(data.data(), data.size());
    if (dataBuffer.isEmpty()) {
        return true;
    }

    pendingWrites_.push_back(dataBuffer);
    // Anchor the write timeout to the oldest un-drained frame so continuous writes
    // cannot starve timeout detection on a stalled peer.
    if (!writeTimeoutTimer_.isActive()) {
        armWriteTimeout();
    }
    if (!flushPendingWrites()) {
        return false;
    }
    emitMonitor(true, dataBuffer);
    return true;
}

bool BufferedWritingChannel::flushPendingWrites() {
    QIODevice* dev = device();
    if (!dev) {
        return false;
    }
    if (QThread::currentThread() != dev->thread()) {
        QThread* ownerThread = dev->thread();
        if (!ownerThread || !ownerThread->isRunning()) {
            return false;
        }
        return QMetaObject::invokeMethod(dev, [this]() {
            flushPendingWrites();
        }, Qt::QueuedConnection);
    }

    if (!isWritable() || pendingWrites_.empty()) {
        if (pendingWrites_.empty() && dev->bytesToWrite() == 0) {
            disarmWriteTimeout();
        }
        return true;
    }

    while (!pendingWrites_.empty()) {
        const auto& frame = pendingWrites_.front();
        const qsizetype remainingSize = frame.size() - currentWriteOffset_;
        if (remainingSize <= 0) {
            pendingWrites_.pop_front();
            currentWriteOffset_ = 0;
            continue;
        }
        const char* dataPtr = frame.constData() + currentWriteOffset_;
        const qint64 accepted = dev->write(dataPtr, static_cast<qint64>(remainingSize));
        if (accepted < 0) {
            const QString err = dev->errorString().isEmpty()
                ? QStringLiteral("%1 write failed").arg(errorPrefix())
                : dev->errorString();
            SPDLOG_ERROR("{}: write failed {} error={}",
                          errorPrefix().toStdString(),
                          logContext().toStdString(),
                          err.toStdString());
            failWrite(ChannelErrorCode::WriteFailed, err);
            return false;
        }
        if (accepted == 0) {
            break;
        }
        currentWriteOffset_ += static_cast<qsizetype>(accepted);
        if (currentWriteOffset_ < frame.size()) {
            // Partial write: preserve order and wait for bytesWritten before retrying.
            break;
        }
        pendingWrites_.pop_front();
        currentWriteOffset_ = 0;
    }
    return true;
}

void BufferedWritingChannel::onBytesWritten(qint64 bytes) {
    if (closing_ || !isWritable() || state() == ChannelState::Error) {
        return;
    }
    if (bytes > 0) {
        addTx(static_cast<quint64>(bytes));
    }
    if (!flushPendingWrites() || state() == ChannelState::Error) {
        return;
    }
    QIODevice* dev = device();
    if (pendingWrites_.empty() && (!dev || dev->bytesToWrite() == 0)) {
        disarmWriteTimeout();
        emitWriteDrained();
    } else if (bytes > 0) {
        armWriteTimeout();
    }
}

void BufferedWritingChannel::onWriteTimeout() {
    QIODevice* dev = device();
    if (QThread::currentThread() != dev->thread()) {
        QMetaObject::invokeMethod(dev, [this]() {
            onWriteTimeout();
        }, Qt::QueuedConnection);
        return;
    }

    if (closing_ || !isWritable()) {
        disarmWriteTimeout();
        return;
    }

    if (pendingWrites_.empty() && dev->bytesToWrite() == 0) {
        disarmWriteTimeout();
        return;
    }

    const bool draining = dev->bytesToWrite() > 0;
    SPDLOG_WARN("{}: write timeout {} draining={} pendingWrites={}",
                 errorPrefix().toStdString(),
                 logContext().toStdString(),
                 draining,
                 pendingWrites_.size());
    failWrite(ChannelErrorCode::Timeout, draining
        ? QStringLiteral("%1 write drain timeout").arg(errorPrefix())
        : QStringLiteral("%1 write timeout").arg(errorPrefix()));
}

void BufferedWritingChannel::failWrite(ChannelErrorCode code, const QString& errorMessage) {
    resetWriteState();
    disarmWriteTimeout();
    setState(ChannelState::Error);
    onChannelError();
    emitError(code, errorMessage);
}

void BufferedWritingChannel::armWriteTimeout() {
    const int timeoutMs = timeouts().writeMs;
    if (timeoutMs <= 0) {
        return;
    }
    writeTimeoutTimer_.start(timeoutMs);
}

void BufferedWritingChannel::disarmWriteTimeout() {
    if (writeTimeoutTimer_.isActive()) {
        writeTimeoutTimer_.stop();
    }
}

void BufferedWritingChannel::resetWriteState() {
    pendingWrites_.clear();
    currentWriteOffset_ = 0;
}

void BufferedWritingChannel::logThreadContextOnce(const char* scope, bool& loggedFlag)
{
    if (loggedFlag) {
        return;
    }
    loggedFlag = true;
    QIODevice* dev = device();
    QThread* currentThread = QThread::currentThread();
    QThread* ownerThread = dev->thread();
    QThread* uiThread = QCoreApplication::instance() ? QCoreApplication::instance()->thread() : nullptr;
    SPDLOG_DEBUG("{} current={} owner={}",
                              scope,
                              threadToken(currentThread),
                              threadToken(ownerThread));
    SPDLOG_DEBUG("{} ui_thread={} current_is_ui={} owner_is_ui={}",
                              scope,
                              threadToken(uiThread),
                              currentThread == uiThread,
                              ownerThread == uiThread);
}

void BufferedWritingChannel::moveWriteInfrastructureToThread(QThread* thread) {
    writeTimeoutTimer_.moveToThread(thread);
    openThreadLogged_ = false;
    ioThreadLogged_ = false;
}

} // namespace io
