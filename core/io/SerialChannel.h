#pragma once
#include "AppConstants.h"
#include "ChannelBase.h"
#include <QSerialPort>

namespace io {

struct SerialConfig {
    QString portName;
    qint32 baudRate = app::constants::Constants::Serial::kDefaultBaudRate;
    int dataBits = app::constants::Constants::Serial::kDefaultDataBits;
    int stopBits = app::constants::Constants::Serial::kDefaultStopBits;
    QSerialPort::Parity parity = QSerialPort::NoParity;
};

class SerialChannel : public ChannelBase {
public:
    SerialChannel();
    ~SerialChannel() override;

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
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

    QSerialPort serial_;
    SerialConfig config_;
};

}
