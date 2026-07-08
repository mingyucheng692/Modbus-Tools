/**
 * @file SerialChannel.h
 * @brief Header file for SerialChannel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once
#include "core/Config.h"
#include "BufferedWritingChannel.h"
#include "SerialConfig.h"
#include <QSerialPort>

class QIODevice;

namespace io {

class SerialChannel : public BufferedWritingChannel {
public:
    SerialChannel();
    ~SerialChannel() noexcept override;

    ChannelKind kind() const override { return ChannelKind::Serial; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;

    void setConfig(const SerialConfig& config);

    // Control signals
    void setDtr(bool set);
    void setRts(bool set);

protected:
    QIODevice* device() override { return &serial_; }
    bool isWritable() const override { return serial_.isOpen(); }
    QString logContext() const override;
    QString errorPrefix() const override { return QStringLiteral("Serial"); }

private:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

    QSerialPort serial_;
    SerialConfig config_;
};

}
