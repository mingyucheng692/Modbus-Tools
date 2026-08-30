/**
 * @file GenericSerialView.cpp
 * @brief Implementation of GenericSerialView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericSerialView.h"
#include "Config.h"
#include "infra/config/ISettingsService.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../../infra/io/ChannelOperationWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMetaObject>
#include <QCheckBox>
#include <QEvent>
#include <spdlog/spdlog.h>

namespace ui::views::generic_serial {

GenericSerialView::GenericSerialView(infra::config::ISettingsService* settingsService, QWidget *parent)
    : GenericChannelViewBase(settingsService, parent),
      channelCtrl_(this) {
    channelController_ = &channelCtrl_;
    setupUi();
    startWorker();
}

GenericSerialView::~GenericSerialView() noexcept {
    // channelCtrl_ is a value member, destroyed automatically.
    // channelController_ pointer in base class is already null when this destructor runs.
}

void GenericSerialView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section (Top)
    auto topLayout = new QHBoxLayout();
    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup(QStringLiteral("serial_port"));
    topLayout->addWidget(connectionWidget_);
    
    // Serial Controls (DTR/RTS)
    controlGroup_ = new QGroupBox(this);
    auto controlLayout = new QHBoxLayout(controlGroup_);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    
    dtrCheck_ = new QCheckBox(this);
    rtsCheck_ = new QCheckBox(this);
    controlLayout->addWidget(dtrCheck_);
    controlLayout->addWidget(rtsCheck_);
    
    topLayout->addWidget(controlGroup_);
    topLayout->addStretch();
    
    mainLayout->addLayout(topLayout);

    // 2. Central Area (Traffic Monitor)
    monitor_ = new widgets::ByteMonitorWidget(settingsService_, this);
    monitor_->setSettingsGroup(QStringLiteral("serial_port/traffic"));
    mainLayout->addWidget(monitor_);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey(QStringLiteral("serial_port/ui/inputCollapsed"));
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("serial_port/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 0);

    // Connections
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked, 
            this, &GenericSerialView::onConnectClicked);
    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked, 
            this, &GenericSerialView::onDisconnectClicked);
    
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, &GenericSerialView::onSendRequested);

    connect(dtrCheck_, &QCheckBox::toggled, this, &GenericSerialView::onDtrChanged);
    connect(rtsCheck_, &QCheckBox::toggled, this, &GenericSerialView::onRtsChanged);
    
    // Disable controls initially
    dtrCheck_->setEnabled(false);
    rtsCheck_->setEnabled(false);
    if (settingsService_) {
        dtrCheck_->setChecked(settingsService_->value(QStringLiteral("serial_port/dtr")).toBool());
        rtsCheck_->setChecked(settingsService_->value(QStringLiteral("serial_port/rts")).toBool());
    }

    retranslateUi();

    // Reconnect timer is managed by ChannelController; connect its signal to our slot.
    connect(&channelCtrl_, &ChannelController::reconnectTimeout,
            this, &GenericSerialView::onReconnectTimerTick);
}

void GenericSerialView::startWorker() {
    auto* worker = channelCtrl_.createWorker();
    connect(worker, &io::ChannelOperationWorker::channelErrorOccurred,
            this, &GenericSerialView::onWorkerError);
    connect(worker, &io::ChannelOperationWorker::monitor,
            this, &GenericSerialView::onWorkerMonitor);
    connect(worker, &io::ChannelOperationWorker::stateChangedWithGeneration,
            this, &GenericSerialView::onWorkerStateChanged);
}

void GenericSerialView::onConnectClicked(const io::SerialConfig& config) {
    auto* worker = channelCtrl_.worker();
    if (!worker) return;

    channelCtrl_.stopReconnectTimer();
    channelCtrl_.resetReconnect();
    reconnectConfig_ = config;

    // Fresh user intent: a prior manual disconnect is superseded, so a
    // later passive loss may trigger the reconnect loop again (NEW-C).
    manualDisconnectRequested_ = false;
    const quint64 generation = ++connectionGeneration_;

    SPDLOG_INFO("GenericSerial: Connecting to {}", config.portName.toStdString());
    if (monitor_) {
        monitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    }
    connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker, "openSerial",
                              Qt::QueuedConnection,
                              Q_ARG(io::SerialConfig, config),
                              Q_ARG(quint64, generation));
}

void GenericSerialView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    if (generation != connectionGeneration_) {
        return;
    }

    const bool wasConnected = isConnected_;

    isConnected_ = (state == io::ChannelState::Open);
    dtrCheck_->setEnabled(isConnected_);
    rtsCheck_->setEnabled(isConnected_);

    QString stateStr;
    switch (state) {
        case io::ChannelState::Closed: stateStr = tr("Closed"); break;
        case io::ChannelState::Opening: stateStr = tr("Opening"); break;
        case io::ChannelState::Open: stateStr = tr("Open"); break;
        case io::ChannelState::Closing: stateStr = tr("Closing"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }

    switch (state) {
    case io::ChannelState::Opening:
        connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);
        break;
    case io::ChannelState::Open:
        connectionWidget_->setConnected(true);
        break;
    case io::ChannelState::Closing:
        connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Disconnecting);
        break;
    case io::ChannelState::Closed:
    case io::ChannelState::Error:
        connectionWidget_->setConnected(false);
        break;
    }

    if (monitor_) {
        monitor_->appendInfo(tr("State changed: %1").arg(stateStr));
    }

    if (isConnected_) {
        // A live session invalidates any prior manual-disconnect intent;
        // subsequent losses are passive again (reconnect eligible).
        manualDisconnectRequested_ = false;
        channelCtrl_.reconnectPolicy().onSuccess();
    } else if (wasConnected
               && !manualDisconnectRequested_ // NEW-C: passive losses only
               && connectionWidget_->autoReconnectEnabled()
               && !channelCtrl_.reconnectTimer()->isActive()) {
        auto& policy = channelCtrl_.reconnectPolicy();
        if (policy.exhausted()) {
            if (monitor_) {
                monitor_->appendInfo(tr("Auto-reconnect exhausted (%1 attempts)")
                                         .arg(policy.maxRetries()));
            }
            channelCtrl_.stopReconnectTimer();
            return;
        }
        const int delay = connectionWidget_->reconnectDelayMs();
        if (monitor_) {
            monitor_->appendInfo(tr("Auto-reconnect in %1ms (%2)")
                                     .arg(delay)
                                     .arg(policy.statusString()));
        }
        channelCtrl_.startReconnectTimer(delay);
    }
}

void GenericSerialView::onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        QString localizedMsg;
        switch (code) {
            case io::ChannelErrorCode::ConnectionFailed: localizedMsg = tr("Connection failed"); break;
            case io::ChannelErrorCode::Timeout: localizedMsg = tr("Connection timeout"); break;
            case io::ChannelErrorCode::WriteFailed: localizedMsg = tr("Write failed"); break;
            case io::ChannelErrorCode::ReadFailed: localizedMsg = tr("Read failed"); break;
            case io::ChannelErrorCode::PortNotFound: localizedMsg = tr("Port not found"); break;
            case io::ChannelErrorCode::PermissionDenied: localizedMsg = tr("Permission denied"); break;
            case io::ChannelErrorCode::ConnectionReset: localizedMsg = tr("Connection reset"); break;
            default: localizedMsg = tr("Unknown error"); break;
        }
        monitor_->appendError(tr("Error: %1").arg(localizedMsg));
    }
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("Serial Worker") : deviceHint;
    SPDLOG_ERROR("{} Error (code={}): {}", hint.toStdString(), static_cast<int>(code), error.toStdString());
}

void GenericSerialView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (monitor_) {
        monitor_->appendMessage(isTx, data);
    }
}

void GenericSerialView::onDtrChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(QStringLiteral("serial_port/dtr"), checked);
    }
    auto* worker = channelCtrl_.worker();
    if (!worker) return;
    QMetaObject::invokeMethod(worker, "setDtr", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::onRtsChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(QStringLiteral("serial_port/rts"), checked);
    }
    auto* worker = channelCtrl_.worker();
    if (!worker) return;
    QMetaObject::invokeMethod(worker, "setRts", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::retranslateUi() {
    if (controlGroup_) controlGroup_->setTitle(tr("Control"));
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
    if (dtrCheck_) dtrCheck_->setText(tr("DTR"));
    if (rtsCheck_) rtsCheck_->setText(tr("RTS"));
}

void GenericSerialView::onReconnectTimerTick() {
    if (!connectionWidget_) return;
    auto* worker = channelCtrl_.worker();
    if (!worker) return;

    if (!connectionWidget_->autoReconnectEnabled()) {
        channelCtrl_.stopReconnectTimer();
        return;
    }

    if (reconnectConfig_.portName.isEmpty()) {
        channelCtrl_.stopReconnectTimer();
        return;
    }

    SPDLOG_INFO("GenericSerial: Auto-reconnecting to {} (attempt {})",
                 reconnectConfig_.portName.toStdString(),
                 channelCtrl_.reconnectPolicy().attemptCount());

    manualDisconnectRequested_ = false; // reconnect tick = fresh intent
    const quint64 generation = ++connectionGeneration_;
    if (monitor_) {
        monitor_->appendInfo(tr("Reconnecting to %1...").arg(reconnectConfig_.portName));
    }
    connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker, "openSerial",
                              Qt::QueuedConnection,
                              Q_ARG(io::SerialConfig, reconnectConfig_),
                              Q_ARG(quint64, generation));
}

} // namespace ui::views::generic_serial