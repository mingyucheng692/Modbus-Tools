// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "../session/SessionTypes.h"
#include "common/LogDedupe.h"
#include <memory>
#include <deque>
#include <atomic>
#include <tuple>
#include <chrono>
#include <QObject>
#include <QPointer>
#include <QThread>

namespace modbus::session { class ModbusClient; }

namespace modbus::dispatch {

/// True when a completed request is a "clean success" (no error, no retry)
/// and its completion log should be demoted to debug level. Clean successes
/// are the steady-state majority of log lines under polling; failures and
/// retried requests keep info level to preserve signal density.
bool isCleanSuccess(const session::ModbusResponse& response);

// Role: Worker object serializing Modbus requests, raw frames, and lifecycle commands onto a dedicated thread.
// Thread-safety: Thread-safe via Qt QueuedConnection; slots run exclusively on worker's QThread.
// Ownership: Owned by ModbusStack / Presenter composition root via shared_ptr and QObject hierarchy.
class ModbusWorker : public QObject {
    Q_OBJECT
public:
    explicit ModbusWorker(std::shared_ptr<session::ModbusClient> client, QThread* workerThread, QObject* parent = nullptr);
    ~ModbusWorker() noexcept override;

    void start();
    void stop();

    void submit(const base::Pdu& request, int slaveId, int requestId, quint64 traceId = 0);
    void sendRaw(const QByteArray& data);
    void requestConnect();
    void requestDisconnect();
    void updateConfig(const base::ModbusConfig& config);

signals:
    void requestFinished(int requestId, modbus::session::ModbusResponse response);
    void connectFinished(bool ok, const QString& error);
    void disconnectFinished();
    void stopped();

private:
    struct QueuedRequest {
        base::Pdu request;
        int slaveId = -1;
        int requestId = -1;
        quint64 traceId = 0;
    };

    void handleSubmit(base::Pdu request, int slaveId, int requestId, quint64 traceId);
    void handleSendRaw(QByteArray data);
    void handleConnect();
    void handleDisconnect();
    void handleRequestDisconnectInThread();
    void handleStopInThread();
    void handleUpdateConfig(base::ModbusConfig config);
    void drainQueuedRequests(const QString& reason);
    void processQueue();

    std::shared_ptr<session::ModbusClient> client_;
    QPointer<QThread> thread_;
    // stopping_/stopped_ are set from any thread (e.g. the UI thread calling
    // stop()), so they remain atomic. processQueue()/submit() run exclusively
    // on the worker thread via QueuedConnection, so queuedRequests_ does not
    // need its own atomic guard.
    std::atomic_bool stopping_ {false};
    std::atomic_bool stopped_ {false};
    bool isProcessing_ {false};
    std::deque<QueuedRequest> queuedRequests_;

    using WorkerDedupeKey = std::tuple<uint8_t, uint8_t, uint8_t>; // (slaveId, fc, errorCode)
    ::common::LogDedupe<WorkerDedupeKey> failureDedupe_{std::chrono::seconds(5)};
};

} // namespace modbus::dispatch
