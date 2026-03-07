#pragma once
#include "ChannelBase.h"
#include <QTcpSocket>
#include <QHostAddress>

namespace io {

class TcpChannel : public ChannelBase {
public:
    TcpChannel();
    ~TcpChannel() override;

    ChannelKind kind() const override { return ChannelKind::Tcp; }
    bool open() override;
    void moveToThread(QThread* thread) override;
    void close() override;
    bool write(QByteArrayView data) override;
    
    void setEndpoint(const QString& ip, int port);

private:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);

    QTcpSocket socket_;
    QString ip_;
    int port_ = 502;
};

}
