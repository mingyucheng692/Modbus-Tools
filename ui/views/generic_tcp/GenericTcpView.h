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
#include "../../../infra/io/IChannel.h"
#include <QList>

class QComboBox;
class QStackedWidget;

namespace io {
class ServerChannelWorker;
}

namespace ui::widgets {
class TcpClientConnectionWidget;
class TcpServerConnectionWidget;
class UdpConnectionWidget;
class ServerClientPanel;
}

class QThread;

namespace core::common {
class ISettingsService;
}

namespace ui::views::generic_tcp {

class GenericTcpView : public GenericChannelViewBase {
    Q_OBJECT

public:
    enum class Protocol {
        TcpClient = 0,
        TcpServer,
        Udp
    };
    Q_ENUM(Protocol)

    explicit GenericTcpView(core::common::ISettingsService* settingsService, QWidget *parent = nullptr);
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
    void onProtocolChanged(int index);
    
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
    void switchToProtocol(Protocol protocol);
    void retranslateUi() override;

    // Protocol selector
    QComboBox* protocolCombo_ = nullptr;

    // Stacked connection widgets
    QStackedWidget* connectionStack_ = nullptr;
    widgets::TcpClientConnectionWidget* tcpClientWidget_ = nullptr;
    widgets::TcpServerConnectionWidget* tcpServerWidget_ = nullptr;
    widgets::UdpConnectionWidget* udpWidget_ = nullptr;

    // Server panel
    widgets::ServerClientPanel* serverClientPanel_ = nullptr;

    // Backend - Server
    io::ServerChannelWorker* serverWorker_ = nullptr;
    QThread* serverThread_ = nullptr;

    Protocol currentProtocol_ = Protocol::TcpClient;
    bool suppressDisconnectAlert_ = false;
    quint64 connectionGeneration_ = 0;

    QString reconnectHost_;
    int reconnectPort_ = 0;
};

} // namespace ui::views::generic_tcp