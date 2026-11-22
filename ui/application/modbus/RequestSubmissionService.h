// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QByteArray>
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <atomic>
#include <optional>
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusPduBuilder.h"
#include "modbus/base/ModbusFunctionTraits.h"
#include "modbus/base/ModbusTypes.h"
#include "ModbusTypes.h"
#include "ModbusExecuteSpec.h"

namespace ui::application::modbus {

struct RequestTrackingInfo {
    RequestKind kind = RequestKind::Read;
    uint16_t address = 0;
    TraceId traceId = 0;
};

// Role: Plain C++ domain service for building Modbus requests, validating IEC 61158 gates, and tracking submissions.
// Thread-safety: Thread-compatible. Driven synchronously on GUI thread.
// Ownership: Owned via std::unique_ptr by ModbusPagePresenter.
class RequestSubmissionService {

public:
    RequestSubmissionService() = default;

    struct RequestBuildResult {
        bool ok = false;
        QString error;
        ::modbus::base::Pdu pdu;
        int requestId = 0;
        TraceId traceId = 0;
    };

    struct ValidationResult {
        bool isValid = false;
        QString errorMessage;
    };

    [[nodiscard]] ValidationResult validateSpec(const ModbusExecuteSpec& spec,
                                               ::modbus::base::ModbusMode mode = ::modbus::base::ModbusMode::TCP) const;

    RequestBuildResult buildExecuteRequest(const ModbusExecuteSpec& spec,
                                          RequestKind kind = RequestKind::Read);

    RequestBuildResult buildReadRequest(const PollSpec& spec,
                                        RequestKind kind = RequestKind::Read);

    std::optional<RequestTrackingInfo> lookupAndRemove(int requestId);
    void clearAll();

    /// Replaces the former txCountUpdated() Qt signal. Invoked on the GUI
    /// thread whenever a new request is tracked. May be empty.
    std::function<void()> onTxCountUpdated;

private:
    [[nodiscard]] ValidationResult validateBroadcastGate(const ModbusExecuteSpec& spec, ::modbus::base::ModbusMode mode) const;
    [[nodiscard]] ValidationResult validateQuantityAndRangeGate(const ModbusExecuteSpec& spec) const;
    [[nodiscard]] ValidationResult validateWirePayloadGate(const ModbusExecuteSpec& spec) const;

    int nextRequestId();
    TraceId nextTraceId();
    void trackRequest(int requestId, RequestKind kind, uint16_t addr, TraceId traceId);

    int requestId_ = 0;
    std::atomic<TraceId> traceIdCounter_{1};
    std::unordered_map<int, RequestTrackingInfo> requestTracking_;
};

} // namespace ui::application::modbus
