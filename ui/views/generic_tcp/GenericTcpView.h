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
#include "../ChannelController.h"
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
class ByteMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

class QThread;

namespace core::common {
class ISettingsService;
}

namespace ui::views::generic_tcp {

// ADR 0004 (design note): GenericTcpView / GenericSerialView deliberately hold
// their worker/channel directly and bypass the Presenter layer used by
// ModbusPage. Presenter is reserved for views with cross-cutting application
// state (session lifecycle, update flow, navigation); the generic_* views are
// self-contained channel pages with no shared presenter-worthy state, so a
// presenter here would be pure ceremony. Do not retro-fit a presenter without
// first extracting state worth abstracting.
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

private slots:
    void onConnectClicked(const QString& ip, int port);
    void onStartListenClicked(const QString& ip, int port);
    void onStopListenClicked();
    void onBindClicked(const QString& localIp, int localPort,
                       const QString& remoteIp, int remotePort);
    void onUnbindClicked();
    void onProtocolChanged(int index);
    
    void onSendRequested(const QByteArray& data) override;

    void onWorkerStateChanged(io::ChannelState state, quint64 generation);
    void onWorkerError(const QString& deviceHint, const QString& error);
    void onWorkerMonitor(bool isTx, const QByteArray& data);
    void onServerClientConnected(int clientId, const QString& peerInfo);
    void onServerClientDisconnected(int clientId);
    void onServerMonitorWithClient(bool isTx, const QByteArray& data, int clientId);
    void onServerStateChanged(io::ChannelState state);
    void onServerError(const QString& deviceHint, const QString& error);
    void onDisconnectSelectedClientsRequested(const QList<int>& clientIds);
    void onDisconnectAllClientsRequested();
    void onReconnectTimerTick();

private:
    void setupUi();
    void startWorker();
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

    // UI Components (moved from base class)
    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;

    // Channel controller (composite) – manages client worker thread + reconnect timer
    ChannelController channelCtrl_;

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