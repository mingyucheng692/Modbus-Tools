/**
 * @file GenericSerialView.cpp
 * @brief Implementation of GenericSerialView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericSerialView.h"
#include "AppConstants.h"
#include "../../../core/common/ISettingsService.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../../infra/io/ChannelOperationWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QThread>
#include <QTimer>
#include <QMetaObject>
#include <QCheckBox>
#include <QEvent>
#include <spdlog/spdlog.h>

namespace ui::views::generic_serial {

GenericSerialView::GenericSerialView(core::common::ISettingsService* settingsService, QWidget *parent)
    : GenericChannelViewBase(settingsService, parent) {
    setupUi();
    startWorker();
}

GenericSerialView::~GenericSerialView() noexcept {
    stopWorker();
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
    connect(inputWidget_, &widgets::GenericInputWidget::fileSendRequested,
            this, &GenericSerialView::onFileSendRequested);
            
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

    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &GenericSerialView::onReconnectTimerTick);
}

void GenericSerialView::startWorker() {
    GenericChannelViewBase::startWorker();
    if (worker_) {
        connect(worker_, &io::ChannelOperationWorker::stateChanged, this, &GenericSerialView::onWorkerStateChanged);
    }
}

void GenericSerialView::onConnectClicked(const io::SerialConfig& config) {
    if (!worker_) return;

    stopReconnectTimer();
    reconnectPolicy_.reset();
    reconnectConfig_ = config;

    spdlog::info("GenericSerial: Connecting to {}", config.portName.toStdString());
    if (monitor_) {
        monitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    }
    connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);
    
    QMetaObject::invokeMethod(worker_, "openSerial", 
                              Qt::QueuedConnection, 
                              Q_ARG(io::SerialConfig, config));
}

void GenericSerialView::onWorkerStateChanged(io::ChannelState state) {
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
        reconnectPolicy_.onSuccess();
    } else if (wasConnected
               && connectionWidget_->autoReconnectEnabled()
               && !reconnectTimer_->isActive()) {
        startReconnectTimer(connectionWidget_);
    }
}

void GenericSerialView::onDtrChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(QStringLiteral("serial_port/dtr"), checked);
    }
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "setDtr", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::onRtsChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(QStringLiteral("serial_port/rts"), checked);
    }
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "setRts", 
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
    if (!connectionWidget_ || !worker_) return;

    if (!connectionWidget_->autoReconnectEnabled()) {
        stopReconnectTimer();
        return;
    }

    if (reconnectConfig_.portName.isEmpty()) {
        stopReconnectTimer();
        return;
    }

    spdlog::info("GenericSerial: Auto-reconnecting to {} (attempt {})",
                 reconnectConfig_.portName.toStdString(),
                 reconnectPolicy_.attemptCount());

    if (monitor_) {
        monitor_->appendInfo(tr("Reconnecting to %1...").arg(reconnectConfig_.portName));
    }
    connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker_, "openSerial",
                              Qt::QueuedConnection,
                              Q_ARG(io::SerialConfig, reconnectConfig_));
}

} // namespace ui::views::generic_serial
