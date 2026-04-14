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
    explicit TcpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~TcpConnectionWidget() override;

    void setDefaultPort(int port);
    void setSettingsGroup(const QString& group);
    QString getIpAddress() const;
    int getPort() const;

signals:
    void connectClicked(const QString& ip, int port);
    void disconnectClicked();

public slots:
    void setConnected(bool connected);

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    CollapsibleSection* section_ = nullptr;
    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    
    bool isConnected_ = false;
    QString settingsGroup_ = "modbus/tcp";
    ui::common::ISettingsService* settingsService_ = nullptr;
    int defaultPort_ = app::constants::Values::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets
