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
#include "modbus/session/SessionTypes.h"
#include <spdlog/spdlog.h>
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

ModbusPagePresenter::~ModbusPagePresenter() noexcept {
    teardownServices();
}

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
    sessionPresenter_ = new ModbusSessionPresenter(mode_, this);
    // Plain C++ service: unique_ptr, no QObject parent.
    requestService_ = std::make_unique<RequestSubmissionService>();
    pollingController_ = new PollingController(requestService_.get(), this);
    trafficLogController_ = new TrafficLogController(
        trafficMonitor_, pollingController_, this);

    sessionPresenter_->setConnectionWidget(connectionWidget_);
    sessionPresenter_->setControlWidget(controlWidget_);
    sessionPresenter_->setRequestService(requestService_.get());
    sessionPresenter_->setPollingController(pollingController_);
    sessionPresenter_->setTrafficLogController(trafficLogController_);

    // TrafficEvent flows through TrafficLogController::publishEvent,
    // the single legitimate bridge entry (monitor append + LogBridge relay).
    // Wiring directly to the monitor would bypass relay() and re-create the
    // dual-entry problem.
    connect(pollingController_, &PollingController::trafficEvent,
            trafficLogController_, &TrafficLogController::publishEvent);

    syncWidgetGuards(SessionConnectionState::Disconnected);
}

