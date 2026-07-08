/**
 * @file BufferedWritingChannel.cpp
 * @brief Implementation of BufferedWritingChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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
    emitMonitor(true, dataBuffer);
    armWriteTimeout();
    flushPendingWrites();
    return true;
}

void BufferedWritingChannel::flushPendingWrites() {
    QIODevice* dev = device();
    if (QThread::currentThread() != dev->thread()) {
        QMetaObject::invokeMethod(dev, [this]() {
            flushPendingWrites();
        }, Qt::QueuedConnection);
        return;
    }

    if (!isWritable() || pendingWrites_.empty()) {
        if (pendingWrites_.empty() && dev->bytesToWrite() == 0) {
            disarmWriteTimeout();
        }
        return;
    }

    auto& frame = pendingWrites_.front();
    while (currentWriteOffset_ < frame.size()) {
        const char* dataPtr = frame.constData() + currentWriteOffset_;
        const qint64 remaining = static_cast<qint64>(frame.size() - currentWriteOffset_);
        const qint64 accepted = dev->write(dataPtr, remaining);
        if (accepted < 0) {
            const QString err = dev->errorString().isEmpty()
                ? QStringLiteral("%1 write failed").arg(errorPrefix())
                : dev->errorString();
            spdlog::error("{}: write failed {} error={}",
                          errorPrefix().toStdString(),
                          logContext().toStdString(),
                          err.toStdString());
            resetWriteState();
            disarmWriteTimeout();
            setState(ChannelState::Error);
            emitError(err);
            return;
        }
        if (accepted == 0) {
            break;
        }
        currentWriteOffset_ += static_cast<qsizetype>(accepted);
        armWriteTimeout();
    }
}

void BufferedWritingChannel::onBytesWritten(qint64 bytes) {
    Q_UNUSED(bytes);
    QIODevice* dev = device();
    bool queueDrained = false;
    while (!pendingWrites_.empty() &&
           currentWriteOffset_ >= pendingWrites_.front().size() &&
           dev->bytesToWrite() == 0) {
        addTx(pendingWrites_.front().size());
        pendingWrites_.pop_front();
        currentWriteOffset_ = 0;
        queueDrained = pendingWrites_.empty();
    }
    if (queueDrained && dev->bytesToWrite() == 0) {
        disarmWriteTimeout();
        emitWriteDrained();
    } else if (!pendingWrites_.empty() || dev->bytesToWrite() > 0) {
        armWriteTimeout();
    }
    flushPendingWrites();
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
    spdlog::warn("{}: write timeout {} draining={} pendingWrites={}",
                 errorPrefix().toStdString(),
                 logContext().toStdString(),
                 draining,
                 pendingWrites_.size());
    resetWriteState();
    disarmWriteTimeout();
    setState(ChannelState::Error);
    emitError(draining
        ? QStringLiteral("%1 write drain timeout").arg(errorPrefix())
        : QStringLiteral("%1 write timeout").arg(errorPrefix()));
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
    MODBUS_TOOLS_VERBOSE_INFO("{} current={} owner={}",
                              scope,
                              threadToken(currentThread),
                              threadToken(ownerThread));
    MODBUS_TOOLS_VERBOSE_INFO("{} ui_thread={} current_is_ui={} owner_is_ui={}",
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
