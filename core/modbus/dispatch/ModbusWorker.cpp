#include "ModbusWorker.h"
#include <spdlog/spdlog.h>
#include <QMetaObject>
#include <QThread>

namespace modbus::dispatch {

namespace {
bool isThreadReady(const QPointer<QThread>& thread) {
    return thread && thread->isRunning();
}
}

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
    if (stopping_ || stopped_) return;
    stopping_ = true;
    spdlog::info("ModbusWorker: asynchronous stop requested");

    if (client_) {
        client_->abort();
    }

    if (!thread_) {
        handleStopInThread();
        stopped_ = true;
        stopping_ = false;
        emit stopped();
        return;
    }

    if (!thread_->isRunning()) {
        spdlog::info("ModbusWorker: stop requested with non-running thread");
        drainQueuedRequests("Worker stopped");
        stopped_ = true;
        stopping_ = false;
        emit stopped();
        return;
    }

    if (QThread::currentThread() == thread_) {
        handleStopInThread();
        thread_->quit();
        return;
    }

    QMetaObject::invokeMethod(this, [this]() {
        handleStopInThread();
        if (thread_) {
            thread_->quit();
        }
    }, Qt::QueuedConnection);

    thread_->requestInterruption();
    thread_->quit();
}

void ModbusWorker::submit(const base::Pdu& request, int slaveId, int requestId) {
    if (!isThreadReady(thread_)) {
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker thread not running"));
        return;
    }
    QMetaObject::invokeMethod(this, [this, request, slaveId, requestId]() {
        if (stopping_) {
            emit requestFinished(requestId, session::ModbusResponse::Error("Worker is stopping"));
            return;
        }
        queuedRequests_.push_back(QueuedRequest{request, slaveId, requestId});
        scheduleProcessQueue();
    }, Qt::QueuedConnection);
}

void ModbusWorker::sendRaw(const QByteArray& data) {
    if (!isThreadReady(thread_)) {
        emit requestFinished(-1, session::ModbusResponse::Error("Worker thread not running"));
        return;
    }
    QMetaObject::invokeMethod(this, [this, data]() {
        handleSendRaw(data);
    }, Qt::QueuedConnection);
}

void ModbusWorker::requestConnect() {
    if (!isThreadReady(thread_)) {
        emit connectFinished(false, "Worker thread not running");
        return;
    }
    QMetaObject::invokeMethod(this, [this]() {
        handleConnect();
    }, Qt::QueuedConnection);
}

void ModbusWorker::requestDisconnect() {
    if (!isThreadReady(thread_)) {
        handleRequestDisconnectInThread();
        return;
    }
    QMetaObject::invokeMethod(this, [this]() {
        handleRequestDisconnectInThread();
    }, Qt::QueuedConnection);
}

void ModbusWorker::updateConfig(const base::ModbusConfig& config) {
    if (!isThreadReady(thread_)) {
        handleUpdateConfig(config);
        return;
    }

    QMetaObject::invokeMethod(this, [this, config]() {
        handleUpdateConfig(config);
    }, Qt::QueuedConnection);
}

void ModbusWorker::handleSubmit(base::Pdu request, int slaveId, int requestId) {
    if (stopping_) {
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker is stopping"));
        return;
    }
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

void ModbusWorker::drainQueuedRequests(const QString& reason) {
    while (!queuedRequests_.empty()) {
        auto item = queuedRequests_.front();
        queuedRequests_.pop_front();
        emit requestFinished(item.requestId, session::ModbusResponse::Error(reason));
    }
}

void ModbusWorker::scheduleProcessQueue() {
    if (processing_ || queuedRequests_.empty() || stopping_) {
        return;
    }
    QMetaObject::invokeMethod(this, [this]() {
        processQueue();
    }, Qt::QueuedConnection);
}

void ModbusWorker::processQueue() {
    if (processing_ || stopping_) {
        return;
    }
    if (queuedRequests_.empty()) {
        return;
    }
    processing_ = true;
    auto item = queuedRequests_.front();
    queuedRequests_.pop_front();
    handleSubmit(item.request, item.slaveId, item.requestId);
    processing_ = false;
    if (!queuedRequests_.empty() && !stopping_) {
        scheduleProcessQueue();
    }
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
    if (stopping_) {
        emit connectFinished(false, "Worker is stopping");
        return;
    }
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

void ModbusWorker::handleRequestDisconnectInThread() {
    if (client_) {
        client_->abort();
    }
    drainQueuedRequests("Disconnected");
    handleDisconnect();
}

void ModbusWorker::handleStopInThread() {
    if (client_) {
        client_->abort();
    }
    drainQueuedRequests("Worker stopped");
    handleDisconnect();
    stopped_ = true;
    stopping_ = false;
    emit stopped();
}

void ModbusWorker::handleUpdateConfig(base::ModbusConfig config) {
    if (client_) {
        client_->setConfig(config);
    }
}

} // namespace modbus::dispatch
