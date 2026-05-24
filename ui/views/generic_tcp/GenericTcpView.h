/**
 * @file GenericTcpView.h
 * @brief Header file for GenericTcpView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QTimer>
#include <memory>
#include <QThread>
#include "../../../core/io/IChannel.h"
#include "../../../core/ReconnectPolicy.h"
#include "../../widgets/TcpConnectionWidget.h"

namespace io {
class ChannelOperationWorker;
class ServerChannelWorker;
}

namespace ui::widgets {
class TcpConnectionWidget;
class ByteMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QEvent;

namespace ui::common {
class ISettingsService;
}

namespace ui::views::generic_tcp {

class GenericTcpView : public QWidget {
    Q_OBJECT

public:
    explicit GenericTcpView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericTcpView() override;

private slots:
    void onConnectClicked(const QString& ip, int port);
    void onDisconnectClicked();
    void onStartListenClicked(const QString& ip, int port);
    void onStopListenClicked();
    void onBindClicked(const QString& localIp, int localPort,
                       const QString& remoteIp, int remotePort);
    void onUnbindClicked();
    void onProtocolChanged(widgets::TcpConnectionWidget::Protocol protocol);
    void onSendRequested(const QByteArray& data);
    void onFileSendRequested(const QString& filePath);
    void onWorkerStateChanged(io::ChannelState state, quint64 generation);
    void onWorkerError(const QString& deviceHint, const QString& error);
    void onWorkerMonitor(bool isTx, const QByteArray& data);
    void onServerClientConnected(int clientId, const QString& peerInfo);
    void onServerClientDisconnected(int clientId);
    void onServerMonitorWithClient(bool isTx, const QByteArray& data, int clientId);
    void onServerStateChanged(io::ChannelState state);
    void onServerError(const QString& deviceHint, const QString& error);

private:
    void setupUi();
    void startWorker();
    void stopWorker();
    void startServerWorker();
    void stopServerWorker();
    void switchToClientMode();
    void switchToServerMode();
    void switchToUdpMode();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    void startReconnectTimer();
    void stopReconnectTimer();
    void onReconnectTimerTick();

    // UI Components
    widgets::TcpConnectionWidget* connectionWidget_ = nullptr;
    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;

    // Backend - Client/UDP
    io::ChannelOperationWorker* worker_ = nullptr;
    QThread* workerThread_ = nullptr;

    // Backend - Server
    io::ServerChannelWorker* serverWorker_ = nullptr;
    QThread* serverThread_ = nullptr;

    widgets::TcpConnectionWidget::Protocol currentProtocol_ = 
        widgets::TcpConnectionWidget::Protocol::TcpClient;
    bool isConnected_ = false;
    bool suppressDisconnectAlert_ = false;
    quint64 connectionGeneration_ = 0;
    int lastLoggedFileTransferPct_ = -1;
    ui::common::ISettingsService* settingsService_ = nullptr;

    io::ReconnectPolicy reconnectPolicy_;
    QTimer* reconnectTimer_ = nullptr;
    QString reconnectHost_;
    int reconnectPort_ = 0;
};

} // namespace ui::views::generic_tcp
