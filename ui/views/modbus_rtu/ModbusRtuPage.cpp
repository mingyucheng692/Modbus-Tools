/**
 * @file ModbusRtuPage.cpp
 * @brief Implementation of ModbusRtuPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusRtuPage.h"
#include "AppConstants.h"
#include "../../common/ISettingsService.h"
#include "../../application/modbus/RequestSubmissionService.h"
#include "../../application/modbus/PollingController.h"
#include "../../application/modbus/TrafficLogController.h"
#include "../../application/modbus/ModbusSessionPresenter.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <algorithm>

namespace ui::views::modbus_rtu {

ModbusRtuPage::ModbusRtuPage(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    sessionPresenter_ = new ui::application::modbus::ModbusSessionPresenter(
        ui::application::modbus::SessionMode::Rtu, this);
    setupUi();
}

ModbusRtuPage::~ModbusRtuPage() {
}

void ModbusRtuPage::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    if (sessionPresenter_) {
        ui::application::modbus::ModbusTimingParams params;
        params.timeout = std::chrono::milliseconds(timeoutMs);
        params.retryCount = retries;
        params.retryInterval = std::chrono::milliseconds(retryIntervalMs);
        sessionPresenter_->updateSettings(params);
    }
}

void ModbusRtuPage::setLinked(bool linked) {
    linked_ = linked;
    if (sessionPresenter_) {
        sessionPresenter_->setLinked(linked);
    }
}

bool ModbusRtuPage::isLinked() const {
    return linked_;
}

void ModbusRtuPage::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);

    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup("modbus/rtu/serial");
    mainLayout_->addWidget(connectionWidget_);

    functionWidget_ = new widgets::FunctionWidget(settingsService_, this);
    functionWidget_->setSettingsGroup("modbus/rtu/standard");
    functionWidget_->setTcpMode(false);
    mainLayout_->addWidget(functionWidget_);

    dataGroup_ = new widgets::CollapsibleSection(settingsService_, this);
    dataGroup_->setSettingsKey("modbus/rtu/ui/dataMonitorCollapsed");
    auto dataLayout = new QHBoxLayout(dataGroup_->contentWidget());
    dataLayout->setContentsMargins(4, 0, 4, 4);
    dataLayout->setSpacing(4);

    receiveGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto receiveLayout = new QVBoxLayout(receiveGroup_);
    auto receiveToolbar = new QHBoxLayout();
    receiveHexCheck_ = new QCheckBox(receiveGroup_);
    receiveHexCheck_->setChecked(true);
    receiveHexCheck_->setVisible(false);
    copyReceiveButton_ = new QPushButton(receiveGroup_);
    clearReceiveButton_ = new QPushButton(receiveGroup_);
    receiveToolbar->addWidget(receiveHexCheck_);
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
    sendHexCheck_ = new QCheckBox(sendGroup_);
    sendHexCheck_->setChecked(true);
    sendHexCheck_->setVisible(false);
    copySendButton_ = new QPushButton(sendGroup_);
    clearSendButton_ = new QPushButton(sendGroup_);
    sendToolbar->addWidget(sendHexCheck_);
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
    trafficMonitor_->setSettingsGroup("modbus/rtu/traffic");

    mainLayout_->addWidget(dataGroup_);
    mainLayout_->addWidget(trafficMonitor_);

    controlWidget_ = new widgets::ControlWidget(settingsService_, this);
    controlWidget_->setSettingsGroup("modbus/rtu/control");
    mainLayout_->addWidget(controlWidget_);

    sessionPresenter_->setConnectionWidget(connectionWidget_);
    sessionPresenter_->setControlWidget(controlWidget_);

    if (functionWidget_ && trafficLogController_) {
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

    if (controlWidget_ && trafficLogController_) {
        connect(controlWidget_, &widgets::ControlWidget::logMessageRequested,
            this, [this](const QString& message, bool isError) {
                if (!trafficLogController_) return;
                if (isError) {
                    trafficLogController_->logWarning(message);
                } else {
                    trafficLogController_->logInfo(message);
                }
            });
    }

    connect(controlWidget_, &widgets::ControlWidget::linkToggled, this, [this](bool active){
        linked_ = active;
        emit linkageToggled(active);
    });

    connect(sessionPresenter_,
            &ui::application::modbus::ModbusSessionPresenter::linkageSourceDisconnected,
            this, &ModbusRtuPage::linkageSourceDisconnected);

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
                if (!requestService_) return;

                auto trackingInfo = requestService_->lookupAndRemove(requestId);
                if (!trackingInfo.has_value()) {
                    return;
                }

                auto kind = trackingInfo->kind;
                uint16_t addr = trackingInfo->address;

                if (response.isSuccess && linked_) {
                    emit linkageDataReceived(response.pdu,
                                             modbus::core::parser::ProtocolType::Rtu,
                                             addr);
                }
            });

    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
        [this](const io::SerialConfig& config) {
            spdlog::info("ModbusRtuPage: Connect requested to {}", config.portName.toStdString());

            if (!trafficLogController_) {
                trafficLogController_ = new ui::application::modbus::TrafficLogController(
                    trafficMonitor_, nullptr, this);
            }

            sessionPresenter_->setTrafficLogController(trafficLogController_);

            modbus::base::ModbusConfig modbusConfig;
            modbusConfig.mode = modbus::base::ModbusMode::RTU;
            modbusConfig.portName = config.portName;
            modbusConfig.baudRate = config.baudRate;
            modbusConfig.dataBits = config.dataBits;
            modbusConfig.stopBits = static_cast<int>(config.stopBits);
            modbusConfig.parity = static_cast<int>(config.parity);
            modbusConfig.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
            modbusConfig.timeoutMs = 1000;
            modbusConfig.retries = 0;
            modbusConfig.retryIntervalMs = 200;

            requestService_ = new ui::application::modbus::RequestSubmissionService(this);
            connect(requestService_, &ui::application::modbus::RequestSubmissionService::txCountUpdated,
                    controlWidget_, &ui::widgets::ControlWidget::recordTx);

            sessionPresenter_->setRequestService(requestService_);

            pollingController_ = new ui::application::modbus::PollingController(requestService_, this);
            connect(pollingController_, &ui::application::modbus::PollingController::submitPollRequest,
                    this, [this](const ::modbus::base::Pdu& pdu, int slaveId, int requestId) {
                        sessionPresenter_->submitRequest(pdu, slaveId, requestId);
                    });
            connect(pollingController_, &ui::application::modbus::PollingController::trafficEvent,
                    trafficMonitor_, &ui::widgets::TrafficMonitorWidget::appendEvent);
            connect(pollingController_, &ui::application::modbus::PollingController::summaryReady,
                    trafficLogController_, &ui::application::modbus::TrafficLogController::logPollSummary);

            sessionPresenter_->setPollingController(pollingController_);
            trafficLogController_->setPollingController(pollingController_);

            sessionPresenter_->connectRtu(config, modbusConfig);
    });

    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("ModbusRtuPage: Disconnect requested");
            sessionPresenter_->requestDisconnect();
    });

    auto ensureConnected = [this]() {
        if (sessionPresenter_ && sessionPresenter_->isSessionConnected()) {
            return true;
        }
        ui::common::ConnectionAlert::showNotConnected(this);
        return false;
    };

    controlWidget_->setConnectionValidator(ensureConnected);

    connect(functionWidget_, &widgets::FunctionWidget::readRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty, int slaveId) {
            if (!ensureConnected()) return;

            if (!requestService_) {
                trafficLogController_->logError(tr("Error: Request service not available"));
                return;
            }

            ui::application::modbus::PollSpec spec;
            spec.functionCode = fc;
            spec.startAddress = static_cast<uint16_t>(addr);
            spec.quantity = static_cast<uint16_t>(qty);
            spec.slaveId = static_cast<uint8_t>(slaveId);

            auto result = requestService_->buildReadRequest(spec);
            if (!result.ok) {
                trafficLogController_->logError(tr("Error: %1").arg(result.error));
                return;
            }

            trafficLogController_->logSendingReadRequest(fc, addr, qty, slaveId);

            sessionPresenter_->submitRequest(result.pdu, slaveId, result.requestId);
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this, ensureConnected](uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId) {
            if (!ensureConnected()) return;

            if (!requestService_) {
                trafficLogController_->logError(tr("Error: Request service not available"));
                return;
            }

            int quantity = functionWidget_->getQuantity();

            auto result = requestService_->buildWriteRequest(fc, addr, dataStr, fmt, slaveId, quantity);
            if (!result.ok) {
                trafficLogController_->logError(tr("Error: %1").arg(result.error));
                return;
            }

            trafficLogController_->logSendingWriteRequest(fc, addr, dataStr, slaveId);

            sessionPresenter_->submitRequest(result.pdu, slaveId, result.requestId);
    });

    connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
        [this, ensureConnected](const QByteArray& data) {
            if (!ensureConnected()) return;
            if (!requestService_ || !requestService_->validateRawData(data)) return;

            trafficLogController_->logSendingRawData(data);

            sessionPresenter_->sendRaw(data);
    });

    connect(controlWidget_, &widgets::ControlWidget::pollRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty) {
            if (!ensureConnected()) return;
            if (!pollingController_) return;
            pollingController_->setSessionConnected(sessionPresenter_->isSessionConnected());
            pollingController_->setPollingInterval(controlWidget_->pollingIntervalMs());
            ui::application::modbus::PollSpec spec;
            spec.functionCode = fc;
            spec.startAddress = static_cast<uint16_t>(addr);
            spec.quantity = static_cast<uint16_t>(qty);
            spec.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
            pollingController_->handlePollRequest(spec);
    });

    connect(clearReceiveButton_, &QPushButton::clicked, [this]() {
        lastReceiveFrame_.clear();
        if (receiveTextEdit_) receiveTextEdit_->clear();
    });
    connect(clearSendButton_, &QPushButton::clicked, [this]() {
        lastSendFrame_.clear();
        if (sendTextEdit_) sendTextEdit_->clear();
    });
    connect(copyReceiveButton_, &QPushButton::clicked, [this]() {
        if (!receiveTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(receiveTextEdit_->toPlainText());
    });
    connect(copySendButton_, &QPushButton::clicked, [this]() {
        if (!sendTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(sendTextEdit_->toPlainText());
    });
    connect(receiveHexCheck_, &QCheckBox::toggled, this, [this]() {
        refreshReceiveDisplay();
    });
    connect(sendHexCheck_, &QCheckBox::toggled, this, [this]() {
        refreshSendDisplay();
    });

    mainLayout_->addStretch();

    retranslateUi();
}

QString ModbusRtuPage::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void ModbusRtuPage::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void ModbusRtuPage::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void ModbusRtuPage::refreshReceiveDisplay() {
    if (!receiveTextEdit_) return;
    if (lastReceiveFrame_.isEmpty()) {
        receiveTextEdit_->clear();
        return;
    }
    bool hex = receiveHexCheck_ ? receiveHexCheck_->isChecked() : true;
    receiveTextEdit_->setPlainText(QStringLiteral("[%1] %2")
                                       .arg(QStringLiteral("RX"))
                                       .arg(formatData(lastReceiveFrame_, hex)));
}

void ModbusRtuPage::refreshSendDisplay() {
    if (!sendTextEdit_) return;
    if (lastSendFrame_.isEmpty()) {
        sendTextEdit_->clear();
        return;
    }
    bool hex = sendHexCheck_ ? sendHexCheck_->isChecked() : true;
    sendTextEdit_->setPlainText(QStringLiteral("[%1] %2")
                                     .arg(QStringLiteral("TX"))
                                     .arg(formatData(lastSendFrame_, hex)));
}

void ModbusRtuPage::retranslateUi() {
    if (dataGroup_) dataGroup_->setTitle(tr("Data Monitor"));
    if (receiveGroup_) receiveGroup_->setTitle(tr("Receive Data"));
    if (sendGroup_) sendGroup_->setTitle(tr("Send Data"));
    if (copyReceiveButton_) copyReceiveButton_->setText(tr("Copy"));
    if (copySendButton_) copySendButton_->setText(tr("Copy"));
    if (clearReceiveButton_) clearReceiveButton_->setText(tr("Clear"));
    if (clearSendButton_) clearSendButton_->setText(tr("Clear"));
}

void ModbusRtuPage::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

} // namespace ui::views::modbus_rtu
