/**
 * @file RequestCoordinator.cpp
 * @brief Implementation of RequestCoordinator.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RequestCoordinator.h"
#include "RequestSubmissionService.h"
#include "PollingController.h"
#include "TrafficLogController.h"
#include "ModbusTypes.h"
#include "../../common/ConnectionAlert.h"
#include <QWidget>
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

RequestCoordinator::RequestCoordinator(ModbusSessionPresenter* presenter,
                                       RequestSubmissionService* requestService,
                                       PollingController* pollingController,
                                       TrafficLogController* trafficLogController,
                                       SessionMode sessionMode,
                                       QObject* parent)
    : QObject(parent),
      presenter_(presenter),
      requestService_(requestService),
      pollingController_(pollingController),
      trafficLogController_(trafficLogController),
      sessionMode_(sessionMode) {
}

bool RequestCoordinator::ensureConnected() const {
    if (presenter_ && presenter_->isSessionConnected()) {
        return true;
    }
    // ConnectionAlert::showNotConnected needs a QWidget* parent; we pass
    // nullptr here since the alert is modeless and the user will see it.
    ui::common::ConnectionAlert::showNotConnected(nullptr);
    return false;
}

void RequestCoordinator::handleReadRequest(uint8_t fc, int addr, int qty, int slaveId) {
    if (!ensureConnected()) return;

    if (!requestService_) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Error: Request service not available"));
        }
        return;
    }

    PollSpec spec;
    spec.functionCode = fc;
    spec.startAddress = static_cast<uint16_t>(addr);
    spec.quantity = static_cast<uint16_t>(qty);
    spec.slaveId = static_cast<uint8_t>(slaveId);

    auto result = requestService_->buildReadRequest(spec);
    if (!result.ok) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Error: %1").arg(result.error));
        }
        return;
    }

    if (trafficLogController_) {
        trafficLogController_->logSendingReadRequest(fc, addr, qty, slaveId);
    }

    presenter_->submitRequest(result.pdu, slaveId, result.requestId);
}

void RequestCoordinator::handleWriteRequest(uint8_t fc, int addr,
                                            const QString& dataStr,
                                            const QString& fmt, int slaveId,
                                            int quantity) {
    if (!ensureConnected()) return;

    if (!requestService_) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Error: Request service not available"));
        }
        return;
    }

    auto result = requestService_->buildWriteRequest(fc, addr, dataStr, fmt, slaveId, quantity);
    if (!result.ok) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Error: %1").arg(result.error));
        }
        return;
    }

    if (trafficLogController_) {
        trafficLogController_->logSendingWriteRequest(fc, addr, dataStr, slaveId);
    }

    presenter_->submitRequest(result.pdu, slaveId, result.requestId);
}

void RequestCoordinator::handleRawSendRequest(const QByteArray& data) {
    if (!ensureConnected()) return;
    if (!requestService_ || !requestService_->validateRawData(data)) return;

    if (trafficLogController_) {
        trafficLogController_->logSendingRawData(data);
    }

    presenter_->sendRaw(data);
}

void RequestCoordinator::handlePollRequest(uint8_t fc, int addr, int qty, int intervalMs) {
    if (!ensureConnected()) return;
    if (!pollingController_) return;

    pollingController_->setPollingInterval(intervalMs);

    PollSpec spec;
    spec.functionCode = fc;
    spec.startAddress = static_cast<uint16_t>(addr);
    spec.quantity = static_cast<uint16_t>(qty);
    spec.slaveId = static_cast<uint8_t>(config::Modbus::kDefaultSlaveId);

    pollingController_->handlePollRequest(spec);
}

void RequestCoordinator::handleRequestFinished(int requestId,
                                               const ::modbus::session::ModbusResponse& response) {
    if (!requestService_) return;

    auto trackingInfo = requestService_->lookupAndRemove(requestId);
    if (!trackingInfo.has_value()) {
        return;
    }

    uint16_t addr = trackingInfo->address;

    if (!response.isError()) {
        const ::modbus::core::parser::ProtocolType protocolType =
            modeDescriptor(sessionMode_).protocolType;
        emit linkageDataReceived(response.pdu, protocolType, addr);
    }
}

} // namespace ui::application::modbus
