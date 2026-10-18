// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "TrafficLogController.h"
#include "PollingController.h"
#include "Config.h"
#include "../../logging/LogBridge.h"
#include <QCoreApplication>
#include <utility>

namespace ui::application::modbus {

TrafficLogController::TrafficLogController(PollingController* pollingController,
                                           QObject* parent)
    : QObject(parent)
    , pollingController_(pollingController) {
}

void TrafficLogController::setPollingController(PollingController* controller) {
    pollingController_ = controller;
}

void TrafficLogController::setRawFramesEnabled(bool enabled) noexcept {
    rawFramesEnabled_ = enabled;
}

bool TrafficLogController::isRawFramesEnabled() const noexcept {
    return rawFramesEnabled_;
}

void TrafficLogController::publishEvent(ui::common::TrafficEvent event) {
    emit trafficEvent(event);
    ui::logging::relay(event);
}

void TrafficLogController::logConnectionInfo(const QString& message) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::Connection;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logRawFrame(ui::common::TrafficDirection direction,
                                       const QByteArray& data) {
    const bool suppressLog = pollingController_
        && pollingController_->isSuppressingTrafficLog();
    const bool rawEnabled = rawFramesEnabled_;
    const bool allowRawFrameLog = !suppressLog || rawEnabled;
    if (!allowRawFrameLog) return;

    if (suppressLog && rawEnabled) {
        rawFramePhase_ = (rawFramePhase_ + 1) % config::Ui::kRawFrameSampleRate;
        if (rawFramePhase_ != 0) {
            ++rawFramesDropped_;
            return;
        }
        ++rawFramesShown_;
    }

    ui::common::TrafficEvent event;
    event.direction = direction;
    event.requestType = ui::common::TrafficRequestType::Unknown;
    event.isPoll = suppressLog;
    event.payload = data;
    publishEvent(std::move(event));
}

void TrafficLogController::logReadSuccess(int retryCount, TraceId traceId) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualRead;
    event.traceId = traceId;
    event.summary = successWithRetrySummary(tr("Success: Response received"), retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logWriteSuccess(int retryCount, TraceId traceId) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualWrite;
    event.traceId = traceId;
    event.summary = successWithRetrySummary(tr("Success: Write confirmed"), retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logBroadcastWriteSuccess(int retryCount, TraceId traceId) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualWrite;
    event.traceId = traceId;
    event.summary = successWithRetrySummary(
        tr("Success: Broadcast write sent, no response expected"), retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logRequestError(const QString& error, int retryCount, TraceId traceId) {
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Error;
    event.traceId = traceId;
    event.summary = errorWithRetrySummary(error, retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logSendingReadRequest(uint8_t fc, int addr, int qty, int slaveId,
                                                 TraceId traceId) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualRead;
    event.traceId = traceId;
    event.summary = tr("Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4")
        .arg(fc).arg(addr).arg(qty).arg(slaveId);
    publishEvent(std::move(event));
}

void TrafficLogController::logSendingWriteRequest(uint8_t fc, int addr,
                                                  const QString& dataStr, int slaveId,
                                                  TraceId traceId) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualWrite;
    event.traceId = traceId;
    event.summary = tr("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
        .arg(fc).arg(addr).arg(dataStr).arg(slaveId);
    publishEvent(std::move(event));
}

void TrafficLogController::logSendingRawData(const QByteArray& data) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::RawSend;
    event.summary = tr("Sending Raw Data: %1")
        .arg(QString(data.toHex(' ').toUpper()));
    publishEvent(std::move(event));
}

void TrafficLogController::logError(const QString& message) {
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Error;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logWarning(const QString& message) {
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Warning;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logInfo(const QString& message) {
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Info;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logPollSummary(const PollSummary& summary) {
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::Poll;
    event.isPoll = true;

    QString summaryText = tr("Poll Summary: %1 ok, %2 err, %3 retry, avg RTT %4 ms")
        .arg(summary.successCount)
        .arg(summary.errorCount)
        .arg(summary.retryCount)
        .arg(summary.avgRttMs);

    if (rawFramesShown_ > 0 || rawFramesDropped_ > 0) {
        summaryText += tr(" (raw frames: %1 shown, %2 sampled)")
            .arg(rawFramesShown_)
            .arg(rawFramesDropped_);
        rawFramesShown_ = 0;
        rawFramesDropped_ = 0;
    }

    event.summary = summaryText;
    publishEvent(std::move(event));
}

QString TrafficLogController::retryWord(int retryCount) {
    return (retryCount == 1)
        ? QCoreApplication::translate("TrafficLogController", "retry")
        : QCoreApplication::translate("TrafficLogController", "retries");
}

QString TrafficLogController::successWithRetrySummary(const QString& baseMessage,
                                                      int retryCount) {
    if (retryCount <= 0) return baseMessage;
    return QStringLiteral("%1 (%2 %3)")
        .arg(baseMessage)
        .arg(retryCount)
        .arg(retryWord(retryCount));
}

QString TrafficLogController::errorWithRetrySummary(const QString& error, int retryCount) {
    if (retryCount <= 0) {
        return QCoreApplication::translate("TrafficLogController", "Error: %1").arg(error);
    }
    return QCoreApplication::translate("TrafficLogController", "Error: %1 (after %2 %3)")
        .arg(error)
        .arg(retryCount)
        .arg(retryWord(retryCount));
}

} // namespace ui::application::modbus
