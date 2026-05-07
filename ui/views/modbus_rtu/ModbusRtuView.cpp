/**
 * @file ModbusRtuView.cpp
 * @brief Implementation of ModbusRtuView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusRtuView.h"
#include "AppConstants.h"
#include "../../common/ISettingsService.h"
#include "../../application/modbus/RequestSubmissionService.h"
#include "../../application/modbus/PollingController.h"
#include "../../application/modbus/TrafficLogController.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "modbus/factory/ModbusFactory.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QEvent>
#include <QEventLoop>
#include <QClipboard>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <QMetaObject>
#include <QPointer>
#include <algorithm>
#include <limits>

namespace ui::views::modbus_rtu {

ModbusRtuView::ModbusRtuView(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
}

ModbusRtuView::~ModbusRtuView() {
    releaseStack();
}

void ModbusRtuView::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    timeoutMs_ = timeoutMs;
    retries_ = retries;
    retryIntervalMs_ = retryIntervalMs;
    if (worker_) {
        currentConfig_.timeoutMs = timeoutMs_;
        currentConfig_.retries = retries_;
        currentConfig_.retryIntervalMs = retryIntervalMs_;
        worker_->updateConfig(currentConfig_);
    }
}

void ModbusRtuView::setLinked(bool linked) {
    linked_ = linked;
    if (controlWidget_) {
        controlWidget_->setLinked(linked);
    }
}

bool ModbusRtuView::isLinked() const {
    return linked_;
}

void ModbusRtuView::setupUi() {
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
    
    if (functionWidget_ && trafficMonitor_) {
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

    if (controlWidget_ && trafficMonitor_) {
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

    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked, 
        [this](const io::SerialConfig& config) {
            spdlog::info("ModbusRtuView: Connect requested to {}", config.portName.toStdString());

            if (!trafficLogController_) {
                trafficLogController_ = new ui::application::modbus::TrafficLogController(
                    trafficMonitor_, nullptr, this);
            }

            trafficLogController_->logConnectionInfo(tr("Opening %1...").arg(config.portName));

            releaseStack();
            const quint64 generation = connectionGeneration_;

            modbus::base::ModbusConfig modbusConfig;
            modbusConfig.mode = modbus::base::ModbusMode::RTU;
            modbusConfig.portName = config.portName;
            modbusConfig.baudRate = config.baudRate;
            modbusConfig.dataBits = config.dataBits;
            modbusConfig.stopBits = static_cast<int>(config.stopBits);
            modbusConfig.parity = static_cast<int>(config.parity);
            modbusConfig.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
            modbusConfig.timeoutMs = timeoutMs_;
            modbusConfig.retries = retries_;
            modbusConfig.retryIntervalMs = retryIntervalMs_;

            modbus::factory::ModbusFactory factory;
            auto stack = factory.createStack(modbusConfig);
            if (!stack.worker || !stack.thread) {
                trafficLogController_->logError(tr("Failed to create Modbus stack"));
                return;
            }

            currentConfig_ = modbusConfig;
            channel_ = std::move(stack.channel);
            client_ = std::move(stack.client);
            worker_ = std::move(stack.worker);
            workerThread_ = std::move(stack.thread);

            requestService_ = new ui::application::modbus::RequestSubmissionService(this);
            connect(requestService_, &ui::application::modbus::RequestSubmissionService::txCountUpdated,
                    controlWidget_, &ui::widgets::ControlWidget::recordTx);

            pollingController_ = new ui::application::modbus::PollingController(requestService_, this);
            connect(pollingController_, &ui::application::modbus::PollingController::submitPollRequest,
                    this, [this](const ::modbus::base::Pdu& pdu, int slaveId, int requestId) {
                        if (worker_) worker_->submit(pdu, slaveId, requestId);
                    });
            connect(pollingController_, &ui::application::modbus::PollingController::trafficEvent,
                    trafficMonitor_, &ui::widgets::TrafficMonitorWidget::appendEvent);

            trafficLogController_->setPollingController(pollingController_);

            QPointer<ModbusRtuView> self(this);
            channel_->setMonitor([self, generation](bool isTx, const QByteArray& data) {
                if (!self) return;
                QMetaObject::invokeMethod(self, [self, generation, isTx, data]() {
                    if (!self) return;
                    if (generation != self->connectionGeneration_) return;
                    if (isTx) {
                        self->trafficLogController_->logRawFrame(
                            ui::common::TrafficDirection::Tx, data);
                        self->appendSendData(data);
                    } else {
                        self->trafficLogController_->logRawFrame(
                            ui::common::TrafficDirection::Rx, data);
                        self->appendReceiveData(data);
                    }
                }, Qt::QueuedConnection);
            });
            channel_->addStateHandler([self, generation](io::ChannelState state) {
                if (!self) return;
                QMetaObject::invokeMethod(self, [self, generation, state]() {
                    if (!self) return;
                    if (generation != self->connectionGeneration_) return;
                    const bool connected = state == io::ChannelState::Open;
                    self->rtuSessionConnected_ = connected;
                    self->connectionWidget_->setConnected(connected);
                    if (!connected) {
                        self->controlWidget_->setPollingEnabled(false);
                    }
                }, Qt::QueuedConnection);
            });

            connect(worker_.get(), &modbus::dispatch::ModbusWorker::connectFinished, this,
                [this, self, generation](bool ok, const QString& error) {
                    if (!self) return;
                    if (generation != connectionGeneration_) return;
                    rtuSessionConnected_ = ok;
                    if (!ok) {
                        if (pollingController_) pollingController_->stopPoll();
                    }
                    if (ok) {
                        connectionWidget_->setConnected(true);
                        self->trafficLogController_->logConnectionInfo(tr("Connected"));
                    } else {
                        connectionWidget_->setConnected(false);
                        self->trafficLogController_->logConnectionInfo(tr("Connection failed: %1").arg(error));
                    }
                }, Qt::QueuedConnection);

            connect(worker_.get(), &modbus::dispatch::ModbusWorker::requestFinished, this,
                [this, self, generation](int requestId, const modbus::session::ModbusResponse& response) {
                    if (!self) return;
                    if (generation != connectionGeneration_) return;
                    if (!requestService_) return;

                    auto trackingInfo = requestService_->lookupAndRemove(requestId);
                    if (!trackingInfo.has_value()) {
                        return;
                    }

                    auto kind = trackingInfo->kind;
                    uint16_t addr = trackingInfo->address;
                    auto startTime = trackingInfo->startTime;

                    if (kind == ui::application::modbus::RequestKind::Poll) {
                        pollingController_->handleResponse(response.isSuccess,
                                                           response.rttMs,
                                                           response.retryCount,
                                                           response.error);
                    }

                    // 分流设计：仅在成功时通过底层测量的精�?RTT 更新统计
                    if (response.isSuccess && response.noResponseExpected) {
                        trafficLogController_->logBroadcastWriteSuccess(response.retryCount);
                    } else if (response.isSuccess) {
                        controlWidget_->recordRx(response.rttMs);

                        if (kind == ui::application::modbus::RequestKind::Read) {
                            trafficLogController_->logReadSuccess(response.retryCount);
                        } else if (kind == ui::application::modbus::RequestKind::Write) {
                            trafficLogController_->logWriteSuccess(response.retryCount);
                        }

                        if (linked_) {
                            emit linkageDataReceived(response.pdu, modbus::core::parser::ProtocolType::Rtu, addr);
                        }
                    } else {
                        controlWidget_->recordError();

                        if (kind != ui::application::modbus::RequestKind::Poll) {
                            trafficLogController_->logRequestError(response.error, response.retryCount);
                        }
                    }

                }, Qt::QueuedConnection);

            worker_->start();
            worker_->requestConnect();
    });

    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("ModbusRtuView: Disconnect requested");
            trafficLogController_->logConnectionInfo(tr("Disconnected"));
            releaseStack();
            connectionWidget_->setConnected(false);
    });

    auto ensureConnected = [this]() {
        if (worker_ && rtuSessionConnected_) {
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

            auto result = requestService_->buildReadRequest(fc, addr, qty, slaveId);
            if (!result.ok) {
                trafficLogController_->logError(tr("Error: %1").arg(result.error));
                return;
            }

            trafficLogController_->logSendingReadRequest(fc, addr, qty, slaveId);

            worker_->submit(result.pdu, slaveId, result.requestId);
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

            worker_->submit(result.pdu, slaveId, result.requestId);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
        [this, ensureConnected](const QByteArray& data) {
            if (!ensureConnected()) return;
            if (!requestService_ || !requestService_->validateRawData(data)) return;

            trafficLogController_->logSendingRawData(data);

            worker_->sendRaw(data);
    });

    connect(controlWidget_, &widgets::ControlWidget::pollRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty) {
            if (!ensureConnected()) return;
            if (!pollingController_) return;
            pollingController_->setSessionConnected(rtuSessionConnected_);
            pollingController_->setPollingInterval(controlWidget_->pollingIntervalMs());
            int slaveId = functionWidget_->getSlaveId();
            pollingController_->handlePollRequest(fc, addr, qty, slaveId);
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

QString ModbusRtuView::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void ModbusRtuView::releaseStack() {
    if (pollingController_) pollingController_->reset();
    ++connectionGeneration_;
    rtuSessionConnected_ = false;
    if (connectionWidget_) {
        connectionWidget_->setConnected(false);
    }
    const bool wasLinked = linked_;
    linked_ = false;
    if (controlWidget_) {
        controlWidget_->setLinked(false);
        controlWidget_->setPollingEnabled(false);
    }
    if (wasLinked) {
        emit linkageSourceDisconnected();
    }
    
    auto channel = std::move(channel_);
    auto client = std::move(client_);
    auto worker = std::move(worker_);
    auto thread = std::move(workerThread_);
    if (!worker && !client && !channel && !thread) {
        return;
    }

    if (worker) {
        QEventLoop wait;
        QObject::connect(worker.get(), &modbus::dispatch::ModbusWorker::stopped,
                         &wait, &QEventLoop::quit);
        worker->stop();
        wait.exec();
    } else if (thread && thread->isRunning()) {
        thread->requestInterruption();
        thread->quit();
    }
    worker.reset();
    client.reset();
    if (thread && thread->isRunning()) {
        thread->requestInterruption();
        thread->quit();
        thread->wait(2000);
    }
    channel.reset();
    thread.reset();
}


void ModbusRtuView::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void ModbusRtuView::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void ModbusRtuView::refreshReceiveDisplay() {
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

void ModbusRtuView::refreshSendDisplay() {
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

void ModbusRtuView::retranslateUi() {
    if (dataGroup_) dataGroup_->setTitle(tr("Data Monitor"));
    if (receiveGroup_) receiveGroup_->setTitle(tr("Receive Data"));
    if (sendGroup_) sendGroup_->setTitle(tr("Send Data"));
    if (receiveHexCheck_) receiveHexCheck_->setText(tr("HEX"));
    if (sendHexCheck_) sendHexCheck_->setText(tr("HEX"));
    if (copyReceiveButton_) copyReceiveButton_->setText(tr("Copy"));
    if (copySendButton_) copySendButton_->setText(tr("Copy"));
    if (clearReceiveButton_) clearReceiveButton_->setText(tr("Clear"));
    if (clearSendButton_) clearSendButton_->setText(tr("Clear"));
    refreshReceiveDisplay();
    refreshSendDisplay();
}

void ModbusRtuView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::modbus_rtu
