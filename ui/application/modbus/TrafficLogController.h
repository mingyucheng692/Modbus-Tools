/**
 * @file TrafficLogController.h
 * @brief Centralized traffic log policy for Modbus views.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <cstdint>
#include "../../common/TrafficEvent.h"
#include "PollingController.h"

namespace ui::widgets {
class TrafficMonitorWidget;
}

namespace ui::application::modbus {

class PollingController;

class TrafficLogController : public QObject {
    Q_OBJECT

public:
    explicit TrafficLogController(ui::widgets::TrafficMonitorWidget* monitor,
                                  PollingController* pollingController,
                                  QObject* parent = nullptr);

    void setPollingController(PollingController* controller);

    void logConnectionInfo(const QString& message);
    void logRawFrame(ui::common::TrafficDirection direction, const QByteArray& data);
    void logReadSuccess(int retryCount, TraceId traceId = 0);
    void logWriteSuccess(int retryCount, TraceId traceId = 0);
    void logBroadcastWriteSuccess(int retryCount, TraceId traceId = 0);
    void logRequestError(const QString& error, int retryCount, TraceId traceId = 0);
    void logSendingReadRequest(uint8_t fc, int addr, int qty, int slaveId, TraceId traceId = 0);
    void logSendingWriteRequest(uint8_t fc, int addr, const QString& dataStr, int slaveId,
                                TraceId traceId = 0);
    void logSendingRawData(const QByteArray& data);
    void logError(const QString& message);
    void logWarning(const QString& message);
    void logInfo(const QString& message);
    void logPollSummary(const PollSummary& summary);

    /// Single legitimate entry point for traffic events (Task 1.5): appends to
    /// the monitor and crosses the LogBridge into spdlog. Producers must emit
    /// signals wired here; calling ui::logging::relay() directly is forbidden.
    void publishEvent(ui::common::TrafficEvent event);

private:
    static QString retryWord(int retryCount);
    static QString successWithRetrySummary(const QString& baseMessage, int retryCount);
    static QString errorWithRetrySummary(const QString& error, int retryCount);

    ui::widgets::TrafficMonitorWidget* monitor_ = nullptr;
    PollingController* pollingController_ = nullptr;

    // Task 1.4 Raw Frames sampling state (poll frames only; manual frames are
    // never sampled). rawFramePhase_ keeps the 1/N rhythm across summary
    // windows; shown/dropped are per-window counters reported in the summary.
    int rawFramePhase_ = 0;
    int rawFramesShown_ = 0;
    int rawFramesDropped_ = 0;
};

} // namespace ui::application::modbus
