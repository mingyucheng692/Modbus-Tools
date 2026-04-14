/**
 * @file ModbusWorker.cpp
 * @brief Implementation of ModbusWorker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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
    stopped_.store(false);
    stopping_.store(false);
    processing_.store(false);
    if (thread_ && !thread_->isRunning()) {
        thread_->start();
    }
}

void ModbusWorker::stop() {
    if (stopped_.load()) {
        return;
    }
    bool expectedStopping = false;
    if (!stopping_.compare_exchange_strong(expectedStopping, true)) {
        return;
    }
    spdlog::info("ModbusWorker: asynchronous stop requested");

    if (client_) {
        client_->abort();
    }

    if (!thread_) {
        handleStopInThread();
        return;
    }

    if (!thread_->isRunning()) {
        spdlog::info("ModbusWorker: stop requested with non-running thread");
        handleStopInThread();
        return;
    }

    if (QThread::currentThread() == thread_) {
        handleStopInThread();
        return;
    }

    QMetaObject::invokeMethod(this, [this]() {
        handleStopInThread();
    }, Qt::QueuedConnection);
}

void ModbusWorker::submit(const base::Pdu& request, int slaveId, int requestId) {
    if (!isThreadReady(thread_)) {
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker thread not running"));
        return;
    }
    QMetaObject::invokeMethod(this, [this, request, slaveId, requestId]() {
        if (stopping_.load()) {
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
    if (stopping_.load()) {
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker is stopping"));
        return;
    }
    if (!client_) {
        emit requestFinished(requestId, session::ModbusResponse::Error("No client attached"));
        return;
    }
    if (!client_->isConnected()) {
        if (!client_->connect()) {
            const QString reason = client_->lastChannelError().isEmpty()
                ? QStringLiteral("Failed to connect")
                : client_->lastChannelError();
            emit requestFinished(requestId, session::ModbusResponse::Error(reason));
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
    if (processing_.load() || queuedRequests_.empty() || stopping_.load()) {
        return;
    }
    QMetaObject::invokeMethod(this, [this]() {
        processQueue();
    }, Qt::QueuedConnection);
}

void ModbusWorker::processQueue() {
    if (stopping_.load()) {
        return;
    }
    bool expectedProcessing = false;
    if (!processing_.compare_exchange_strong(expectedProcessing, true)) {
        return;
    }
    if (queuedRequests_.empty()) {
        processing_.store(false);
        return;
    }
    auto item = queuedRequests_.front();
    queuedRequests_.pop_front();
    handleSubmit(item.request, item.slaveId, item.requestId);
    processing_.store(false);
    if (!queuedRequests_.empty() && !stopping_.load()) {
        scheduleProcessQueue();
    }
}

void ModbusWorker::handleSendRaw(QByteArray data) {
    if (!client_) {
        return;
    }
    if (!client_->isConnected()) {
        if (!client_->connect()) {
            const QString reason = client_->lastChannelError().isEmpty()
                ? QStringLiteral("Failed to connect")
                : client_->lastChannelError();
            spdlog::warn("ModbusWorker: sendRaw connect failed: {}", reason.toStdString());
            return;
        }
    }
    client_->sendRaw(data);
}

void ModbusWorker::handleConnect() {
    if (stopping_.load()) {
        emit connectFinished(false, "Worker is stopping");
        return;
    }
    if (!client_) {
        emit connectFinished(false, "No client attached");
        return;
    }
    spdlog::info("ModbusWorker: connect requested");
    const bool ok = client_->connect();
    if (ok) {
        spdlog::info("ModbusWorker: connect succeeded");
        emit connectFinished(true, QString());
        return;
    }
    const QString reason = client_->lastChannelError().isEmpty()
        ? QStringLiteral("Failed to connect")
        : client_->lastChannelError();
    spdlog::warn("ModbusWorker: connect failed: {}", reason.toStdString());
    emit connectFinished(false, reason);
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
    if (stopped_.exchange(true)) {
        stopping_.store(false);
        return;
    }
    if (client_) {
        client_->abort();
    }
    drainQueuedRequests("Worker stopped");
    handleDisconnect();
    processing_.store(false);
    stopping_.store(false);
    emit stopped();
}

void ModbusWorker::handleUpdateConfig(base::ModbusConfig config) {
    if (client_) {
        client_->setConfig(config);
    }
}

} // namespace modbus::dispatch