void ModbusPagePresenter::wireConnections() {
    // RequestSubmissionService is no longer a QObject; the
    // former txCountUpdated -> recordTx connection is a direct callback.
    // Lifetime: controlWidget_ is owned by the View and outlives this
    // presenter's services (torn down on switchMode before widgets change).
    if (controlWidget_) {
        requestService_->onTxCountUpdated = [controlWidget = controlWidget_]() {
            controlWidget->recordTx();
        };
    }

    connect(pollingController_, &PollingController::submitPollRequest,
            this, [this](const ::modbus::base::Pdu& pdu, int slaveId, int requestId,
                         TraceId traceId) {
                if (sessionPresenter_) {
                    sessionPresenter_->submitRequest(pdu, slaveId, requestId, traceId);
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
                    handlePollRequest(fc, addr, qty,
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
                this, &ModbusPagePresenter::handleRequestFinished);

        connect(sessionPresenter_, &ModbusSessionPresenter::connectFinished, this,
                [this](bool ok, const QString&) {
                    if (!ok) return;
                    syncWidgetGuards(SessionConnectionState::Connected);
                });
        connect(sessionPresenter_, &ModbusSessionPresenter::sessionConnected, this,
                [this]() { syncWidgetGuards(SessionConnectionState::Connected); });
        connect(sessionPresenter_, &ModbusSessionPresenter::sessionDisconnected, this,
                [this](const QString&) { syncWidgetGuards(SessionConnectionState::Disconnected); });
    }

    if (functionWidget_) {
        connect(functionWidget_, &ui::widgets::FunctionWidget::readRequested,
                this, &ModbusPagePresenter::handleReadRequest);

        connect(functionWidget_, &ui::widgets::FunctionWidget::writeRequested,
                this, [this](uint8_t fc, int addr, const QString& dataStr,
                             const QString& fmt, int slaveId) {
                    handleWriteRequest(fc, addr, dataStr, fmt, slaveId,
                                       functionWidget_->getQuantity());
                });

        connect(functionWidget_, &ui::widgets::FunctionWidget::rawSendRequested,
                this, &ModbusPagePresenter::handleRawSendRequest);
    }
}

void ModbusPagePresenter::teardownServices() {
    // Delete in reverse construction order. All are QObject children of this,
    // so synchronous delete is safe; ~QObject() removes pending queued
    // connections automatically. Using delete (not deleteLater()) ensures the
    // old services are fully gone before createServices() runs.
    if (trafficLogController_) { delete trafficLogController_; trafficLogController_ = nullptr; }
    if (pollingController_) { delete pollingController_; pollingController_ = nullptr; }
    requestService_.reset();
    if (sessionPresenter_) { delete sessionPresenter_; sessionPresenter_ = nullptr; }
}

void ModbusPagePresenter::switchMode(SessionMode newMode,
                                      ui::widgets::BaseConnectionWidget* newConnectionWidget) {
    if (newMode == mode_ && newConnectionWidget == connectionWidget_) {
        return;
    }

    pendingMode_ = newMode;
    pendingConnectionWidget_ = newConnectionWidget;

    if (sessionPresenter_) {
        connect(sessionPresenter_, &ModbusSessionPresenter::stackReleased,
                this, &ModbusPagePresenter::onStackReleasedForSwitch, Qt::UniqueConnection);
        if (sessionPresenter_->isSessionConnected()) {
            sessionPresenter_->requestDisconnect();
        } else {
            sessionPresenter_->releaseStack();
        }
        return;
    }

    teardownServices();
    mode_ = newMode;
    connectionWidget_ = newConnectionWidget;
    linked_ = false;
    lastSyncedState_.reset();
    createServices();
    wireConnections();
}

void ModbusPagePresenter::onStackReleasedForSwitch() {
    if (!sessionPresenter_) return;

    disconnect(sessionPresenter_, &ModbusSessionPresenter::stackReleased,
               this, &ModbusPagePresenter::onStackReleasedForSwitch);

    // Defer teardown to the next event loop iteration: stackReleased is
    // emitted synchronously from ModbusSessionPresenter::requestRelease(),
    // so we must not delete sessionPresenter_ while it is still on the
    // call stack.
    QMetaObject::invokeMethod(this, [this]() {
        teardownServices();

        mode_ = pendingMode_;
        connectionWidget_ = pendingConnectionWidget_;
        linked_ = false;
        lastSyncedState_.reset();

        createServices();
        wireConnections();
    }, Qt::QueuedConnection);
}

void ModbusPagePresenter::setLinked(bool linked) {
    linked_ = linked;
    if (sessionPresenter_) {
        sessionPresenter_->setLinked(linked);
    }
}

bool ModbusPagePresenter::isLinked() const {
    return linked_;
}

ModbusSessionPresenter* ModbusPagePresenter::sessionPresenter() const {
    return sessionPresenter_;
}

void ModbusPagePresenter::requestConnect(const ModbusConnectionSpec& spec) {
    if (!sessionPresenter_) {
        SPDLOG_WARN("ModbusPagePresenter: requestConnect dropped (no session presenter)");
        return;
    }
    // 防重入保护：如果当前正在 Connecting 阶段，忽略重复连击
    if (sessionPresenter_->connectionState() == SessionConnectionState::Connecting) {
        SPDLOG_INFO("ModbusPagePresenter: connect request ignored (already connecting)");
        return;
    }
    sessionPresenter_->requestConnect(spec);
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

bool ModbusPagePresenter::ensureConnected() {
    if (sessionPresenter_ && sessionPresenter_->isSessionConnected()) {
        return true;
    }
    const auto state = sessionPresenter_
        ? sessionPresenter_->connectionState()
        : SessionConnectionState::Disconnected;
    const QString msg = [state, this]() -> QString {
        switch (state) {
        case SessionConnectionState::Connecting:
            return tr("Request rejected: connection in progress, please wait.");
        case SessionConnectionState::Disconnecting:
            return tr("Request rejected: disconnecting in progress.");
        default:
            return tr("Request rejected: device not connected.");
        }
    }();
    if (trafficLogController_) {
        trafficLogController_->logInfo(msg);
    } else {
        SPDLOG_WARN("ensureConnected (TLC unavailable): {}", msg.toStdString());
    }
    return false;
}

void ModbusPagePresenter::syncWidgetGuards(SessionConnectionState state) {
    if (lastSyncedState_.has_value() && *lastSyncedState_ == state) {
        return;
    }
    lastSyncedState_ = state;

    const bool readEnabled  = (state == SessionConnectionState::Connected
                               || state == SessionConnectionState::TransportConnected);
    const bool writeEnabled = (state == SessionConnectionState::Connected);
    const bool pollEnabled  = (state == SessionConnectionState::Connected);

    SPDLOG_DEBUG("ModbusPagePresenter::syncWidgetGuards state={} read={} write={} poll={}",
                 connectionStateName(state), readEnabled, writeEnabled, pollEnabled);

    if (functionWidget_) {
        if (state == SessionConnectionState::TransportConnected) {
            const QString writeTip = tr("Device is establishing session. "
                                        "Read commands available; write commands locked until ready.");
            functionWidget_->setReadOpsEnabled(true);
            functionWidget_->setWriteOpsEnabled(false, writeTip);
        } else {
            const QString tip = [state, this]() -> QString {
                switch (state) {
                case SessionConnectionState::Connecting:
                    return tr("Connecting — please wait before sending commands.");
                case SessionConnectionState::Disconnecting:
                    return tr("Disconnecting...");
                default:
                    return tr("Connect device to send commands.");
                }
            }();
            functionWidget_->setReadOpsEnabled(readEnabled, tip);
            functionWidget_->setWriteOpsEnabled(writeEnabled, tip);
        }
    }
    if (controlWidget_) {
        controlWidget_->setInteractionsEnabled(pollEnabled);
    }
}

void ModbusPagePresenter::handleReadRequest(uint8_t fc, int addr, int qty, int slaveId) {
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
        trafficLogController_->logSendingReadRequest(fc, addr, qty, slaveId, result.traceId);
    }

    sessionPresenter_->submitRequest(result.pdu, slaveId, result.requestId, result.traceId);
}

void ModbusPagePresenter::handleWriteRequest(uint8_t fc, int addr,
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
        trafficLogController_->logSendingWriteRequest(fc, addr, dataStr, slaveId,
                                                      result.traceId);
    }

    sessionPresenter_->submitRequest(result.pdu, slaveId, result.requestId, result.traceId);
}

void ModbusPagePresenter::handleRawSendRequest(const QByteArray& data) {
    if (!ensureConnected()) return;
    if (!requestService_ || !requestService_->validateRawData(data)) return;

    if (trafficLogController_) {
        trafficLogController_->logSendingRawData(data);
    }

    sessionPresenter_->sendRaw(data);
}

void ModbusPagePresenter::handlePollRequest(uint8_t fc, int addr, int qty, int intervalMs) {
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

void ModbusPagePresenter::handleRequestFinished(int requestId,
                                                const ::modbus::session::ModbusResponse& response) {
    if (!requestService_) return;

    auto trackingInfo = requestService_->lookupAndRemove(requestId);
    if (!trackingInfo.has_value()) {
        return;
    }

    auto kind = trackingInfo->kind;
    uint16_t addr = trackingInfo->address;
    const TraceId traceId = trackingInfo->traceId;

    if (kind == RequestKind::Poll) {
        if (pollingController_) {
            pollingController_->handleResponse(!response.isError(),
                                                response.rttMs,
                                                response.retryCount(),
                                                response.error);
        }
    }

    switch (response.kind) {
    case ::modbus::session::ModbusResponseKind::NoResponseExpected:
        if (trafficLogController_) {
            trafficLogController_->logBroadcastWriteSuccess(response.retryCount(), traceId);
        }
        break;
    case ::modbus::session::ModbusResponseKind::Success:
        if (controlWidget_) {
            controlWidget_->recordRx(response.rttMs);
        }

        if (kind == RequestKind::Read && trafficLogController_) {
            trafficLogController_->logReadSuccess(response.retryCount(), traceId);
        } else if (kind == RequestKind::Write && trafficLogController_) {
            trafficLogController_->logWriteSuccess(response.retryCount(), traceId);
        }
        break;
    case ::modbus::session::ModbusResponseKind::Error:
        if (response.isBusy()) {
            if (kind != RequestKind::Poll && trafficLogController_) {
                trafficLogController_->logWarning(response.error);
            }
            break;
        }
        if (controlWidget_) {
            controlWidget_->recordError();
        }

        if (kind != RequestKind::Poll && trafficLogController_) {
            trafficLogController_->logRequestError(response.error, response.retryCount(),
                                                   traceId);
        }
        break;
    }

    if (!response.isError() && linked_) {
        const ::modbus::parser::ProtocolType protocolType =
            modeDescriptor(mode_).protocolType;
        emit linkageDataReceived(response.pdu, protocolType, addr);
    }
}

void ModbusPagePresenter::setTrafficLogControllerForTest(TrafficLogController* controller) {
    trafficLogController_ = controller;
}

} // namespace ui::application::modbus

