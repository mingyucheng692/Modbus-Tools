#pragma once
#include "IChannel.h"
#include <QTcpSocket>
#include <QHostAddress>

class TcpChannel : public IChannel {
    Q_OBJECT
public:
    explicit TcpChannel(QObject* parent = nullptr);
    ~TcpChannel() override;

    void setConnectionSettings(const QString& ip, uint16_t port);

    void open() override;
    void close() override;
    void write(const std::vector<uint8_t>& data) override;
    void write(const char* data, size_t size) override;

    ChannelState state() const override;
    QString errorString() const override;

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();

private:
    QTcpSocket* socket_;
    QString ip_;
    uint16_t port_ = 502;
    ChannelState state_ = ChannelState::Closed;
};
