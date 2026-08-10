/**
 * @file TrafficLogController.cpp
 * @brief Implementation of TrafficLogController.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TrafficLogController.h"
#include "PollingController.h"
#include "../../logging/LogBridge.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include <QCoreApplication>
#include <utility>

namespace ui::application::modbus {

TrafficLogController::TrafficLogController(ui::widgets::TrafficMonitorWidget* monitor,
                                           PollingController* pollingController,
                                           QObject* parent)
    : QObject(parent)
    , monitor_(monitor)
    , pollingController_(pollingController) {
}

void TrafficLogController::setPollingController(PollingController* controller) {
    pollingController_ = controller;
}

void TrafficLogController::publishEvent(ui::common::TrafficEvent event) {
    if (!monitor_) {
        return;
    }
    monitor_->appendEvent(event);
    ui::logging::relay(event);
}

void TrafficLogController::logConnectionInfo(const QString& message) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::Connection;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logRawFrame(ui::common::TrafficDirection direction,
                                       const QByteArray& data) {
    if (!monitor_) return;

    const bool suppressLog = pollingController_
        && pollingController_->isSuppressingTrafficLog();
    const bool allowRawFrameLog = !suppressLog
        || monitor_->isRawFramesModeEnabled();
    if (!allowRawFrameLog) return;

    ui::common::TrafficEvent event;
    event.direction = direction;
    event.requestType = ui::common::TrafficRequestType::Unknown;
    event.isPoll = suppressLog;
    event.payload = data;
    publishEvent(std::move(event));
}

void TrafficLogController::logReadSuccess(int retryCount, TraceId traceId) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualRead;
    event.traceId = traceId;
    event.summary = successWithRetrySummary(tr("Success: Response received"), retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logWriteSuccess(int retryCount, TraceId traceId) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualWrite;
    event.traceId = traceId;
    event.summary = successWithRetrySummary(tr("Success: Write confirmed"), retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logBroadcastWriteSuccess(int retryCount, TraceId traceId) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualWrite;
    event.traceId = traceId;
    event.summary = successWithRetrySummary(
        tr("Success: Broadcast write sent, no response expected"), retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logRequestError(const QString& error, int retryCount, TraceId traceId) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Error;
    event.traceId = traceId;
    event.summary = errorWithRetrySummary(error, retryCount);
    publishEvent(std::move(event));
}

void TrafficLogController::logSendingReadRequest(uint8_t fc, int addr, int qty, int slaveId,
                                                 TraceId traceId) {
    if (!monitor_) return;
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
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::ManualWrite;
    event.traceId = traceId;
    event.summary = tr("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
        .arg(fc).arg(addr).arg(dataStr).arg(slaveId);
    publishEvent(std::move(event));
}

void TrafficLogController::logSendingRawData(const QByteArray& data) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::RawSend;
    event.summary = tr("Sending Raw Data: %1")
        .arg(QString(data.toHex(' ').toUpper()));
    publishEvent(std::move(event));
}

void TrafficLogController::logError(const QString& message) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Error;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logWarning(const QString& message) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Warning;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logInfo(const QString& message) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.summary = message;
    publishEvent(std::move(event));
}

void TrafficLogController::logPollSummary(const PollSummary& summary) {
    if (!monitor_) return;
    const QString avgRttText = summary.successCount > 0
        ? tr("%1 ms").arg(summary.avgRttMs)
        : tr("--");

    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Info;
    event.requestType = ui::common::TrafficRequestType::Poll;
    event.isPoll = true;
    event.summary = tr("Poll Summary FC:%1 Addr:%2 Qty:%3 Slave:%4 Success:%5 Error:%6 Retries:%7 Avg Success RTT:%8")
        .arg(summary.functionCode)
        .arg(summary.address)
        .arg(summary.quantity)
        .arg(summary.slaveId)
        .arg(summary.successCount)
        .arg(summary.errorCount)
        .arg(summary.retryCount)
        .arg(avgRttText);
    publishEvent(std::move(event));
}

QString TrafficLogController::retryWord(int retryCount) {
    return QCoreApplication::translate(
        "ui::application::modbus::TrafficLogController",
        retryCount == 1 ? "retry" : "retries");
}

QString TrafficLogController::successWithRetrySummary(const QString& baseMessage,
                                                      int retryCount) {
    if (retryCount <= 0) {
        return baseMessage;
    }
    return QCoreApplication::translate(
               "ui::application::modbus::TrafficLogController",
               "%1 after %2 %3")
        .arg(baseMessage)
        .arg(retryCount)
        .arg(retryWord(retryCount));
}

QString TrafficLogController::errorWithRetrySummary(const QString& error,
                                                    int retryCount) {
    if (retryCount <= 0) {
        return QCoreApplication::translate(
                   "ui::application::modbus::TrafficLogController",
                   "Error: %1")
            .arg(error);
    }
    return QCoreApplication::translate(
               "ui::application::modbus::TrafficLogController",
               "Error: %1 (failed after %2 %3, see log for details)")
        .arg(error)
        .arg(retryCount)
        .arg(retryWord(retryCount));
}

} // namespace ui::application::modbus
