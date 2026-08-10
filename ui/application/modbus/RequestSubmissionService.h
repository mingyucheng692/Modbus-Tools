/**
 * @file RequestSubmissionService.h
 * @brief Service for building Modbus requests and tracking submissions.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
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

class RequestSubmissionService : public QObject {
    Q_OBJECT

public:
    explicit RequestSubmissionService(QObject* parent = nullptr);

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

signals:
    void txCountUpdated();

private:
    int nextRequestId();
    TraceId nextTraceId();
    void trackRequest(int requestId, RequestKind kind, uint16_t addr, TraceId traceId);

    int requestId_ = 0;
    std::atomic<TraceId> traceIdCounter_{1};
    std::unordered_map<int, RequestTrackingInfo> requestTracking_;
};

} // namespace ui::application::modbus
