/**
 * @file ModbusWorker.h
 * @brief Header file for ModbusWorker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../session/SessionTypes.h"
#include <memory>
#include <deque>
#include <atomic>
#include <QObject>
#include <QPointer>
#include <QThread>

namespace modbus::session { class ModbusClient; }

namespace modbus::dispatch {

/**
 * @brief Worker object for submitting Modbus requests from a dedicated thread.
 *
 * @thread This worker must be moved to a dedicated QThread via moveToThread().
 *         All slot invocations (submit, sendRaw, requestConnect, etc.) are
 *         queued and executed on the worker thread. Results are signaled back
 *         to the caller via queued connections.
 *
 * @note The underlying ModbusClient is thread-safe; this worker serializes
 *       access through queued connections to avoid concurrent submit() calls.
 */
class ModbusWorker : public QObject {
    Q_OBJECT
public:
    explicit ModbusWorker(std::shared_ptr<session::ModbusClient> client, QThread* workerThread, QObject* parent = nullptr);
    ~ModbusWorker() noexcept override;

    void start();
    void stop();

    void submit(const base::Pdu& request, int slaveId, int requestId);
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
    };

    void handleSubmit(base::Pdu request, int slaveId, int requestId);
    void handleSendRaw(QByteArray data);
    void handleConnect();
    void handleDisconnect();
    void handleRequestDisconnectInThread();
    void handleStopInThread();
    void handleUpdateConfig(base::ModbusConfig config);
    void drainQueuedRequests(const QString& reason);
    void scheduleProcessQueue();
    void processQueue();

    std::shared_ptr<session::ModbusClient> client_;
    QPointer<QThread> thread_;
    std::atomic_bool stopping_ {false};
    std::atomic_bool stopped_ {false};
    std::atomic_bool processing_ {false};
    // True while a processQueue() invocation is pending on the worker thread.
    // Prevents redundant queued invokes when several submit() calls arrive
    // before the first processQueue() runs. All access happens on the worker
    // thread (submit/processQueue/scheduleProcessQueue are marshalled there),
    // so the atomic is defensive rather than strictly required.
    std::atomic_bool processQueued_ {false};
    std::deque<QueuedRequest> queuedRequests_;
};

} // namespace modbus::dispatch
