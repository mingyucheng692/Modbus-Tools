/**
 * @file BufferedWritingChannel.h
 * @brief Base class for channels with a pending-write queue and write-timeout.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ChannelBase.h"
#include <QTimer>
#include <deque>

class QIODevice;

namespace io {

/**
 * @brief Common write-queue + write-timeout machinery for Serial/Tcp channels.
 *
 * Owns the pending-write buffer, the write-offset cursor, the write-timeout
 * timer, and the thread-context log flags. Subclasses provide the underlying
 * QIODevice (serial port / TCP socket) and device-specific open/close/configure
 * logic via pure-virtual hooks.
 *
 * Thread-affinity contract: all write-queue methods (flushPendingWrites,
 * onBytesWritten, onWriteTimeout) must run on the device's owner thread
 * (device()->thread()). The base write() marshals cross-thread calls onto
 * that thread via Qt::QueuedConnection.
 */
class BufferedWritingChannel : public ChannelBase {
public:
    ~BufferedWritingChannel() noexcept override;

    bool write(QByteArrayView data) override;

protected:
    BufferedWritingChannel();

    // --- Subclass hooks ---
    /// Returns the underlying device (serial port / socket). Must not be null.
    virtual QIODevice* device() = 0;
    /// True when the device can accept writes right now (e.g. open/connected).
    virtual bool isWritable() const = 0;
    /// Context string for spdlog, e.g. "port=COM3" or "endpoint=1.2.3.4:502".
    virtual QString logContext() const = 0;
    /// Prefix for user-facing error messages, e.g. "Serial" or "TCP".
    virtual QString errorPrefix() const = 0;

    // --- Common write-queue machinery (callable by subclasses) ---
    void flushPendingWrites();
    void onBytesWritten(qint64 bytes);
    void onWriteTimeout();
    void armWriteTimeout();
    void disarmWriteTimeout();
    void resetWriteState();
    void logThreadContextOnce(const char* scope, bool& loggedFlag);

    /// Moves the write-timeout timer to @p thread and clears thread-log flags.
    /// Subclasses must call this from their moveToThread() after moving the
    /// device, so the timer and device share the same thread affinity.
    void moveWriteInfrastructureToThread(QThread* thread);

    // --- State accessors for subclasses ---
    bool isClosing() const { return closing_; }
    void setClosing(bool v) { closing_ = v; }
    bool& openThreadLoggedFlag() { return openThreadLogged_; }
    bool& ioThreadLoggedFlag() { return ioThreadLogged_; }

private:
    QTimer writeTimeoutTimer_;
    std::deque<QByteArray> pendingWrites_;
    qsizetype currentWriteOffset_ = 0;
    bool closing_ = false;
    bool openThreadLogged_ = false;
    bool ioThreadLogged_ = false;
};

} // namespace io
