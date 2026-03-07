#pragma once

#include "../session/IModbusClient.h"
#include <memory>
#include <QObject>
#include <QPointer>
#include <QThread>

namespace modbus::dispatch {

class ModbusWorker : public QObject {
    Q_OBJECT
public:
    explicit ModbusWorker(std::shared_ptr<session::IModbusClient> client, QThread* workerThread, QObject* parent = nullptr);
    ~ModbusWorker() override;

    void start();
    void stop();

    void submit(const base::Pdu& request, int slaveId, int requestId);
    void sendRaw(const QByteArray& data);
    void requestConnect();
    void requestDisconnect();

signals:
    void requestFinished(int requestId, modbus::session::ModbusResponse response);
    void connectFinished(bool ok, const QString& error);
    void disconnectFinished();

private:
    void handleSubmit(base::Pdu request, int slaveId, int requestId);
    void handleSendRaw(QByteArray data);
    void handleConnect();
    void handleDisconnect();

    std::shared_ptr<session::IModbusClient> client_;
    QPointer<QThread> thread_;
    bool stopping_ = false;
};

} // namespace modbus::dispatch
