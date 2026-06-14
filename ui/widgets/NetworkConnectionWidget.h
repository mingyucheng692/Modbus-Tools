/**
 * @file NetworkConnectionWidget.h
 * @brief Base class for network connection widgets (TCP Client, TCP Server, UDP).
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
class QLabel;
class QEvent;

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {

/**
 * @class NetworkConnectionWidget
 * @brief Common base for TcpClientConnectionWidget, TcpServerConnectionWidget, and UdpConnectionWidget.
 *
 * Provides shared IP/Port input fields, settings persistence, and collapsible section layout.
 * Subclasses specialize the button behavior, host label, and supported display states.
 */
class NetworkConnectionWidget : public BaseConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit NetworkConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~NetworkConnectionWidget() override;

    void setDefaultPort(int port);
    [[nodiscard]] QString getIpAddress() const;
    [[nodiscard]] int getPort() const;

public slots:
    void setConnected(bool connected) override = 0;

protected:
    void loadSettings() override;
    void saveSettings() override;
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    /** @brief Set up the common UI: IP/Port inputs, common widgets, section layout. */
    void setupCommonUi();

    /** @brief Subclass-specific UI setup called after setupCommonUi(). */
    virtual void setupProtocolUi() = 0;

    /** @brief Refresh protocol-specific label text and widget visibility. */
    virtual void updateProtocolUi() = 0;

    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    int defaultPort_ = app::constants::Values::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets