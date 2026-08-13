/**
 * @file ModbusWorker.cpp
 * @brief Implementation of ModbusWorker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusWorker.h"
#include "../session/ModbusClient.h"
#include "../TraceContext.h"
#include <spdlog/spdlog.h>
#include <QMetaObject>
#include <QThread>

namespace modbus::dispatch {

bool isCleanSuccess(const session::ModbusResponse& response) {
    return !response.isError() && response.retryCount() == 0;
}

namespace {
bool isThreadReady(const QPointer<QThread>& thread) {
    return thread && thread->isRunning();
}
}

ModbusWorker::ModbusWorker(std::shared_ptr<session::ModbusClient> client, QThread* workerThread, QObject* parent)
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

void ModbusWorker::submit(const base::Pdu& request, int slaveId, int requestId, quint64 traceId) {
    if (!isThreadReady(thread_)) {
        spdlog::warn("ModbusWorker: reject request trace_id={} because worker thread is not running",
                     static_cast<unsigned long long>(traceId));
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker thread not running"));
        return;
    }
    QMetaObject::invokeMethod(this, [this, request, slaveId, requestId, traceId]() {
        // Check stopped_ first — handleStopInThread sets stopped_=true and
        // clears stopping_, so a post-stop submit would otherwise slip through.
        if (stopped_.load()) {
            spdlog::warn("ModbusWorker: reject request trace_id={} because worker has stopped",
                         static_cast<unsigned long long>(traceId));
            emit requestFinished(requestId, session::ModbusResponse::Error("Worker has stopped"));
            return;
        }
        if (stopping_.load()) {
            spdlog::warn("ModbusWorker: reject request trace_id={} because worker is stopping",
                         static_cast<unsigned long long>(traceId));
            emit requestFinished(requestId, session::ModbusResponse::Error("Worker is stopping"));
            return;
        }
        spdlog::info("ModbusWorker: enqueue request trace_id={} request_id={} slave={}",
                     static_cast<unsigned long long>(traceId), requestId, slaveId);
        queuedRequests_.push_back(QueuedRequest{std::move(request), slaveId, requestId, traceId});
        // Draining runs inline on the worker thread. processQueue() re-arms
        // itself via QueuedConnection so stop()/sendRaw()/etc. can interleave
        // between items; no separate scheduleProcessQueue() hop is needed.
        processQueue();
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

void ModbusWorker::handleSubmit(base::Pdu request, int slaveId, int requestId, quint64 traceId) {
    // Thread affinity guard: the thread_local TraceContext below (and the
    // request serialization it feeds) is only correct when this handler runs
    // on the worker thread. QueuedConnection guarantees that today; the
    // assert catches future regressions in debug builds.
    Q_ASSERT(!thread_ || QThread::currentThread() == thread_);
    // Publish the trace id for the duration of this request so
    // RequestExecutor / state-machine log sites can correlate with the
    // UI-visible TrafficEvent.traceId.
    trace::Scope traceScope(traceId);
    if (stopping_.load()) {
        spdlog::warn("ModbusWorker: fail request trace_id={} because worker is stopping",
                     static_cast<unsigned long long>(traceId));
        emit requestFinished(requestId, session::ModbusResponse::Error("Worker is stopping"));
        return;
    }
    if (!client_) {
        spdlog::warn("ModbusWorker: fail request trace_id={} because no client is attached",
                     static_cast<unsigned long long>(traceId));
        emit requestFinished(requestId, session::ModbusResponse::Error("No client attached"));
        return;
    }
    if (!client_->isConnected()) {
        spdlog::warn("ModbusWorker: fail request trace_id={} because client is not connected",
                     static_cast<unsigned long long>(traceId));
        emit requestFinished(requestId, session::ModbusResponse::Error("Not connected"));
        return;
    }
    auto response = client_->sendRequest(request, slaveId);
    // Clean successes (no error, no retry) are the steady-state majority of
    // log lines under polling; demote them to debug so production logs keep
    // signal density. Failures and retried requests stay at info.
    if (isCleanSuccess(response)) {
        spdlog::debug("ModbusWorker: complete request trace_id={} request_id={} success=true",
                      static_cast<unsigned long long>(traceId),
                      requestId);
    } else {
        spdlog::info("ModbusWorker: complete request trace_id={} request_id={} success={} retries={} error='{}'",
                     static_cast<unsigned long long>(traceId),
                     requestId,
                     !response.isError(),
                     response.retryCount(),
                     response.error.toStdString());
    }
    emit requestFinished(requestId, response);
}

void ModbusWorker::drainQueuedRequests(const QString& reason) {
    while (!queuedRequests_.empty()) {
        auto item = queuedRequests_.front();
        queuedRequests_.pop_front();
        emit requestFinished(item.requestId, session::ModbusResponse::Error(reason));
    }
}

void ModbusWorker::processQueue() {
    // All access to queuedRequests_ is marshalled onto the worker thread via
    // QueuedConnection (submit() / the re-arm below), so no atomic flag is
    // needed here. Only one item is consumed per pass to keep the event loop
    // responsive between requests (stop()/sendRaw()/updateConfig() can run).
    if (stopping_.load() || queuedRequests_.empty()) {
        return;
    }
    auto item = queuedRequests_.front();
    queuedRequests_.pop_front();
    handleSubmit(item.request, item.slaveId, item.requestId, item.traceId);
    if (!queuedRequests_.empty() && !stopping_.load()) {
        QMetaObject::invokeMethod(this, [this]() {
            processQueue();
        }, Qt::QueuedConnection);
    }
}

void ModbusWorker::handleSendRaw(QByteArray data) {
    if (!client_) {
        return;
    }
    if (!client_->isConnected()) {
        spdlog::warn("ModbusWorker: sendRaw skipped — not connected");
        return;
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
    stopping_.store(false);
    emit stopped();
}

void ModbusWorker::handleUpdateConfig(base::ModbusConfig config) {
    if (client_) {
        client_->setConfig(config);
    }
}

} // namespace modbus::dispatch
