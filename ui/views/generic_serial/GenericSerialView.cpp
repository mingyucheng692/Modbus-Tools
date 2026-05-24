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
#include "../../common/ISettingsService.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "../../../core/io/ChannelOperationWorker.h"
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

GenericSerialView::GenericSerialView(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
    startWorker();
}

GenericSerialView::~GenericSerialView() {
    stopWorker();
}

void GenericSerialView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section (Top)
    auto topLayout = new QHBoxLayout();
    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup("serial_port");
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
    monitor_->setSettingsGroup("serial_port/traffic");
    mainLayout->addWidget(monitor_);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey("serial_port/ui/inputCollapsed");
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup("serial_port/input");
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
        dtrCheck_->setChecked(settingsService_->value("serial_port/dtr").toBool());
        rtsCheck_->setChecked(settingsService_->value("serial_port/rts").toBool());
    }

    retranslateUi();

    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &GenericSerialView::onReconnectTimerTick);
}

void GenericSerialView::startWorker() {
    workerThread_ = new QThread();
    auto* worker = new io::ChannelOperationWorker();
    worker->moveToThread(workerThread_);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);

    connect(worker, &io::ChannelOperationWorker::stateChanged, this, &GenericSerialView::onWorkerStateChanged);
    connect(worker, &io::ChannelOperationWorker::channelErrorOccurred, this, &GenericSerialView::onWorkerError);
    connect(worker, &io::ChannelOperationWorker::monitor, this, &GenericSerialView::onWorkerMonitor);
    connect(worker, &io::ChannelOperationWorker::fileTransferStarted, this, [this](const QString& filePath, qint64 totalBytes) {
        lastLoggedFileTransferPct_ = -1;
        monitor_->appendInfo(tr("File transfer started: %1 (%2 bytes)").arg(filePath).arg(totalBytes));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferProgress, this, [this](const QString& filePath, qint64 sentBytes, qint64 totalBytes) {
        if (totalBytes <= 0) {
            return;
        }
        const int progress = static_cast<int>((sentBytes * 100) / totalBytes);
        if (progress == lastLoggedFileTransferPct_) {
            return;
        }
        lastLoggedFileTransferPct_ = progress;
        monitor_->appendInfo(tr("File transfer progress: %1 %2/%3")
                                        .arg(filePath)
                                        .arg(sentBytes)
                                        .arg(totalBytes));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferFinished, this, [this](const QString& filePath) {
        monitor_->appendInfo(tr("File transfer finished: %1").arg(filePath));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferFailed, this, [this](const QString& filePath, const QString& error) {
        monitor_->appendInfo(tr("File transfer failed: %1 (%2)").arg(filePath, error));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferCanceled, this, [this](const QString& filePath) {
        monitor_->appendInfo(tr("File transfer canceled: %1").arg(filePath));
    });
    
    workerThread_->start();
    worker_ = worker;
}

void GenericSerialView::stopWorker() {
    stopReconnectTimer();

    auto* thread = workerThread_;
    auto* worker = worker_;
    workerThread_ = nullptr;
    worker_ = nullptr;

    if (!thread) {
        return;
    }

    if (!worker) {
        if (thread->isRunning()) {
            thread->quit();
        } else {
            delete thread;
        }
        return;
    }

    worker->disconnect(this);

    if (!thread->isRunning()) {
        delete worker;
        delete thread;
        return;
    }

    QMetaObject::invokeMethod(worker, [worker, thread]() {
        QObject::connect(worker, &QObject::destroyed, thread, &QThread::quit, Qt::UniqueConnection);
        worker->close();
        worker->deleteLater();
    }, Qt::QueuedConnection);
}

void GenericSerialView::onConnectClicked(const io::SerialConfig& config) {
    if (!worker_) return;

    stopReconnectTimer();
    reconnectPolicy_.reset();
    reconnectConfig_ = config;

    spdlog::info("GenericSerial: Connecting to {}", config.portName.toStdString());
    monitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    
    QMetaObject::invokeMethod(worker_, "openSerial", 
                              Qt::QueuedConnection, 
                              Q_ARG(io::SerialConfig, config));
}

void GenericSerialView::onDisconnectClicked() {
    if (!worker_) return;

    stopReconnectTimer();
    reconnectPolicy_.reset();
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericSerialView::onSendRequested(const QByteArray& data) {
    if (!worker_) return;
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }
    QMetaObject::invokeMethod(worker_, "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericSerialView::onFileSendRequested(const QString& filePath) {
    if (!worker_) return;
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }
    QMetaObject::invokeMethod(worker_, "sendFile",
                              Qt::QueuedConnection,
                              Q_ARG(QString, filePath),
                              Q_ARG(int, app::constants::Values::GenericIo::kFileSendChunkSizeBytes));
}

void GenericSerialView::onWorkerStateChanged(io::ChannelState state) {
    const bool wasConnected = isConnected_;

    isConnected_ = (state == io::ChannelState::Open);
    connectionWidget_->setConnected(isConnected_);
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

    monitor_->appendInfo(tr("State changed: %1").arg(stateStr));

    if (isConnected_) {
        reconnectPolicy_.onSuccess();
    } else if (wasConnected
               && connectionWidget_->autoReconnectEnabled()
               && !reconnectTimer_->isActive()) {
        startReconnectTimer();
    }
}

void GenericSerialView::onWorkerError(const QString& deviceHint, const QString& error) {
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("Serial") : deviceHint;
    monitor_->appendError(tr("Error: %1").arg(error));
    spdlog::error("{} Error: {}", hint.toStdString(), error.toStdString());
}

void GenericSerialView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (isTx) {
        monitor_->appendTx(data);
    } else {
        monitor_->appendRx(data);
    }
}

void GenericSerialView::onDtrChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue("serial_port/dtr", checked);
    }
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "setDtr", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::onRtsChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue("serial_port/rts", checked);
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

void GenericSerialView::startReconnectTimer() {
    if (!connectionWidget_ || !connectionWidget_->autoReconnectEnabled()) {
        return;
    }

    if (reconnectPolicy_.exhausted()) {
        monitor_->appendInfo(tr("Auto-reconnect exhausted (%1 attempts)")
                                 .arg(reconnectPolicy_.maxRetries()));
        stopReconnectTimer();
        return;
    }

    reconnectPolicy_.onFailed();
    const int delay = connectionWidget_->reconnectDelayMs();
    reconnectPolicy_.setDelayMs(delay);

    monitor_->appendInfo(tr("Auto-reconnect in %1ms (%2)")
                             .arg(delay)
                             .arg(reconnectPolicy_.statusString()));
    reconnectTimer_->start(delay);
}

void GenericSerialView::stopReconnectTimer() {
    reconnectTimer_->stop();
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

    monitor_->appendInfo(tr("Reconnecting to %1...").arg(reconnectConfig_.portName));

    QMetaObject::invokeMethod(worker_, "openSerial",
                              Qt::QueuedConnection,
                              Q_ARG(io::SerialConfig, reconnectConfig_));
}

void GenericSerialView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::generic_serial
