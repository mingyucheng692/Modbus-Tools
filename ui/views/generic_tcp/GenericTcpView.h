/**
 * @file GenericTcpView.h
 * @brief Header file for GenericTcpView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../GenericChannelViewBase.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../../core/io/IChannel.h"
#include <QList>

namespace io {
class ServerChannelWorker;
}

namespace ui::widgets {
class TcpConnectionWidget;
class ServerClientPanel;
}

class QThread;

namespace ui::views::generic_tcp {

class GenericTcpView : public GenericChannelViewBase {
    Q_OBJECT

public:
    explicit GenericTcpView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericTcpView() noexcept override;

protected:
    void startWorker();

private slots:
    void onConnectClicked(const QString& ip, int port);
    void onStartListenClicked(const QString& ip, int port);
    void onStopListenClicked();
    void onBindClicked(const QString& localIp, int localPort,
                       const QString& remoteIp, int remotePort);
    void onUnbindClicked();
    void onProtocolChanged(widgets::TcpConnectionWidget::Protocol protocol);
    
    void onSendRequested(const QByteArray& data) override;
    void onFileSendRequested(const QString& filePath) override;
    
    void onWorkerStateChanged(io::ChannelState state, quint64 generation);
    void onServerClientConnected(int clientId, const QString& peerInfo);
    void onServerClientDisconnected(int clientId);
    void onServerMonitorWithClient(bool isTx, const QByteArray& data, int clientId);
    void onServerStateChanged(io::ChannelState state);
    void onServerError(const QString& deviceHint, const QString& error);
    void onDisconnectSelectedClientsRequested(const QList<int>& clientIds);
    void onDisconnectAllClientsRequested();
    void onReconnectTimerTick() override;

private:
    void setupUi();
    void startServerWorker();
    void stopServerWorker();
    void switchToClientMode();
    void switchToServerMode();
    void switchToUdpMode();
    void retranslateUi() override;

    // UI Components
    widgets::TcpConnectionWidget* connectionWidget_ = nullptr;
    widgets::ServerClientPanel* serverClientPanel_ = nullptr;

    // Backend - Server
    io::ServerChannelWorker* serverWorker_ = nullptr;
    QThread* serverThread_ = nullptr;

    widgets::TcpConnectionWidget::Protocol currentProtocol_ = 
        widgets::TcpConnectionWidget::Protocol::TcpClient;
    bool suppressDisconnectAlert_ = false;
    quint64 connectionGeneration_ = 0;

    QString reconnectHost_;
    int reconnectPort_ = 0;

private:
    void setupUi_internal(); // For any internal setup if needed
};

} // namespace ui::views::generic_tcp
