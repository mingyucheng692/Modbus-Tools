// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QtGlobal>

namespace modbus::trace {

// Trace id of the request currently being processed on this thread (thread_local).
// Set by ModbusWorker::handleSubmit() and read by RequestExecutor/StateMachine log sites.
inline thread_local quint64 currentTraceId = 0;

// Role: RAII guard publishing traceId for current thread and restoring previous on scope exit.
// Thread-safety: Thread-compatible (bound to current thread stack).
// Ownership: Stack-allocated RAII guard.
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
