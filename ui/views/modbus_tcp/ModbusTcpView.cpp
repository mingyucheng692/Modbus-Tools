/**
 * @file ModbusTcpView.cpp
 * @brief Implementation of ModbusTcpView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusTcpView.h"
#include "AppConstants.h"
#include "../../common/ISettingsService.h"
#include "../../application/modbus/RequestSubmissionService.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TcpConnectionStateCoordinator.h"
#include "../../common/TrafficEvent.h"
#include "modbus/factory/ModbusFactory.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QEvent>
#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <QMetaObject>
#include <QPointer>
#include <algorithm>
#include <limits>

namespace ui::views::modbus_tcp {

namespace {
QString connectedText() {
    return QCoreApplication::translate("ui::views::modbus_tcp::ModbusTcpView", "Connected");
}

QString disconnectedText() {
    return QCoreApplication::translate("ui::views::modbus_tcp::ModbusTcpView", "Disconnected");
}

QString retryWord(int retryCount)
{
    return QCoreApplication::translate(
        "ui::views::modbus_tcp::ModbusTcpView",
        retryCount == 1 ? "retry" : "retries");
}

QString successWithRetrySummary(const QString& baseMessage, int retryCount)
{
    if (retryCount <= 0) {
        return baseMessage;
    }
    return QCoreApplication::translate(
               "ui::views::modbus_tcp::ModbusTcpView",
               "%1 after %2 %3")
        .arg(baseMessage)
        .arg(retryCount)
        .arg(retryWord(retryCount));
}

QString errorWithRetrySummary(const QString& error, int retryCount)
{
    if (retryCount <= 0) {
        return QCoreApplication::translate(
                   "ui::views::modbus_tcp::ModbusTcpView",
                   "Error: %1")
            .arg(error);
    }
    return QCoreApplication::translate(
               "ui::views::modbus_tcp::ModbusTcpView",
               "Error: %1 (failed after %2 %3, see log for details)")
        .arg(error)
        .arg(retryCount)
        .arg(retryWord(retryCount));
}
}

ModbusTcpView::ModbusTcpView(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
}

ModbusTcpView::~ModbusTcpView() {
    releaseStack();
}

void ModbusTcpView::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
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

void ModbusTcpView::setLinked(bool linked) {
    linked_ = linked;
    if (controlWidget_) {
        controlWidget_->setLinked(linked);
    }
}

bool ModbusTcpView::isLinked() const {
    return linked_;
}

void ModbusTcpView::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);
    
    connectionWidget_ = new widgets::TcpConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup("modbus/tcp");
    mainLayout_->addWidget(connectionWidget_);
    
    functionWidget_ = new widgets::FunctionWidget(settingsService_, this);
    functionWidget_->setSettingsGroup("modbus/tcp/standard");
    functionWidget_->setTcpMode(true);
    mainLayout_->addWidget(functionWidget_);
    
    dataGroup_ = new widgets::CollapsibleSection(settingsService_, this);
    dataGroup_->setSettingsKey("modbus/tcp/ui/dataMonitorCollapsed");
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
    trafficMonitor_->setSettingsGroup("modbus/tcp/traffic");

    mainLayout_->addWidget(dataGroup_);
    mainLayout_->addWidget(trafficMonitor_);

    controlWidget_ = new widgets::ControlWidget(settingsService_, this);
    controlWidget_->setSettingsGroup("modbus/tcp/control");
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

    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
        [this](const QString& ip, int port) {
            spdlog::info("ModbusTcpView: Connect requested to {}:{}", ip.toStdString(), port);
            releaseStack();
            suppressDisconnectAlert_ = false;
            const quint64 generation = connectionGeneration_;
            appendConnectionInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));

            modbus::base::ModbusConfig config;
            config.mode = modbus::base::ModbusMode::TCP;
            config.ipAddress = ip;
            config.port = port;
            config.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
            config.timeoutMs = timeoutMs_;
            config.retries = retries_;
            config.retryIntervalMs = retryIntervalMs_;

            modbus::factory::ModbusFactory factory;
            auto stack = factory.createStack(config);
            if (!stack.worker || !stack.thread) {
                trafficMonitor_->appendInfo(tr("Failed to create Modbus stack"));
                return;
            }

            currentConfig_ = config;
            channel_ = std::move(stack.channel);
            client_ = std::move(stack.client);
            worker_ = std::move(stack.worker);
            workerThread_ = std::move(stack.thread);

            requestService_ = new ui::application::modbus::RequestSubmissionService(this);
            connect(requestService_, &ui::application::modbus::RequestSubmissionService::txCountUpdated,
                    controlWidget_, &ui::widgets::ControlWidget::recordTx);

            QPointer<ModbusTcpView> self(this);
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
                    const bool wasConnected = self->tcpSessionConnected_;
                    const auto transition = ui::common::TcpConnectionStateCoordinator::transition(
                        state,
                        wasConnected,
                        self->suppressDisconnectAlert_);
                    if (transition.setConnected) {
                        self->tcpSessionConnected_ = true;
                        self->connectionWidget_->setConnected(true);
                        self->appendConnectionInfo(connectedText());
                    } else if (state == io::ChannelState::Open) {
                        self->tcpSessionConnected_ = true;
                        self->connectionWidget_->setConnected(true);
                    }
                    if (transition.clearSuppressDisconnectAlert) {
                        self->suppressDisconnectAlert_ = false;
                    }
                    if (transition.setConnected) {
                        return;
                    }
                    if (transition.setDisconnected) {
                        self->tcpSessionConnected_ = false;
                        self->connectionWidget_->setConnected(false);
                        self->controlWidget_->setPollingEnabled(false);
                        self->appendConnectionInfo(disconnectedText());
                        if (transition.showDisconnectAlert) {
                            ui::common::ConnectionAlert::showDisconnected(self);
                        }
                    }
                }, Qt::QueuedConnection);
            });

            connect(worker_.get(), &modbus::dispatch::ModbusWorker::connectFinished, this,
                [this, self, generation](bool ok, const QString& error) {
                    if (!self) return;
                    if (generation != connectionGeneration_) return;
                    const bool wasConnected = tcpSessionConnected_;
                    tcpSessionConnected_ = ok;
                    if (!ok) {
                        pollInFlight_ = false;
                        suppressPollTrafficLog_ = false;
                    }
                    if (ok) {
                        suppressDisconnectAlert_ = false;
                        connectionWidget_->setConnected(true);
                        if (!wasConnected) {
                            self->appendConnectionInfo(tr("Connected"));
                        }
                    } else {
                        connectionWidget_->setConnected(false);
                        self->appendConnectionInfo(tr("Connection failed: %1").arg(error));
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
                        pollInFlight_ = false;
                        suppressPollTrafficLog_ = false;
                        handlePollCompletion(response.isSuccess,
                                             response.rttMs,
                                             response.retryCount,
                                             response.error);
                    }

                    // 分流设计：仅在成功时通过底层测量的精�?RTT 更新统计
                    if (response.isSuccess) {
                        // 更新成功统计 (RX + 1, 更新 RTT)
                        controlWidget_->recordRx(response.rttMs);

                        if (kind == ui::application::modbus::RequestKind::Read) {
                            trafficMonitor_->appendInfo(
                                successWithRetrySummary(tr("Success: Response received"), response.retryCount));
                        } else if (kind == ui::application::modbus::RequestKind::Write) {
                            trafficMonitor_->appendInfo(
                                successWithRetrySummary(tr("Success: Write confirmed"), response.retryCount));
                        }
                        
                        // Emit linkage signal for Frame Analyzer if linked
                        if (linked_) {
                            emit linkageDataReceived(response.pdu, modbus::core::parser::ProtocolType::Tcp, addr);
                        }
                    } else {
                        // Failure path: record the error count and skip RTT statistics.
                        controlWidget_->recordError();

                        if (kind != ui::application::modbus::RequestKind::Poll) {
                            trafficMonitor_->appendError(
                                errorWithRetrySummary(response.error, response.retryCount));
                        }
                    }

                }, Qt::QueuedConnection);

            worker_->start();
            worker_->requestConnect();
    });

    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("ModbusTcpView: Disconnect requested");
            suppressDisconnectAlert_ = true;
            tcpSessionConnected_ = false;
            connectionWidget_->setConnected(false);
            appendConnectionInfo(tr("Disconnected"));
            releaseStack();
    });

    auto ensureConnected = [this]() {
        if (worker_ && tcpSessionConnected_) {
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
                trafficMonitor_->appendError(tr("Error: Request service not available"));
                return;
            }

            auto result = requestService_->buildReadRequest(fc, addr, qty, slaveId);
            if (!result.ok) {
                trafficMonitor_->appendError(tr("Error: %1").arg(result.error));
                return;
            }

            trafficMonitor_->appendInfo(tr("Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4")
                .arg(fc).arg(addr).arg(qty).arg(slaveId));

            worker_->submit(result.pdu, slaveId, result.requestId);
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this, ensureConnected](uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId) {
            if (!ensureConnected()) return;

            if (!requestService_) {
                trafficMonitor_->appendError(tr("Error: Request service not available"));
                return;
            }

            int quantity = functionWidget_->getQuantity();

            auto result = requestService_->buildWriteRequest(fc, addr, dataStr, fmt, slaveId, quantity);
            if (!result.ok) {
                trafficMonitor_->appendError(tr("Error: %1").arg(result.error));
                return;
            }

            trafficMonitor_->appendInfo(tr("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
                .arg(fc).arg(addr).arg(dataStr).arg(slaveId));

            worker_->submit(result.pdu, slaveId, result.requestId);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
        [this, ensureConnected](const QByteArray& data) {
            if (!ensureConnected()) return;
            if (!requestService_ || !requestService_->validateRawData(data)) return;

            trafficMonitor_->appendInfo(tr("Sending Raw Data: %1").arg(QString(data.toHex(' ').toUpper())));

            worker_->sendRaw(data);
    });

    connect(controlWidget_, &widgets::ControlWidget::pollRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty) {
            if (!ensureConnected()) return;
            if (pollInFlight_) return;

            if (!requestService_) {
                trafficMonitor_->appendError(tr("Error: Request service not available"));
                return;
            }

            int slaveId = functionWidget_->getSlaveId();

            auto result = requestService_->buildReadRequest(fc, addr, qty, slaveId,
                ui::application::modbus::RequestKind::Poll);
            if (!result.ok) {
                trafficMonitor_->appendError(tr("Error: %1").arg(result.error));
                return;
            }

            pollFunctionCode_ = fc;
            pollAddress_ = addr;
            pollQuantity_ = qty;
            pollSlaveId_ = slaveId;
            if (pollSummaryWindowStart_ == std::chrono::steady_clock::time_point{}) {
                pollSummaryWindowStart_ = std::chrono::steady_clock::now();
            }
            pollInFlight_ = true;
            suppressPollTrafficLog_ = true;
            worker_->submit(result.pdu, slaveId, result.requestId);
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

QString ModbusTcpView::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void ModbusTcpView::appendConnectionInfo(const QString& message) {
    if (!trafficMonitor_) {
        return;
    }
    ui::common::TrafficEvent event;
    event.requestType = ui::common::TrafficRequestType::Connection;
    event.summary = message;
    trafficMonitor_->appendEvent(event);
}

void ModbusTcpView::handlePollCompletion(bool success, int rttMs, int retryCount, const QString& error) {
    if (pollSummaryWindowStart_ == std::chrono::steady_clock::time_point{}) {
        pollSummaryWindowStart_ = std::chrono::steady_clock::now();
    }
    const auto now = std::chrono::steady_clock::now();
    pollSummaryRetryCount_ += std::max(0, retryCount);

    if (success) {
        ++pollSummarySuccessCount_;
        pollSummaryTotalRttMs_ += (rttMs > 0 ? rttMs : 0);
        pollLastSuccessTime_ = now;
        if (pollConsecutiveErrorCount_ > 0 && trafficMonitor_) {
            ui::common::TrafficEvent event;
            event.level = ui::common::TrafficEventLevel::Info;
            event.requestType = ui::common::TrafficRequestType::Poll;
            event.isPoll = true;
            event.summary = tr("Poll recovered after %1 consecutive failures")
                .arg(pollConsecutiveErrorCount_);
            trafficMonitor_->appendEvent(event);
        }
        resetPollErrorTracking();
    } else {
        ++pollSummaryErrorCount_;
        ++pollConsecutiveErrorCount_;
        if (pollFailureStreakStartTime_ == std::chrono::steady_clock::time_point{}) {
            pollFailureStreakStartTime_ = now;
        }

        const QString normalizedError = error.toLower();
        int threshold = pollErrorThreshold();
        if (normalizedError.contains(QStringLiteral("timeout"))
            || normalizedError.contains(QStringLiteral("timed out"))) {
            threshold = std::max(2, threshold - 1);
        } else if (normalizedError.contains(QStringLiteral("busy"))
            || normalizedError.contains(QStringLiteral("tempor"))) {
            threshold = std::min(10, threshold + 1);
        }

        const bool zeroSuccessWindowExceeded =
            pollFailureStreakStartTime_ != std::chrono::steady_clock::time_point{}
            && (now - pollFailureStreakStartTime_) >= std::chrono::milliseconds(3000);
        const bool connectionFault = !tcpSessionConnected_;
        const bool escalateToError = connectionFault
            || zeroSuccessWindowExceeded
            || pollConsecutiveErrorCount_ >= threshold;
        const bool shouldLogEscalatedError = !pollErrorEscalated_
            || error != pollLastErrorText_
            || (now - pollLastErrorLogTime_) >= std::chrono::seconds(5);

        ui::common::TrafficEvent event;
        event.level = escalateToError
            ? ui::common::TrafficEventLevel::Error
            : ui::common::TrafficEventLevel::Warning;
        event.requestType = ui::common::TrafficRequestType::Poll;
        event.isPoll = true;

        if (escalateToError) {
            if (connectionFault) {
                event.summary = tr("Poll Error: Connection unavailable during polling (%1)")
                    .arg(error);
            } else if (!pollErrorEscalated_) {
                event.summary = tr("Poll Error escalated after %1 consecutive failures: %2")
                    .arg(pollConsecutiveErrorCount_)
                    .arg(error);
            } else {
                event.summary = tr("Poll Error persists (%1 consecutive failures): %2")
                    .arg(pollConsecutiveErrorCount_)
                    .arg(error);
            }

            if (trafficMonitor_ && shouldLogEscalatedError) {
                trafficMonitor_->appendEvent(event);
                pollLastErrorLogTime_ = now;
            }

            pollErrorEscalated_ = true;
            pollLastErrorText_ = error;
        } else if (trafficMonitor_) {
            event.summary = tr("Poll Error: %1").arg(error);
            trafficMonitor_->appendEvent(event);
            pollLastErrorText_ = error;
        }
    }
    flushPollSummary(false);
}

int ModbusTcpView::pollErrorThreshold() const {
    constexpr int kTargetUpgradeWindowMs = 3000;
    constexpr int kMinThreshold = 3;
    constexpr int kMaxThreshold = 10;

    const int intervalMs = controlWidget_
        ? std::max(controlWidget_->pollingIntervalMs(), 1)
        : app::constants::Values::Polling::kDefaultIntervalMs;
    const int roundedThreshold = (kTargetUpgradeWindowMs + intervalMs / 2) / intervalMs;
    return std::clamp(roundedThreshold, kMinThreshold, kMaxThreshold);
}

void ModbusTcpView::resetPollErrorTracking() {
    pollConsecutiveErrorCount_ = 0;
    pollErrorEscalated_ = false;
    pollLastErrorText_.clear();
    pollLastErrorLogTime_ = std::chrono::steady_clock::time_point{};
    pollFailureStreakStartTime_ = std::chrono::steady_clock::time_point{};
}

void ModbusTcpView::flushPollSummary(bool force) {
    const int total = pollSummarySuccessCount_ + pollSummaryErrorCount_;
    if (total <= 0) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const bool due = (now - pollSummaryWindowStart_) >= std::chrono::milliseconds(1000);
    if (!force && !due) {
        return;
    }

    const QString avgRttText = pollSummarySuccessCount_ > 0
        ? tr("%1 ms").arg(static_cast<int>(pollSummaryTotalRttMs_ / pollSummarySuccessCount_))
        : tr("--");

    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Info;
    event.requestType = ui::common::TrafficRequestType::Poll;
    event.isPoll = true;
    event.summary = tr("Poll Summary FC:%1 Addr:%2 Qty:%3 Slave:%4 Success:%5 Error:%6 Retries:%7 Avg Success RTT:%8")
        .arg(pollFunctionCode_)
        .arg(pollAddress_)
        .arg(pollQuantity_)
        .arg(pollSlaveId_)
        .arg(pollSummarySuccessCount_)
        .arg(pollSummaryErrorCount_)
        .arg(pollSummaryRetryCount_)
        .arg(avgRttText);
    trafficMonitor_->appendEvent(event);

    pollSummarySuccessCount_ = 0;
    pollSummaryErrorCount_ = 0;
    pollSummaryRetryCount_ = 0;
    pollSummaryTotalRttMs_ = 0;
    pollSummaryWindowStart_ = now;
}

void ModbusTcpView::releaseStack() {
    flushPollSummary(true);
    ++connectionGeneration_;
    suppressDisconnectAlert_ = true;
    tcpSessionConnected_ = false;
    pollInFlight_ = false;
    suppressPollTrafficLog_ = false;
    resetPollErrorTracking();
    pollLastSuccessTime_ = std::chrono::steady_clock::time_point{};
    if (connectionWidget_) {
        connectionWidget_->setConnected(false);
    }
    if (requestService_) {
        requestService_->clearAll();
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


void ModbusTcpView::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void ModbusTcpView::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void ModbusTcpView::refreshReceiveDisplay() {
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

void ModbusTcpView::refreshSendDisplay() {
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

void ModbusTcpView::retranslateUi() {
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

void ModbusTcpView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::modbus_tcp
