/**
 * @file TcpConnectionWidget.h
 * @brief Header file for TcpConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "AppConstants.h"
#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QComboBox;
class QCheckBox;
class QEvent;
class QString;

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {
class CollapsibleSection;

class TcpConnectionWidget : public QWidget {
    Q_OBJECT

public:
    enum class Protocol {
        TcpClient = 0,
        TcpServer,
        Udp
    };
    Q_ENUM(Protocol)

    enum class DisplayState {
        Disconnected = 0,
        Connecting,
        TransportConnected,
        Connected,
        Disconnecting,
        Listening,
        Bound
    };
    Q_ENUM(DisplayState)

    explicit TcpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~TcpConnectionWidget() override;

    void setDefaultPort(int port);
    void setSettingsGroup(const QString& group);
    QString getIpAddress() const;
    int getPort() const;
    Protocol currentProtocol() const;
    bool autoReconnectEnabled() const;
    int reconnectDelayMs() const;

signals:
    void connectClicked(const QString& ip, int port);
    void disconnectClicked();
    void startListenClicked(const QString& ip, int port);
    void stopListenClicked();
    void bindClicked(const QString& localIp, int localPort,
                     const QString& remoteIp, int remotePort);
    void unbindClicked();
    void protocolChanged(Protocol protocol);

public slots:
    void setConnected(bool connected);
    void setDisplayState(DisplayState state);

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;
    void updateProtocolUi();
    void applyDisplayState();

    CollapsibleSection* section_ = nullptr;
    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;

    QLabel* remoteIpLabel_ = nullptr;
    QLineEdit* remoteIpEdit_ = nullptr;
    QLabel* remotePortLabel_ = nullptr;
    QSpinBox* remotePortEdit_ = nullptr;

    QCheckBox* autoReconnectCheck_ = nullptr;
    QSpinBox* reconnectDelaySpin_ = nullptr;

    bool isConnected_ = false;
    DisplayState displayState_ = DisplayState::Disconnected;
    Protocol currentProtocol_ = Protocol::TcpClient;
    QString settingsGroup_ = "modbus/tcp";
    ui::common::ISettingsService* settingsService_ = nullptr;
    int defaultPort_ = app::constants::Values::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets
