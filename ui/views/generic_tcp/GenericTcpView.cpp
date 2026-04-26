/**
 * @file GenericTcpView.cpp
 * @brief Implementation of GenericTcpView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericTcpView.h"
#include "AppConstants.h"
#include "../../common/ISettingsService.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TcpConnectionStateCoordinator.h"
#include "../../../core/io/ChannelOperationWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QThread>
#include <QMetaObject>
#include <QEvent>
#include <spdlog/spdlog.h>
#include "../../../core/io/IChannel.h"

namespace ui::views::generic_tcp {

GenericTcpView::GenericTcpView(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
    startWorker();
}

GenericTcpView::~GenericTcpView() {
    stopWorker();
}

void GenericTcpView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section (Top)
    connectionWidget_ = new widgets::TcpConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup("tcp_client");
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    mainLayout->addWidget(connectionWidget_);

    // 2. Central Area (Traffic Monitor)
    trafficMonitor_ = new widgets::TrafficMonitorWidget(settingsService_, this);
    trafficMonitor_->setSettingsGroup("tcp_client/traffic");
    mainLayout->addWidget(trafficMonitor_);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey("tcp_client/ui/inputCollapsed");
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup("tcp_client/input");
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 0);

    // Connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
            this, &GenericTcpView::onConnectClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked, 
            this, &GenericTcpView::onDisconnectClicked);
    
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested, 
            this, &GenericTcpView::onSendRequested);
    connect(inputWidget_, &widgets::GenericInputWidget::fileSendRequested,
            this, &GenericTcpView::onFileSendRequested);

    retranslateUi();
}

void GenericTcpView::startWorker() {
    workerThread_ = new QThread();
    auto* worker = new io::ChannelOperationWorker();
    worker->moveToThread(workerThread_);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);

    connect(worker, &io::ChannelOperationWorker::stateChangedWithGeneration, this, &GenericTcpView::onWorkerStateChanged);
    connect(worker, &io::ChannelOperationWorker::channelErrorOccurred, this, &GenericTcpView::onWorkerError);
    connect(worker, &io::ChannelOperationWorker::monitor, this, &GenericTcpView::onWorkerMonitor);
    connect(worker, &io::ChannelOperationWorker::fileTransferStarted, this, [this](const QString& filePath, qint64 totalBytes) {
        trafficMonitor_->appendInfo(tr("File transfer started: %1 (%2 bytes)").arg(filePath).arg(totalBytes));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferProgress, this, [this](const QString& filePath, qint64 sentBytes, qint64 totalBytes) {
        if (totalBytes <= 0) {
            return;
        }
        const int progress = static_cast<int>((sentBytes * 100) / totalBytes);
        if (progress == 100 || sentBytes == 0) {
            trafficMonitor_->appendInfo(tr("File transfer progress: %1 %2/%3")
                                            .arg(filePath)
                                            .arg(sentBytes)
                                            .arg(totalBytes));
        }
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferFinished, this, [this](const QString& filePath) {
        trafficMonitor_->appendInfo(tr("File transfer finished: %1").arg(filePath));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferFailed, this, [this](const QString& filePath, const QString& error) {
        trafficMonitor_->appendInfo(tr("File transfer failed: %1 (%2)").arg(filePath, error));
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferCanceled, this, [this](const QString& filePath) {
        trafficMonitor_->appendInfo(tr("File transfer canceled: %1").arg(filePath));
    });

    workerThread_->start();
    worker_ = worker;
}

void GenericTcpView::stopWorker() {
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

    suppressDisconnectAlert_ = true;
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

void GenericTcpView::onConnectClicked(const QString& ip, int port) {
    if (!worker_) return;
    
    spdlog::info("GenericTcp: Connecting to {}:{}", ip.toStdString(), port);
    suppressDisconnectAlert_ = false;
    const quint64 generation = ++connectionGeneration_;
    trafficMonitor_->appendInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    
    // Invoke openTcp on worker thread
    QMetaObject::invokeMethod(worker_, "openTcp", 
                              Qt::QueuedConnection, 
                              Q_ARG(QString, ip), 
                              Q_ARG(int, port),
                              Q_ARG(quint64, generation));
}

void GenericTcpView::onDisconnectClicked() {
    if (!worker_) return;
    
    suppressDisconnectAlert_ = true;
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericTcpView::onSendRequested(const QByteArray& data) {
    if (!worker_) return;
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }
    
    QMetaObject::invokeMethod(worker_, "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericTcpView::onFileSendRequested(const QString& filePath) {
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

void GenericTcpView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    if (generation != connectionGeneration_) {
        return;
    }

    const bool wasConnected = isConnected_;
    const auto transition = ui::common::TcpConnectionStateCoordinator::transition(
        state,
        wasConnected,
        suppressDisconnectAlert_);

    isConnected_ = (state == io::ChannelState::Open);
    connectionWidget_->setConnected(isConnected_);
    if (transition.clearSuppressDisconnectAlert) {
        suppressDisconnectAlert_ = false;
    }
    
    QString stateStr;
    switch (state) {
        case io::ChannelState::Closed: stateStr = tr("Closed"); break;
        case io::ChannelState::Opening: stateStr = tr("Opening"); break;
        case io::ChannelState::Open: stateStr = tr("Connected"); break;
        case io::ChannelState::Closing: stateStr = tr("Closing"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }
    
    trafficMonitor_->appendInfo(tr("State changed: %1").arg(stateStr));
    if (transition.showDisconnectAlert) {
        ui::common::ConnectionAlert::showDisconnected(this);
    }
    
}

void GenericTcpView::onWorkerError(const QString& deviceHint, const QString& error) {
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("TCP") : deviceHint;
    trafficMonitor_->appendError(tr("Error: %1").arg(error));
    spdlog::error("{} Error: {}", hint.toStdString(), error.toStdString());
}

void GenericTcpView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (isTx) {
        trafficMonitor_->appendTx(data);
    } else {
        trafficMonitor_->appendRx(data);
    }
}

void GenericTcpView::retranslateUi() {
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
}

void GenericTcpView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::generic_tcp
