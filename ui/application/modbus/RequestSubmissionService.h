/**
 * @file RequestSubmissionService.h
 * @brief Service for building Modbus requests and tracking submissions.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QByteArray>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <cstdint>
#include <atomic>
#include <optional>
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusPduBuilder.h"
#include "ModbusTypes.h"

namespace ui::application::modbus {

struct RequestTrackingInfo {
    RequestKind kind = RequestKind::Read;
    uint16_t address = 0;
    TraceId traceId = 0;
    std::chrono::steady_clock::time_point startTime{};
};

/**
 * @brief Plain C++ service for building Modbus requests and tracking
 *        submissions.
 *
 * Deliberately NOT a QObject: it has no signal/slot needs
 * beyond a single notification, which is exposed as the
 * `onTxCountUpdated` std::function callback. Ownership is std::unique_ptr;
 * the class must never be deleteLater()'d (no event-loop protection).
 *
 * Lifetime contract: the composition root (ModbusPagePresenter) assigns
 * `onTxCountUpdated` and must guarantee the captured target outlives this
 * service, or capture a QPointer/weak guard inside the callback.
 */
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

    RequestBuildResult buildReadRequest(const PollSpec& spec,
                                        RequestKind kind = RequestKind::Read);
    RequestBuildResult buildWriteRequest(uint8_t fc, int addr, const QString& dataStr,
                                         const QString& fmt, int slaveId, int quantity);
    bool validateRawData(const QByteArray& data, QString* errorOut = nullptr);

    std::optional<RequestTrackingInfo> lookup(int requestId) const;
    std::optional<RequestTrackingInfo> lookupAndRemove(int requestId);
    void clearAll();

    /// Replaces the former txCountUpdated() Qt signal. Invoked on the GUI
    /// thread whenever a new request is tracked. May be empty.
    std::function<void()> onTxCountUpdated;

private:
    int nextRequestId();
    TraceId nextTraceId();
    void trackRequest(int requestId, RequestKind kind, uint16_t addr, TraceId traceId);

    int requestId_ = 0;
    std::atomic<TraceId> traceIdCounter_{1};
    std::unordered_map<int, RequestTrackingInfo> requestTracking_;
};

} // namespace ui::application::modbus
