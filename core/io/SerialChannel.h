#pragma once
#include "ChannelBase.h"
#include <QSerialPort>

namespace io {

struct SerialConfig {
    QString portName;
    qint32 baudRate = 9600;
    int dataBits = 8;
    int stopBits = 1;
    QSerialPort::Parity parity = QSerialPort::NoParity;
};

class SerialChannel : public ChannelBase {
public:
    SerialChannel();
    ~SerialChannel() override;

    ChannelKind kind() const override { return ChannelKind::Serial; }
    bool open() override;
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
