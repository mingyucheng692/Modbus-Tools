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
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TcpConnectionStateCoordinator.h"
#include "../../../core/io/ChannelOperationWorker.h"
#include "../../../core/io/ServerChannelWorker.h"
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
    startServerWorker();
}

GenericTcpView::~GenericTcpView() {
    stopServerWorker();
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
    monitor_ = new widgets::ByteMonitorWidget(settingsService_, this);
    monitor_->setSettingsGroup("tcp_client/traffic");
    mainLayout->addWidget(monitor_);

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

    // Client mode connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked,
            this, &GenericTcpView::onConnectClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &GenericTcpView::onDisconnectClicked);

    // Server mode connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::startListenClicked,
            this, &GenericTcpView::onStartListenClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::stopListenClicked,
            this, &GenericTcpView::onStopListenClicked);

    // UDP mode connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::bindClicked,
            this, &GenericTcpView::onBindClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::unbindClicked,
            this, &GenericTcpView::onUnbindClicked);

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

void GenericTcpView::startServerWorker() {
    serverThread_ = new QThread();
    auto* serverWorker = new io::ServerChannelWorker();
    serverWorker->moveToThread(serverThread_);
    connect(serverThread_, &QThread::finished, serverThread_, &QObject::deleteLater);

    connect(serverWorker, &io::ServerChannelWorker::clientConnected,
            this, &GenericTcpView::onServerClientConnected);
    connect(serverWorker, &io::ServerChannelWorker::clientDisconnected,
            this, &GenericTcpView::onServerClientDisconnected);
    connect(serverWorker, &io::ServerChannelWorker::monitorWithClient,
            this, &GenericTcpView::onServerMonitorWithClient);
    connect(serverWorker, &io::ServerChannelWorker::stateChanged,
            this, &GenericTcpView::onServerStateChanged);
    connect(serverWorker, &io::ServerChannelWorker::channelErrorOccurred,
            this, &GenericTcpView::onServerError);

    serverThread_->start();
    serverWorker_ = serverWorker;
}

void GenericTcpView::stopServerWorker() {
    auto* thread = serverThread_;
    auto* serverWorker = serverWorker_;
    serverThread_ = nullptr;
    serverWorker_ = nullptr;

    if (!thread) return;

    if (!serverWorker) {
        if (thread->isRunning()) {
            thread->quit();
        } else {
            delete thread;
        }
        return;
    }

    serverWorker->disconnect(this);

    if (!thread->isRunning()) {
        delete serverWorker;
        delete thread;
        return;
    }

    QMetaObject::invokeMethod(serverWorker, [serverWorker, thread]() {
        QObject::connect(serverWorker, &QObject::destroyed, thread, &QThread::quit, Qt::UniqueConnection);
        serverWorker->deleteLater();
    }, Qt::QueuedConnection);
}

void GenericTcpView::switchToClientMode() {
    currentProtocol_ = widgets::TcpConnectionWidget::Protocol::TcpClient;
    connectionWidget_->setSettingsGroup("tcp_client");
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    monitor_->setSettingsGroup("tcp_client/traffic");
    inputSection_->setSettingsKey("tcp_client/ui/inputCollapsed");
    inputWidget_->setSettingsGroup("tcp_client/input");
}

void GenericTcpView::switchToServerMode() {
    currentProtocol_ = widgets::TcpConnectionWidget::Protocol::TcpServer;
    connectionWidget_->setSettingsGroup("tcp_server");
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    monitor_->setSettingsGroup("tcp_server/traffic");
    inputSection_->setSettingsKey("tcp_server/ui/inputCollapsed");
    inputWidget_->setSettingsGroup("tcp_server/input");
}

void GenericTcpView::switchToUdpMode() {
    currentProtocol_ = widgets::TcpConnectionWidget::Protocol::Udp;
    connectionWidget_->setSettingsGroup("udp");
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    monitor_->setSettingsGroup("udp/traffic");
    inputSection_->setSettingsKey("udp/ui/inputCollapsed");
    inputWidget_->setSettingsGroup("udp/input");
}

void GenericTcpView::onConnectClicked(const QString& ip, int port) {
    if (!worker_) return;

    spdlog::info("GenericTcp: Connecting to {}:{}", ip.toStdString(), port);
    suppressDisconnectAlert_ = false;
    const quint64 generation = ++connectionGeneration_;
    monitor_->appendInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));

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

void GenericTcpView::onStartListenClicked(const QString& ip, int port) {
    if (!serverWorker_) return;

    spdlog::info("GenericTcp: Starting TCP server on {}:{}", ip.toStdString(), port);
    monitor_->appendInfo(tr("Starting TCP server on %1:%2...").arg(ip).arg(port));

    QMetaObject::invokeMethod(serverWorker_, "openTcpServer",
                              Qt::QueuedConnection,
                              Q_ARG(QString, ip),
                              Q_ARG(int, port),
                              Q_ARG(int, 0));
}

