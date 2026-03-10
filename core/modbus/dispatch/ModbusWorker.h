#pragma once

#include "../session/IModbusClient.h"
#include <memory>
#include <deque>
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
    struct QueuedRequest {
        base::Pdu request;
        int slaveId = -1;
        int requestId = -1;
    };

    void handleSubmit(base::Pdu request, int slaveId, int requestId);
    void handleSendRaw(QByteArray data);
    void handleConnect();
    void handleDisconnect();
    void handleRequestDisconnectInThread();
    void handleStopInThread();
    void drainQueuedRequests(const QString& reason);
    void scheduleProcessQueue();
    void processQueue();

    std::shared_ptr<session::IModbusClient> client_;
    QPointer<QThread> thread_;
    bool stopping_ = false;
    bool processing_ = false;
    std::deque<QueuedRequest> queuedRequests_;
};

} // namespace modbus::dispatch
