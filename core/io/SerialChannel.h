/**
 * @file SerialChannel.h
 * @brief Header file for SerialChannel.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once
#include "AppConstants.h"
#include "ChannelBase.h"
#include "SerialConfig.h"
#include <QSerialPort>
#include <QTimer>
#include <deque>

namespace io {

class SerialChannel : public ChannelBase {
public:
    SerialChannel();
    ~SerialChannel() noexcept override;

    ChannelKind kind() const override { return ChannelKind::Serial; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;
    bool write(QByteArrayView data) override;
    
    void setConfig(const SerialConfig& config);
    
    // Control signals
    void setDtr(bool set);
    void setRts(bool set);

private:
    void logThreadContextOnce(const char* scope, bool& loggedFlag);
    void flushPendingWrites();
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void onWriteTimeout();
    void armWriteTimeout();
    void disarmWriteTimeout();
    void resetWriteState();

    QSerialPort serial_;
    QTimer writeTimeoutTimer_;
    SerialConfig config_;
    std::deque<QByteArray> pendingWrites_;
    qsizetype currentWriteOffset_ = 0;
    bool closing_ = false;
    bool openThreadLogged_ = false;
    bool ioThreadLogged_ = false;
};

}
