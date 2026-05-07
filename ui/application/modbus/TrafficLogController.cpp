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
#include "../../widgets/TrafficMonitorWidget.h"
#include <QCoreApplication>

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

void TrafficLogController::logConnectionInfo(const QString& message) {
    if (!monitor_) return;
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::Connection;
    event.summary = message;
    monitor_->appendEvent(event);
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
    monitor_->appendEvent(event);
}

void TrafficLogController::logReadSuccess(int retryCount) {
    if (!monitor_) return;
    monitor_->appendInfo(
        successWithRetrySummary(tr("Success: Response received"), retryCount));
}

void TrafficLogController::logWriteSuccess(int retryCount) {
    if (!monitor_) return;
    monitor_->appendInfo(
        successWithRetrySummary(tr("Success: Write confirmed"), retryCount));
}

void TrafficLogController::logBroadcastWriteSuccess(int retryCount) {
    if (!monitor_) return;
    monitor_->appendInfo(successWithRetrySummary(
        tr("Success: Broadcast write sent, no response expected"), retryCount));
}

void TrafficLogController::logRequestError(const QString& error, int retryCount) {
    if (!monitor_) return;
    monitor_->appendError(errorWithRetrySummary(error, retryCount));
}

void TrafficLogController::logSendingReadRequest(uint8_t fc, int addr, int qty, int slaveId) {
    if (!monitor_) return;
    monitor_->appendInfo(tr("Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4")
        .arg(fc).arg(addr).arg(qty).arg(slaveId));
}

void TrafficLogController::logSendingWriteRequest(uint8_t fc, int addr,
                                                  const QString& dataStr, int slaveId) {
    if (!monitor_) return;
    monitor_->appendInfo(tr("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
        .arg(fc).arg(addr).arg(dataStr).arg(slaveId));
}

void TrafficLogController::logSendingRawData(const QByteArray& data) {
    if (!monitor_) return;
    monitor_->appendInfo(tr("Sending Raw Data: %1")
        .arg(QString(data.toHex(' ').toUpper())));
}

void TrafficLogController::logError(const QString& message) {
    if (!monitor_) return;
    monitor_->appendError(message);
}

void TrafficLogController::logWarning(const QString& message) {
    if (!monitor_) return;
    monitor_->appendWarning(message);
}

void TrafficLogController::logInfo(const QString& message) {
    if (!monitor_) return;
    monitor_->appendInfo(message);
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
