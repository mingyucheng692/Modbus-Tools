#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>
#include "IChannel.h"
#include "SerialChannel.h" // For SerialConfig

namespace io {

class GenericIoWorker : public QObject {
    Q_OBJECT

public:
    explicit GenericIoWorker(QObject* parent = nullptr);
    ~GenericIoWorker() override;

public slots:
    void openTcp(const QString& ip, int port);
    void openSerial(const SerialConfig& config);
    void close();
    void write(const QByteArray& data);
    
    // Serial Control
    void setDtr(bool set);
    void setRts(bool set);

signals:
    void stateChanged(int state); // Maps to ChannelState
    void errorOccurred(const QString& error);
    void monitor(bool isTx, const QByteArray& data);
    void bytesWritten(qint64 bytes);

private:
    void setupChannel();
    void cleanupChannel();

    std::shared_ptr<IChannel> channel_;
};

} // namespace io
