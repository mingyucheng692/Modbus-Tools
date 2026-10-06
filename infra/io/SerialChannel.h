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

/**
 * @class SerialChannel
 * @brief Concrete serial communication channel wrapping QSerialPort.
 *
 * Role: Provides buffered, non-blocking serial communication for Modbus RTU/ASCII.
 * Thread-safety: Thread-compatible. All operations must run on its affinity thread.
 * Ownership: Manages private QSerialPort instance; owned by unique_ptr / shared_ptr.
 */
class SerialChannel : public BufferedWritingChannel {
public:
    SerialChannel();
    ~SerialChannel() noexcept override;

    ChannelKind kind() const override { return ChannelKind::Serial; }
    bool open() override;
    bool write(QByteArrayView data) override;
    void moveToThread(QThread* thread) override;
    void close() override;

    void setConfig(const SerialConfig& config);

    // Control signals
    void setDtr(bool set);
    void setRts(bool set);
    bool setSerialControl(SerialSignal signal, bool value) override;

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
