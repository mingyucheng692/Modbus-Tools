#include "ModbusWorker.h"
#include <QMetaObject>
#include <QThread>

namespace modbus::dispatch {

ModbusWorker::ModbusWorker(std::shared_ptr<session::IModbusClient> client, QThread* workerThread, QObject* parent)
    : QObject(parent),
      client_(std::move(client)),
      thread_(workerThread) {
    qRegisterMetaType<modbus::session::ModbusResponse>("modbus::session::ModbusResponse");
    if (thread_) {
        moveToThread(thread_);
    }
}

ModbusWorker::~ModbusWorker() {
    stop();
}

void ModbusWorker::start() {
    if (thread_ && !thread_->isRunning()) {
        thread_->start();
    }
}

void ModbusWorker::stop() {
    if (stopping_) return;
    stopping_ = true;

    if (!thread_) {
        stopping_ = false;
        return;
    }

    if (QThread::currentThread() == thread_) {
        handleDisconnect();
        thread_->quit();
        stopping_ = false;
        return;
    }

    if (thread_->isRunning()) {
        QMetaObject::invokeMethod(this, [this]() { handleDisconnect(); }, Qt::BlockingQueuedConnection);
        thread_->quit();
        thread_->wait();
    }

    stopping_ = false;
}

void ModbusWorker::submit(const base::Pdu& request, int slaveId, int requestId) {
    if (!thread_) {
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker thread not available"));
        return;
    }
    QMetaObject::invokeMethod(this, [this, request, slaveId, requestId]() {
        handleSubmit(request, slaveId, requestId);
    }, Qt::QueuedConnection);
}

void ModbusWorker::sendRaw(const QByteArray& data) {
    if (!thread_) {
        emit requestFinished(-1, session::ModbusResponse::Error("Worker thread not available"));
        return;
    }
    QMetaObject::invokeMethod(this, [this, data]() {
        handleSendRaw(data);
    }, Qt::QueuedConnection);
}

void ModbusWorker::requestConnect() {
    if (!thread_) {
        emit connectFinished(false, "Worker thread not available");
        return;
    }
    QMetaObject::invokeMethod(this, [this]() {
        handleConnect();
    }, Qt::QueuedConnection);
}

void ModbusWorker::requestDisconnect() {
    if (!thread_) return;
    QMetaObject::invokeMethod(this, [this]() {
        handleDisconnect();
    }, Qt::QueuedConnection);
}

void ModbusWorker::handleSubmit(base::Pdu request, int slaveId, int requestId) {
    if (!client_) {
        emit requestFinished(requestId, session::ModbusResponse::Error("No client attached"));
        return;
    }
    if (!client_->isConnected()) {
        if (!client_->connect()) {
            emit requestFinished(requestId, session::ModbusResponse::Error("Failed to connect"));
            return;
        }
    }
    auto response = client_->sendRequest(request, slaveId);
    emit requestFinished(requestId, response);
}

void ModbusWorker::handleSendRaw(QByteArray data) {
    if (!client_) {
        return;
    }
    if (!client_->isConnected()) {
        if (!client_->connect()) {
            return;
        }
    }
    client_->sendRaw(data);
}

void ModbusWorker::handleConnect() {
    if (!client_) {
        emit connectFinished(false, "No client attached");
        return;
    }
    bool ok = client_->connect();
    emit connectFinished(ok, ok ? QString() : QString("Failed to connect"));
}

void ModbusWorker::handleDisconnect() {
    if (client_ && client_->isConnected()) {
        client_->disconnect();
    }
    emit disconnectFinished();
}

} // namespace modbus::dispatch
