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
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TrafficEvent.h"
#include "modbus/factory/ModbusFactory.h"
#include "modbus/base/ModbusDataHelper.h"
#include "modbus/base/ModbusPduBuilder.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <QMetaObject>
#include <QPointer>
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
                if (isError) {
                    trafficMonitor_->appendWarning(message);
                } else {
                    trafficMonitor_->appendInfo(message);
                }
            });
    }

    if (controlWidget_ && trafficMonitor_) {
        connect(controlWidget_, &widgets::ControlWidget::logMessageRequested,
            this, [this](const QString& message, bool isError) {
                if (isError) {
                    trafficMonitor_->appendWarning(message);
                } else {
                    trafficMonitor_->appendInfo(message);
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
            appendConnectionInfo(tr("Opening %1...").arg(config.portName));

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
                trafficMonitor_->appendInfo(tr("Failed to create Modbus stack"));
                return;
            }

            currentConfig_ = modbusConfig;
            channel_ = std::move(stack.channel);
            client_ = std::move(stack.client);
            worker_ = std::move(stack.worker);
            workerThread_ = std::move(stack.thread);

            QPointer<ModbusRtuView> self(this);
            channel_->setMonitor([self, generation](bool isTx, const QByteArray& data) {
                if (!self) return;
                QMetaObject::invokeMethod(self, [self, generation, isTx, data]() {
                    if (!self) return;
                    if (generation != self->connectionGeneration_) return;
                    if (isTx) {
                        const bool allowRawFrameLog = !self->suppressPollTrafficLog_
                            || (self->trafficMonitor_ && self->trafficMonitor_->isRawFramesModeEnabled());
                        if (allowRawFrameLog) {
                            ui::common::TrafficEvent event;
                            event.direction = ui::common::TrafficDirection::Tx;
                            event.requestType = ui::common::TrafficRequestType::Unknown;
                            event.isPoll = self->suppressPollTrafficLog_;
                            event.payload = data;
                            self->trafficMonitor_->appendEvent(event);
                        }
                        self->appendSendData(data);
                    } else {
                        const bool allowRawFrameLog = !self->suppressPollTrafficLog_
                            || (self->trafficMonitor_ && self->trafficMonitor_->isRawFramesModeEnabled());
                        if (allowRawFrameLog) {
                            ui::common::TrafficEvent event;
                            event.direction = ui::common::TrafficDirection::Rx;
                            event.requestType = ui::common::TrafficRequestType::Unknown;
                            event.isPoll = self->suppressPollTrafficLog_;
                            event.payload = data;
                            self->trafficMonitor_->appendEvent(event);
                        }
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
                        pollInFlight_ = false;
                        suppressPollTrafficLog_ = false;
                    }
                    if (ok) {
                        connectionWidget_->setConnected(true);
                        self->appendConnectionInfo(tr("Connected"));
                    } else {
                        connectionWidget_->setConnected(false);
                        self->appendConnectionInfo(tr("Connection failed: %1").arg(error));
                    }
                }, Qt::QueuedConnection);

            connect(worker_.get(), &modbus::dispatch::ModbusWorker::requestFinished, this,
                [this, self, generation](int requestId, const modbus::session::ModbusResponse& response) {
                    if (!self) return;
                    if (generation != connectionGeneration_) return;
                    auto itStart = requestStart_.find(requestId);
                    auto itKind = requestKinds_.find(requestId);
                    auto itAddr = requestAddrs_.find(requestId);
                    if (itStart == requestStart_.end() || itKind == requestKinds_.end() || itAddr == requestAddrs_.end()) {
                        return;
                    }

                    if (itKind->second == RequestKind::Poll) {
                        pollInFlight_ = false;
                        suppressPollTrafficLog_ = false;
                        handlePollCompletion(response.isSuccess, response.rttMs, response.error);
                    }

                    // 分流设计：仅在成功时通过底层测量的精确 RTT 更新统计
                    if (response.isSuccess && response.noResponseExpected) {
                        trafficMonitor_->appendInfo(tr("Success: Broadcast write sent, no response expected"));
                    } else if (response.isSuccess) {
                        // 更新成功统计 (RX + 1, 更新 RTT)
                        controlWidget_->recordRx(response.rttMs);

                        if (itKind->second == RequestKind::Read) {
                            trafficMonitor_->appendInfo(tr("Success: Response received"));
                        } else if (itKind->second == RequestKind::Write) {
                            trafficMonitor_->appendInfo(tr("Success: Write confirmed"));
                        }

                        // Emit linkage signal for Frame Analyzer if linked
                        if (linked_) {
                            emit linkageDataReceived(response.pdu, modbus::core::parser::ProtocolType::Rtu, itAddr->second);
                        }
                    } else {
                        // 失败路径：仅更新 Error 计数，跳过 RTT 统计防止均值偏移
                        controlWidget_->recordError();

                        if (itKind->second != RequestKind::Poll) {
                            trafficMonitor_->appendInfo(tr("Error: %1").arg(response.error));
                        }
                    }

                    requestStart_.erase(itStart);
                    requestKinds_.erase(itKind);
                    requestAddrs_.erase(itAddr);
                }, Qt::QueuedConnection);

            worker_->start();
            worker_->requestConnect();
    });

    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("ModbusRtuView: Disconnect requested");
            appendConnectionInfo(tr("Disconnected"));
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
            
            using namespace modbus::base;
            auto result = ModbusPduBuilder::buildReadRequest(static_cast<FunctionCode>(fc), addr, qty);
            if (!result.isOk()) {
                trafficMonitor_->appendInfo(tr("Error: %1").arg(result.error));
                return;
            }
            Pdu request = *result.pdu;

            trafficMonitor_->appendInfo(tr("Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4")
                .arg(fc).arg(addr).arg(qty).arg(slaveId));

            int requestId = nextRequestId();
            requestStart_[requestId] = std::chrono::steady_clock::now();
            requestKinds_[requestId] = RequestKind::Read;
            requestAddrs_[requestId] = static_cast<uint16_t>(addr);
            
            controlWidget_->recordTx(); // 提交时增加 TX 计数
            worker_->submit(request, slaveId, requestId);
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this, ensureConnected](uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId) {
            if (!ensureConnected()) return;

            using namespace modbus::base;
            
            PduBuildResult result = PduBuildResult::fail(tr("Unsupported function code"));
            QString trimmed = dataStr.trimmed();
            using modbus::base::ModbusDataHelper;

            if (fc == 0x05) {
                bool coilOn = false;
                if (fmt == "Decimal") {
                    bool ok = false;
                    int value = trimmed.toInt(&ok);
                    if (!ok || (value != 0 && value != 1)) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid decimal value for 0x05"));
                        return;
                    }
                    coilOn = (value != 0);
                } else if (fmt == "Binary") {
                    QString cleaned = trimmed;
                    cleaned.remove(QRegularExpression("[^0-1]"));
                    if (cleaned.isEmpty() || cleaned.size() > 1) {
                         trafficMonitor_->appendInfo(tr("Error: Invalid binary value for 0x05 (expected 0 or 1)"));
                         return;
                    }
                    coilOn = (cleaned == "1");
                } else {
                    QByteArray bytes = ModbusDataHelper::parseHex(trimmed);
                    if (bytes.isEmpty()) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x05"));
                        return;
                    }
                    if (bytes.size() == 1) {
                        uint8_t raw = static_cast<uint8_t>(bytes[0]);
                        if (raw != 0x00 && raw != 0x01) {
                            trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x05"));
                            return;
                        }
                        coilOn = raw != 0x00;
                    } else {
                        uint16_t raw = (static_cast<uint8_t>(bytes[0]) << 8) | static_cast<uint8_t>(bytes[1]);
                        if (raw != 0x0000 && raw != 0xFF00) {
                            trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x05"));
                            return;
                        }
                        coilOn = raw == 0xFF00;
                    }
                }
                result = ModbusPduBuilder::buildWriteSingleCoil(addr, coilOn);
            } else if (fc == 0x06) {
                if (trimmed.isEmpty()) {
                    trafficMonitor_->appendInfo(tr("Error: Empty value for 0x06"));
                    return;
                }
                if (fmt == "Decimal") {
                    bool ok = false;
                    uint value = trimmed.toUInt(&ok, 10);
                    if (!ok || value > 65535) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid decimal value for 0x06"));
                        return;
                    }
                    result = ModbusPduBuilder::buildWriteSingleRegister(addr, static_cast<uint16_t>(value));
                } else if (fmt == "Binary") {
                    trafficMonitor_->appendInfo(tr("Error: Binary format not supported for registers (0x06)"));
                    return;
                } else {
                    QByteArray bytes = ModbusDataHelper::parseHex(trimmed);
                    if (bytes.isEmpty() || bytes.size() > 2) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x06"));
                        return;
                    }
                    uint16_t val = 0;
                    if (bytes.size() == 1) val = static_cast<uint8_t>(bytes[0]);
                    else val = (static_cast<uint8_t>(bytes[0]) << 8) | static_cast<uint8_t>(bytes[1]);
                    result = ModbusPduBuilder::buildWriteSingleRegister(addr, val);
                }
            } else if (fc == 0x0F) {
                QByteArray bytes;
                int quantity = functionWidget_->getQuantity();
                if (quantity <= 0) {
                    trafficMonitor_->appendInfo(tr("Error: Invalid quantity for 0x0F"));
                    return;
                }

                if (fmt == "Binary") {
                    QString bits = trimmed;
                    bits.remove(QRegularExpression("[^0-1]"));
                    if (bits.size() != quantity) {
                        trafficMonitor_->appendInfo(tr("Error: Binary bit count (%1) does not match Quantity (%2)")
                            .arg(bits.size()).arg(quantity));
                        return;
                    }
                    bytes = ModbusDataHelper::parseBinary(bits);
                } else if (fmt == "Hex") {
                    bytes = ModbusDataHelper::parseHex(trimmed);
                } else {
                    trafficMonitor_->appendInfo(tr("Error: 0x0F requires Hex or Binary data"));
                    return;
                }
                result = ModbusPduBuilder::buildWriteMultipleCoils(addr, quantity, bytes);
            } else if (fc == 0x10) {
                if (trimmed.isEmpty()) {
                    trafficMonitor_->appendInfo(tr("Error: Empty value for 0x10"));
                    return;
                }
                int quantity = functionWidget_->getQuantity();
                if (quantity <= 0) {
                    trafficMonitor_->appendInfo(tr("Error: Invalid quantity for 0x10"));
                    return;
                }
                QByteArray payload;
                if (fmt == "Decimal") {
                    bool okList = false;
                    payload = ModbusDataHelper::parseDecimalList(trimmed, okList);
                    if (!okList) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid decimal list for 0x10"));
                        return;
                    }
                } else {
                    payload = ModbusDataHelper::parseHex(trimmed);
                    if (payload.isEmpty() || (payload.size() % 2 != 0)) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x10"));
                        return;
                    }
                }
                result = ModbusPduBuilder::buildWriteMultipleRegisters(addr, quantity, payload);
            }

            if (!result.isOk()) {
                trafficMonitor_->appendInfo(tr("Error: %1").arg(result.error));
                return;
            }
            
            Pdu request = *result.pdu;

            trafficMonitor_->appendInfo(tr("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
                .arg(fc).arg(addr).arg(dataStr).arg(slaveId));

            int requestId = nextRequestId();
            requestStart_[requestId] = std::chrono::steady_clock::now();
            requestKinds_[requestId] = RequestKind::Write;
            requestAddrs_[requestId] = static_cast<uint16_t>(addr);

            controlWidget_->recordTx(); // 提交时增加 TX 计数
            worker_->submit(request, slaveId, requestId);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
        [this, ensureConnected](const QByteArray& data) {
            if (!ensureConnected()) return;
            if (data.isEmpty()) return;
            
            trafficMonitor_->appendInfo(tr("Sending Raw Data: %1").arg(QString(data.toHex(' ').toUpper())));
            
            worker_->sendRaw(data);
            
            controlWidget_->recordTx();
    });

    connect(controlWidget_, &widgets::ControlWidget::pollRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty) {
            if (!ensureConnected()) return;
            if (pollInFlight_) return;
            
            int slaveId = functionWidget_->getSlaveId();
            
            using namespace modbus::base;
            auto result = ModbusPduBuilder::buildReadRequest(static_cast<FunctionCode>(fc), addr, qty);
            if (!result.isOk()) {
                trafficMonitor_->appendInfo(tr("Error: %1").arg(result.error));
                return;
            }
            Pdu request = *result.pdu;
            
            int requestId = nextRequestId();
            requestStart_[requestId] = std::chrono::steady_clock::now();
            requestKinds_[requestId] = RequestKind::Poll;
            requestAddrs_[requestId] = static_cast<uint16_t>(addr);

            pollFunctionCode_ = fc;
            pollAddress_ = addr;
            pollQuantity_ = qty;
            pollSlaveId_ = slaveId;
            if (pollSummaryWindowStart_ == std::chrono::steady_clock::time_point{}) {
                pollSummaryWindowStart_ = std::chrono::steady_clock::now();
            }
            pollInFlight_ = true;
            suppressPollTrafficLog_ = true;
            controlWidget_->recordTx(); // 轮询提交时增加 TX 计数
            worker_->submit(request, slaveId, requestId);
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

void ModbusRtuView::appendConnectionInfo(const QString& message) {
    if (!trafficMonitor_) {
        return;
    }
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::Connection;
    event.summary = message;
    trafficMonitor_->appendEvent(event);
}

void ModbusRtuView::handlePollCompletion(bool success, int rttMs, const QString& error) {
    if (pollSummaryWindowStart_ == std::chrono::steady_clock::time_point{}) {
        pollSummaryWindowStart_ = std::chrono::steady_clock::now();
    }

    if (success) {
        ++pollSummarySuccessCount_;
        pollSummaryTotalRttMs_ += (rttMs > 0 ? rttMs : 0);
    } else {
        ++pollSummaryErrorCount_;
        ui::common::TrafficEvent event;
        event.level = ui::common::TrafficEventLevel::Warning;
        event.requestType = ui::common::TrafficRequestType::Poll;
        event.isPoll = true;
        event.summary = tr("Poll Error: %1").arg(error);
        trafficMonitor_->appendEvent(event);
    }
    flushPollSummary(false);
}

void ModbusRtuView::flushPollSummary(bool force) {
    const int total = pollSummarySuccessCount_ + pollSummaryErrorCount_;
    if (total <= 0) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const bool due = (now - pollSummaryWindowStart_) >= std::chrono::milliseconds(1000);
    if (!force && !due) {
        return;
    }

    const int avgRttMs = pollSummarySuccessCount_ > 0
        ? static_cast<int>(pollSummaryTotalRttMs_ / pollSummarySuccessCount_)
        : 0;

    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Info;
    event.requestType = ui::common::TrafficRequestType::Poll;
    event.isPoll = true;
    event.summary = tr("Poll Summary FC:%1 Addr:%2 Qty:%3 Slave:%4 Success:%5 Error:%6 Avg RTT:%7 ms")
        .arg(pollFunctionCode_)
        .arg(pollAddress_)
        .arg(pollQuantity_)
        .arg(pollSlaveId_)
        .arg(pollSummarySuccessCount_)
        .arg(pollSummaryErrorCount_)
        .arg(avgRttMs);
    trafficMonitor_->appendEvent(event);

    pollSummarySuccessCount_ = 0;
    pollSummaryErrorCount_ = 0;
    pollSummaryTotalRttMs_ = 0;
    pollSummaryWindowStart_ = now;
}

void ModbusRtuView::releaseStack() {
    flushPollSummary(true);
    ++connectionGeneration_;
    rtuSessionConnected_ = false;
    pollInFlight_ = false;
    suppressPollTrafficLog_ = false;
    if (connectionWidget_) {
        connectionWidget_->setConnected(false);
    }
    requestStart_.clear();
    requestKinds_.clear();
    requestAddrs_.clear();
    if (controlWidget_) {
        controlWidget_->setLinked(false);
        controlWidget_->setPollingEnabled(false);
    }
    
    auto channel = std::move(channel_);
    auto client = std::move(client_);
    auto worker = std::move(worker_);
    auto thread = std::move(workerThread_);
    if (!worker && !client && !channel && !thread) {
        return;
    }

    if (worker) {
        worker->stop();
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

int ModbusRtuView::nextRequestId() {
    if (requestId_ == std::numeric_limits<int>::max()) {
        requestId_ = 0;
    }
    return ++requestId_;
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
