#pragma once
#include "IChannel.h"
#include <QSerialPort>
#include <QTimer>

class SerialChannel : public IChannel {
    Q_OBJECT
public:
    explicit SerialChannel(QObject* parent = nullptr);
    ~SerialChannel() override;

    void setConnectionSettings(const QString& portName, int baudRate);

    void open() override;
    void close() override;
    void write(const std::vector<uint8_t>& data) override;
    void write(const char* data, size_t size) override;

    ChannelState state() const override;
    QString errorString() const override;

private slots:
    void onSerialReadyRead();
    void onSilenceTimeout();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    QSerialPort* serial_;
    QTimer* silenceTimer_;
    std::vector<uint8_t> rxBuffer_;
    ChannelState state_ = ChannelState::Closed;
    
    QString portName_ = "COM1";
    int baudRate_ = 9600;
};
