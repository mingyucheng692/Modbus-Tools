/**
 * @file ModbusPagePresenter.cpp
 * @brief Implementation of ModbusPagePresenter.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusPagePresenter.h"
#include "ModbusSessionPresenter.h"
#include "RequestSubmissionService.h"
#include "PollingController.h"
#include "TrafficLogController.h"
#include "RequestCoordinator.h"
#include "../../widgets/BaseConnectionWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../views/modbus/ModbusPage.h"

namespace ui::application::modbus {

ModbusPagePresenter::ModbusPagePresenter(ui::views::modbus::ModbusPage* view,
                                         SessionMode mode,
                                         QObject* parent)
    : QObject(parent),
      view_(view),
      mode_(mode) {
}

ModbusPagePresenter::~ModbusPagePresenter() noexcept = default;

void ModbusPagePresenter::setup(ui::widgets::BaseConnectionWidget* connectionWidget,
                                ui::widgets::ControlWidget* controlWidget,
                                ui::widgets::FunctionWidget* functionWidget,
                                ui::widgets::TrafficMonitorWidget* trafficMonitor) {
    connectionWidget_ = connectionWidget;
    controlWidget_ = controlWidget;
    functionWidget_ = functionWidget;
    trafficMonitor_ = trafficMonitor;
    createServices();
    wireConnections();
}

void ModbusPagePresenter::createServices() {
    sessionPresenter_ = new ModbusSessionPresenter(mode_, nullptr, this);
    requestService_ = new RequestSubmissionService(this);
    pollingController_ = new PollingController(requestService_, this);
    trafficLogController_ = new TrafficLogController(
        trafficMonitor_, pollingController_, this);
    requestCoordinator_ = new RequestCoordinator(
        sessionPresenter_, requestService_, pollingController_,
        trafficLogController_, mode_, this);

    sessionPresenter_->setConnectionWidget(connectionWidget_);
    sessionPresenter_->setControlWidget(controlWidget_);
    sessionPresenter_->setRequestService(requestService_);
    sessionPresenter_->setPollingController(pollingController_);
    sessionPresenter_->setTrafficLogController(trafficLogController_);

    // trafficEvent → trafficMonitor (needs the monitor pointer, so wire here
    // rather than in wireConnections() which does not retain it).
    connect(pollingController_, &PollingController::trafficEvent,
            trafficMonitor_, &ui::widgets::TrafficMonitorWidget::appendEvent);
}

void ModbusPagePresenter::wireConnections() {
    connect(requestService_, &RequestSubmissionService::txCountUpdated,
            controlWidget_, &ui::widgets::ControlWidget::recordTx);

    connect(pollingController_, &PollingController::submitPollRequest,
            this, [this](const ::modbus::base::Pdu& pdu, int slaveId, int requestId) {
                if (sessionPresenter_) {
                    sessionPresenter_->submitRequest(pdu, slaveId, requestId);
                }
            });
    connect(pollingController_, &PollingController::summaryReady,
            trafficLogController_, &TrafficLogController::logPollSummary);

    if (functionWidget_) {
        connect(functionWidget_, &ui::widgets::FunctionWidget::logMessageRequested,
                this, [this](const QString& message, bool isError) {
                    if (!trafficLogController_) return;
                    if (isError) {
                        trafficLogController_->logWarning(message);
                    } else {
                        trafficLogController_->logInfo(message);
                    }
                });
    }

    if (controlWidget_) {
        connect(controlWidget_, &ui::widgets::ControlWidget::logMessageRequested,
                this, [this](const QString& message, bool isError) {
                    if (!trafficLogController_) return;
                    if (isError) {
                        trafficLogController_->logWarning(message);
                    } else {
                        trafficLogController_->logInfo(message);
                    }
                });

        connect(controlWidget_, &ui::widgets::ControlWidget::linkToggled,
                this, [this](bool active) {
                    linked_ = active;
                    emit linkageToggled(active);
                });

        connect(controlWidget_, &ui::widgets::ControlWidget::pollRequested,
                this, [this](uint8_t fc, int addr, int qty) {
                    if (!requestCoordinator_) return;
                    requestCoordinator_->handlePollRequest(fc, addr, qty,
                        controlWidget_->pollingIntervalMs());
                });
    }

    if (sessionPresenter_) {
        connect(sessionPresenter_,
                &ModbusSessionPresenter::linkageSourceDisconnected,
                this, &ModbusPagePresenter::linkageSourceDisconnected);

        connect(sessionPresenter_,
                &ModbusSessionPresenter::rawFrameReceived,
                this, [this](bool isTx, const QByteArray& data) {
                    if (view_) {
                        view_->appendTrafficData(isTx, data);
                    }
                });

        connect(sessionPresenter_,
                &ModbusSessionPresenter::requestFinished,
                this, [this](int requestId, const ::modbus::session::ModbusResponse& response) {
                    if (!requestCoordinator_) return;
                    requestCoordinator_->handleRequestFinished(requestId, response);
                });
    }

    if (requestCoordinator_) {
        connect(requestCoordinator_,
                &RequestCoordinator::linkageDataReceived,
                this, [this](const ::modbus::base::Pdu& pdu,
                             ::modbus::core::parser::ProtocolType protocol,
                             uint16_t addr) {
                    if (linked_) {
                        emit linkageDataReceived(pdu, protocol, addr);
                    }
                });
    }

    if (functionWidget_) {
        connect(functionWidget_, &ui::widgets::FunctionWidget::readRequested,
                this, [this](uint8_t fc, int addr, int qty, int slaveId) {
                    if (!requestCoordinator_) return;
                    requestCoordinator_->handleReadRequest(fc, addr, qty, slaveId);
                });

        connect(functionWidget_, &ui::widgets::FunctionWidget::writeRequested,
                this, [this](uint8_t fc, int addr, const QString& dataStr,
                             const QString& fmt, int slaveId) {
                    if (!requestCoordinator_) return;
                    requestCoordinator_->handleWriteRequest(fc, addr, dataStr, fmt, slaveId,
                                                            functionWidget_->getQuantity());
                });

        connect(functionWidget_, &ui::widgets::FunctionWidget::rawSendRequested,
                this, [this](const QByteArray& data) {
                    if (!requestCoordinator_) return;
                    requestCoordinator_->handleRawSendRequest(data);
                });
    }
}

void ModbusPagePresenter::teardownServices() {
    // Delete in reverse construction order. All are QObject children of this,
    // so synchronous delete is safe; ~QObject() removes pending queued
    // connections automatically. Using delete (not deleteLater()) ensures the
    // old services are fully gone before createServices() runs.
    if (requestCoordinator_) { delete requestCoordinator_; requestCoordinator_ = nullptr; }
    if (trafficLogController_) { delete trafficLogController_; trafficLogController_ = nullptr; }
    if (pollingController_) { delete pollingController_; pollingController_ = nullptr; }
    if (requestService_) { delete requestService_; requestService_ = nullptr; }
    if (sessionPresenter_) { delete sessionPresenter_; sessionPresenter_ = nullptr; }
}

void ModbusPagePresenter::switchMode(SessionMode newMode,
                                      ui::widgets::BaseConnectionWidget* newConnectionWidget) {
    if (newMode == mode_ && newConnectionWidget == connectionWidget_) {
        return;
    }

    // Disconnect first if a session is active.
    if (sessionPresenter_ && sessionPresenter_->isSessionConnected()) {
        sessionPresenter_->requestDisconnect();
    }

    teardownServices();

    mode_ = newMode;
    connectionWidget_ = newConnectionWidget;
    linked_ = false;

    createServices();
    wireConnections();
}

void ModbusPagePresenter::requestConnect(const ModbusConnectionSpec& spec) {
    if (sessionPresenter_) {
        sessionPresenter_->requestConnect(spec);
    }
}

void ModbusPagePresenter::requestDisconnect() {
    if (sessionPresenter_) {
        sessionPresenter_->requestDisconnect();
    }
}

void ModbusPagePresenter::updateSettings(const ModbusTimingParams& params) {
    if (sessionPresenter_) {
        sessionPresenter_->updateSettings(params);
    }
}

void ModbusPagePresenter::setLinked(bool linked) {
    linked_ = linked;
    if (sessionPresenter_) {
        sessionPresenter_->setLinked(linked);
    }
}

bool ModbusPagePresenter::isSessionConnected() const {
    return sessionPresenter_ && sessionPresenter_->isSessionConnected();
}

bool ModbusPagePresenter::isLinked() const {
    return linked_;
}

ModbusSessionPresenter* ModbusPagePresenter::sessionPresenter() const {
    return sessionPresenter_;
}

} // namespace ui::application::modbus
