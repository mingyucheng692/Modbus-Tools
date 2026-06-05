/**
 * @file TcpConnectionWidget.h
 * @brief Header file for TcpConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "BaseConnectionWidget.h"
#include "AppConstants.h"

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

class TcpConnectionWidget : public BaseConnectionWidget {
    Q_OBJECT

public:
    enum class Protocol {
        TcpClient = 0,
        TcpServer,
        Udp
    };
    Q_ENUM(Protocol)

    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit TcpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~TcpConnectionWidget() override;

    void setDefaultPort(int port);
    [[nodiscard]] QString getIpAddress() const;
    [[nodiscard]] int getPort() const;
    [[nodiscard]] Protocol currentProtocol() const;

signals:
    void connectClicked(const QString& ip, int port);
    void startListenClicked(const QString& ip, int port);
    void stopListenClicked();
    void bindClicked(const QString& localIp, int localPort,
                     const QString& remoteIp, int remotePort);
    void unbindClicked();
    void protocolChanged(Protocol protocol);

public slots:
    void setConnected(bool connected) override;

protected:
    void loadSettings() override;
    void saveSettings() override;
    void applyDisplayState() override;

private:
    void setupUi();
    void retranslateUi();
    void changeEvent(QEvent* event) override;
    void updateProtocolUi();

    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;

    QLabel* remoteIpLabel_ = nullptr;
    QLineEdit* remoteIpEdit_ = nullptr;
    QLabel* remotePortLabel_ = nullptr;
    QSpinBox* remotePortEdit_ = nullptr;

    Protocol currentProtocol_ = Protocol::TcpClient;
    int defaultPort_ = app::constants::Values::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets
