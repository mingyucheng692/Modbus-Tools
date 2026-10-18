// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <cstdint>
#include "../../common/TrafficEvent.h"
#include "PollingController.h"

namespace ui::application::modbus {

class PollingController;

/// Role: High-level log & traffic controller converting protocol transactions into structured TrafficEvents.
/// Thread-safety: Thread-affine (bound to GUI main thread).
/// Ownership: Owned by ModbusPagePresenter via QPointer/QObject hierarchy.
class TrafficLogController : public QObject {
    Q_OBJECT

public:
    explicit TrafficLogController(PollingController* pollingController = nullptr,
                                  QObject* parent = nullptr);

    void setPollingController(PollingController* controller);
    void setRawFramesEnabled(bool enabled) noexcept;
    [[nodiscard]] bool isRawFramesEnabled() const noexcept;

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

    /// Single legitimate entry point for traffic events: emits trafficEvent and crosses LogBridge into spdlog.
    void publishEvent(ui::common::TrafficEvent event);

signals:
    void trafficEvent(const ::ui::common::TrafficEvent& event);

private:
    static QString retryWord(int retryCount);
    static QString successWithRetrySummary(const QString& baseMessage, int retryCount);
    static QString errorWithRetrySummary(const QString& error, int retryCount);

    PollingController* pollingController_ = nullptr;
    bool rawFramesEnabled_ = false;

    int rawFramePhase_ = 0;
    int rawFramesShown_ = 0;
    int rawFramesDropped_ = 0;
};

} // namespace ui::application::modbus
