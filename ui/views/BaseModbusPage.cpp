/**
 * @file BaseModbusPage.cpp
 * @brief Implementation of BaseModbusPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "BaseModbusPage.h"
#include "AppConstants.h"
#include "../common/ISettingsService.h"
#include "../application/modbus/RequestCoordinator.h"
#include "../application/modbus/RequestSubmissionService.h"
#include "../application/modbus/PollingController.h"
#include "../application/modbus/TrafficLogController.h"
#include "../application/modbus/ModbusSessionPresenter.h"
#include "../widgets/BaseConnectionWidget.h"
#include "../widgets/FunctionWidget.h"
#include "../widgets/TrafficMonitorWidget.h"
#include "../widgets/ControlWidget.h"
#include "../widgets/CollapsibleSection.h"
#include "../common/ConnectionAlert.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QPushButton>
#include <QEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <algorithm>

namespace ui::views {

BaseModbusPage::BaseModbusPage(ui::application::modbus::SessionMode mode,
                               ui::common::ISettingsService* settingsService,
                               QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService),
      sessionMode_(mode) {
    sessionPresenter_ = new ui::application::modbus::ModbusSessionPresenter(mode, this);
}

BaseModbusPage::~BaseModbusPage() noexcept = default;

void BaseModbusPage::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    if (sessionPresenter_) {
        ui::application::modbus::ModbusTimingParams params;
        params.timeout = std::chrono::milliseconds(timeoutMs);
        params.retryCount = retries;
        params.retryInterval = std::chrono::milliseconds(retryIntervalMs);
        sessionPresenter_->updateSettings(params);
    }
}

void BaseModbusPage::setLinked(bool linked) {
    linked_ = linked;
    if (sessionPresenter_) {
        sessionPresenter_->setLinked(linked);
    }
}

bool BaseModbusPage::isLinked() const {
    return linked_;
}

void BaseModbusPage::setupUi(ui::widgets::BaseConnectionWidget* connWidget) {
    connectionWidget_ = connWidget;
    const auto descriptor = ui::application::modbus::modeDescriptor(sessionMode_);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);

    mainLayout_->addWidget(connectionWidget_);

    const QString prefix = QString::fromLatin1(descriptor.settingsPrefix);

    functionWidget_ = new widgets::FunctionWidget(settingsService_, this);
    functionWidget_->setSettingsGroup(prefix + QStringLiteral("/standard"));
    functionWidget_->setTransportMode(descriptor.transportUiMode);
    mainLayout_->addWidget(functionWidget_);

    dataGroup_ = new widgets::CollapsibleSection(settingsService_, this);
    dataGroup_->setSettingsKey(prefix + QStringLiteral("/ui/dataMonitorCollapsed"));
    auto dataLayout = new QHBoxLayout(dataGroup_->contentWidget());
    dataLayout->setContentsMargins(4, 0, 4, 4);
    dataLayout->setSpacing(4);

    receiveGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto receiveLayout = new QVBoxLayout(receiveGroup_);
    auto receiveToolbar = new QHBoxLayout();
    copyReceiveButton_ = new QPushButton(receiveGroup_);
    clearReceiveButton_ = new QPushButton(receiveGroup_);
    receiveToolbar->addStretch();
    receiveToolbar->addWidget(copyReceiveButton_);
    receiveToolbar->addWidget(clearReceiveButton_);
    receiveTextEdit_ = new QTextEdit(receiveGroup_);
    receiveTextEdit_->setReadOnly(true);
    receiveTextEdit_->document()->setMaximumBlockCount(app::constants::Values::Ui::kTrafficMonitorMaxBlockCount);
    receiveLayout->addLayout(receiveToolbar);
    receiveLayout->addWidget(receiveTextEdit_);

    sendGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto sendLayout = new QVBoxLayout(sendGroup_);
    auto sendToolbar = new QHBoxLayout();
    copySendButton_ = new QPushButton(sendGroup_);
    clearSendButton_ = new QPushButton(sendGroup_);
    sendToolbar->addStretch();
    sendToolbar->addWidget(copySendButton_);
    sendToolbar->addWidget(clearSendButton_);
    sendTextEdit_ = new QTextEdit(sendGroup_);
    sendTextEdit_->setReadOnly(true);
    sendTextEdit_->document()->setMaximumBlockCount(app::constants::Values::Ui::kTrafficMonitorMaxBlockCount);
    sendLayout->addLayout(sendToolbar);
    sendLayout->addWidget(sendTextEdit_);

    dataLayout->addWidget(receiveGroup_, 1);
    dataLayout->addWidget(sendGroup_, 1);

    trafficMonitor_ = new widgets::TrafficMonitorWidget(settingsService_, this);
    trafficMonitor_->setMinimumHeight(140);
    trafficMonitor_->setSettingsGroup(prefix + QStringLiteral("/traffic"));

    mainLayout_->addWidget(dataGroup_);
    mainLayout_->addWidget(trafficMonitor_);

    controlWidget_ = new widgets::ControlWidget(settingsService_, this);
    controlWidget_->setSettingsGroup(prefix + QStringLiteral("/control"));
    mainLayout_->addWidget(controlWidget_);

    // Initialize services once to unify lifecycle
    requestService_ = new ui::application::modbus::RequestSubmissionService(this);
    pollingController_ = new ui::application::modbus::PollingController(requestService_, this);
    trafficLogController_ = new ui::application::modbus::TrafficLogController(
        trafficMonitor_, pollingController_, this);

    requestCoordinator_ = new ui::application::modbus::RequestCoordinator(
        sessionPresenter_, requestService_, pollingController_,
        trafficLogController_, sessionMode_, this);

    sessionPresenter_->setConnectionWidget(connectionWidget_);
    sessionPresenter_->setControlWidget(controlWidget_);
    sessionPresenter_->setRequestService(requestService_);
    sessionPresenter_->setPollingController(pollingController_);
    sessionPresenter_->setTrafficLogController(trafficLogController_);

    setupCommonConnections();

    mainLayout_->addStretch();

    retranslateUi();
}

void BaseModbusPage::setupCommonConnections() {
    connect(requestService_, &ui::application::modbus::RequestSubmissionService::txCountUpdated,
            controlWidget_, &ui::widgets::ControlWidget::recordTx);

    connect(pollingController_, &ui::application::modbus::PollingController::submitPollRequest,
            this, [this](const ::modbus::base::Pdu& pdu, int slaveId, int requestId) {
                if (sessionPresenter_) {
                    sessionPresenter_->submitRequest(pdu, slaveId, requestId);
                }
            });
    connect(pollingController_, &ui::application::modbus::PollingController::trafficEvent,
            trafficMonitor_, &ui::widgets::TrafficMonitorWidget::appendEvent);
    connect(pollingController_, &ui::application::modbus::PollingController::summaryReady,
            trafficLogController_, &ui::application::modbus::TrafficLogController::logPollSummary);

    if (functionWidget_) {
        connect(functionWidget_, &widgets::FunctionWidget::logMessageRequested,
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
        connect(controlWidget_, &widgets::ControlWidget::logMessageRequested,
            this, [this](const QString& message, bool isError) {
                if (!trafficLogController_) return;
                if (isError) {
                    trafficLogController_->logWarning(message);
                } else {
                    trafficLogController_->logInfo(message);
                }
            });

        connect(controlWidget_, &widgets::ControlWidget::linkToggled, this, [this](bool active) {
            linked_ = active;
            emit linkageToggled(active);
        });

        auto ensureConnected = [this]() {
            if (sessionPresenter_ && sessionPresenter_->isSessionConnected()) {
                return true;
            }
            ui::common::ConnectionAlert::showNotConnected(this);
            return false;
        };
        controlWidget_->setConnectionValidator(ensureConnected);

        connect(controlWidget_, &widgets::ControlWidget::pollRequested,
            this, [this](uint8_t fc, int addr, int qty) {
                if (!requestCoordinator_) return;
                requestCoordinator_->handlePollRequest(fc, addr, qty,
                    controlWidget_->pollingIntervalMs());
            });
    }

    if (sessionPresenter_) {
        connect(sessionPresenter_,
                &ui::application::modbus::ModbusSessionPresenter::linkageSourceDisconnected,
                this, &BaseModbusPage::linkageSourceDisconnected);

        connect(sessionPresenter_,
                &ui::application::modbus::ModbusSessionPresenter::rawFrameReceived,
                this, [this](bool isTx, const QByteArray& data) {
                    if (isTx) {
                        appendSendData(data);
                    } else {
                        appendReceiveData(data);
                    }
                });

        connect(sessionPresenter_,
                &ui::application::modbus::ModbusSessionPresenter::requestFinished,
                this, [this](int requestId, const ::modbus::session::ModbusResponse& response) {
                    if (!requestCoordinator_) return;
                    requestCoordinator_->handleRequestFinished(requestId, response);
                });
    }

    // RequestCoordinator → linkage data forwarding
    if (requestCoordinator_) {
        connect(requestCoordinator_,
                &ui::application::modbus::RequestCoordinator::linkageDataReceived,
                this, [this](const ::modbus::base::Pdu& pdu,
                             modbus::core::parser::ProtocolType protocol,
                             uint16_t addr) {
                    if (linked_) {
                        emit linkageDataReceived(pdu, protocol, addr);
                    }
                });
    }

    if (functionWidget_) {
        connect(functionWidget_, &widgets::FunctionWidget::readRequested,
            this, [this](uint8_t fc, int addr, int qty, int slaveId) {
                if (!requestCoordinator_) return;
                requestCoordinator_->handleReadRequest(fc, addr, qty, slaveId);
            });

        connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
            this, [this](uint8_t fc, int addr, const QString& dataStr,
                         const QString& fmt, int slaveId) {
                if (!requestCoordinator_) return;
                requestCoordinator_->handleWriteRequest(fc, addr, dataStr, fmt, slaveId,
                                                        functionWidget_->getQuantity());
            });

        connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
            this, [this](const QByteArray& data) {
                if (!requestCoordinator_) return;
                requestCoordinator_->handleRawSendRequest(data);
            });
    }

    connect(clearReceiveButton_, &QPushButton::clicked, this, [this]() {
        lastReceiveFrame_.clear();
        if (receiveTextEdit_) receiveTextEdit_->clear();
    });
    connect(clearSendButton_, &QPushButton::clicked, this, [this]() {
        lastSendFrame_.clear();
        if (sendTextEdit_) sendTextEdit_->clear();
    });
    connect(copyReceiveButton_, &QPushButton::clicked, this, [this]() {
        if (!receiveTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(receiveTextEdit_->toPlainText());
    });
    connect(copySendButton_, &QPushButton::clicked, this, [this]() {
        if (!sendTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(sendTextEdit_->toPlainText());
    });
}

QString BaseModbusPage::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void BaseModbusPage::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void BaseModbusPage::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void BaseModbusPage::refreshReceiveDisplay() {
    if (!receiveTextEdit_) return;
    if (lastReceiveFrame_.isEmpty()) {
        receiveTextEdit_->clear();
        return;
    }
    const bool hex = true;
    receiveTextEdit_->setPlainText(tr("[%1] %2")
                                       .arg(tr("RX"))
                                       .arg(formatData(lastReceiveFrame_, hex)));
}

void BaseModbusPage::refreshSendDisplay() {
    if (!sendTextEdit_) return;
    if (lastSendFrame_.isEmpty()) {
        sendTextEdit_->clear();
        return;
    }
    const bool hex = true;
    sendTextEdit_->setPlainText(tr("[%1] %2")
                                     .arg(tr("TX"))
                                     .arg(formatData(lastSendFrame_, hex)));
}

void BaseModbusPage::retranslateUi() {
    if (dataGroup_) dataGroup_->setTitle(tr("Data Monitor"));
    if (receiveGroup_) receiveGroup_->setTitle(tr("Receive Data"));
    if (sendGroup_) sendGroup_->setTitle(tr("Send Data"));
    if (copyReceiveButton_) copyReceiveButton_->setText(tr("Copy"));
    if (copySendButton_) copySendButton_->setText(tr("Copy"));
    if (clearReceiveButton_) clearReceiveButton_->setText(tr("Clear"));
    if (clearSendButton_) clearSendButton_->setText(tr("Clear"));
    refreshReceiveDisplay();
    refreshSendDisplay();
}

void BaseModbusPage::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

} // namespace ui::views
