/**
 * @file TraceContext.h
 * @brief Thread-local trace identifier for the Modbus request being processed.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QtGlobal>

namespace modbus::trace {

/**
 * @brief Trace id of the request currently being processed on this thread.
 *
 * Set by ModbusWorker::handleSubmit() before calling into ModbusClient and
 * read by RequestExecutor / StateMachineBase log sites, so timeout, retry and
 * state-transition log lines can be correlated with the UI-visible
 * TrafficEvent.traceId.
 *
 * thread_local by design: the single-session model (ADR 0005) guarantees one
 * dedicated worker thread per session, and QueuedConnection serializes
 * requests on that thread. Each thread owns its copy — no atomic needed.
 *
 * @warning Any code path that reaches RequestExecutor from a thread other
 * than the ModbusWorker thread will silently observe 0. ModbusWorker asserts
 * thread affinity in debug builds; RequestExecutor logs a one-shot error when
 * it sees 0 on a path that should carry a trace.
 */
inline thread_local quint64 currentTraceId = 0;

/// RAII guard: publishes @p traceId for the current thread and restores the
/// previous value when the request handling scope ends (including on early
/// return). Save/restore — not reset-to-zero — so a nested Scope never erases
/// an outer trace context.
class Scope {
public:
    explicit Scope(quint64 traceId) : previous_(currentTraceId) {
        currentTraceId = traceId;
    }
    ~Scope() { currentTraceId = previous_; }

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

private:
    quint64 previous_;
};

} // namespace modbus::trace