void GenericTcpView::onStopListenClicked() {
    if (!serverWorker_) return;

    monitor_->appendInfo(tr("Stopping TCP server..."));
    QMetaObject::invokeMethod(serverWorker_, "closeAllClients", Qt::QueuedConnection);
}

void GenericTcpView::onBindClicked(const QString& localIp, int localPort,
                                    const QString& remoteIp, int remotePort) {
    if (!worker_) return;

    spdlog::info("GenericTcp: Binding UDP {}:{}", localIp.toStdString(), localPort);
    if (!remoteIp.isEmpty()) {
        monitor_->appendInfo(tr("Binding UDP %1:%2 -> %3:%4...")
                                 .arg(localIp).arg(localPort)
                                 .arg(remoteIp).arg(remotePort));
    } else {
        monitor_->appendInfo(tr("Binding UDP %1:%2...").arg(localIp).arg(localPort));
    }

    suppressDisconnectAlert_ = false;

    QMetaObject::invokeMethod(worker_, "openUdp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, localIp),
                              Q_ARG(int, localPort),
                              Q_ARG(QString, remoteIp),
                              Q_ARG(int, remotePort));
}

void GenericTcpView::onUnbindClicked() {
    if (!worker_) return;

    suppressDisconnectAlert_ = true;
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericTcpView::onProtocolChanged(widgets::TcpConnectionWidget::Protocol protocol) {
    if (isConnected_) return;

    switch (protocol) {
    case widgets::TcpConnectionWidget::Protocol::TcpClient:
        switchToClientMode();
        break;
    case widgets::TcpConnectionWidget::Protocol::TcpServer:
        switchToServerMode();
        break;
    case widgets::TcpConnectionWidget::Protocol::Udp:
        switchToUdpMode();
        break;
    }
}

void GenericTcpView::onSendRequested(const QByteArray& data) {
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }

    switch (currentProtocol_) {
    case widgets::TcpConnectionWidget::Protocol::TcpClient:
    case widgets::TcpConnectionWidget::Protocol::Udp:
        if (!worker_) return;
        QMetaObject::invokeMethod(worker_, "write",
                                  Qt::QueuedConnection,
                                  Q_ARG(QByteArray, data));
        break;
    case widgets::TcpConnectionWidget::Protocol::TcpServer:
        if (!serverWorker_) return;
        monitor_->appendWarn(tr("Server mode: use client list to send data"));
        break;
    }
}

void GenericTcpView::onFileSendRequested(const QString& filePath) {
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }

    switch (currentProtocol_) {
    case widgets::TcpConnectionWidget::Protocol::TcpClient:
    case widgets::TcpConnectionWidget::Protocol::Udp:
        if (!worker_) return;
        QMetaObject::invokeMethod(worker_, "sendFile",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, filePath),
                                  Q_ARG(int, app::constants::Values::GenericIo::kFileSendChunkSizeBytes));
        break;
    case widgets::TcpConnectionWidget::Protocol::TcpServer:
        monitor_->appendWarn(tr("Server mode: file transfer not supported"));
        break;
    }
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

    monitor_->appendInfo(tr("State changed: %1").arg(stateStr));
    if (transition.showDisconnectAlert) {
        ui::common::ConnectionAlert::showDisconnected(this);
    }
}

void GenericTcpView::onWorkerError(const QString& deviceHint, const QString& error) {
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("TCP") : deviceHint;
    monitor_->appendError(tr("Error: %1").arg(error));
    spdlog::error("{} Error: {}", hint.toStdString(), error.toStdString());
}

void GenericTcpView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    monitor_->appendMessage(isTx, data);
}

void GenericTcpView::onServerClientConnected(int clientId, const QString& peerInfo) {
    monitor_->appendInfo(tr("Client #%1 connected: %2").arg(clientId).arg(peerInfo));
}

void GenericTcpView::onServerClientDisconnected(int clientId) {
    monitor_->appendInfo(tr("Client #%1 disconnected").arg(clientId));
}

void GenericTcpView::onServerMonitorWithClient(bool isTx, const QByteArray& data, int clientId) {
    monitor_->appendMessageWithClient(isTx, data, clientId);
}

void GenericTcpView::onServerStateChanged(io::ChannelState state) {
    isConnected_ = (state == io::ChannelState::Open);
    connectionWidget_->setConnected(isConnected_);

    QString stateStr;
    switch (state) {
        case io::ChannelState::Open: stateStr = tr("Listening"); break;
        case io::ChannelState::Closed: stateStr = tr("Stopped"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }

    monitor_->appendInfo(tr("Server state: %1").arg(stateStr));
}

void GenericTcpView::onServerError(const QString& deviceHint, const QString& error) {
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("TCP Server") : deviceHint;
    monitor_->appendError(tr("Server Error: %1").arg(error));
    spdlog::error("{} Error: {}", hint.toStdString(), error.toStdString());
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