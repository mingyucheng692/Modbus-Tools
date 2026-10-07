// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "BaseConnectionWidget.h"
#include "Config.h"

class QLineEdit;
class QSpinBox;
class QLabel;


namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

/**
 * @class NetworkConnectionWidget
 * @brief Common base for TcpConnectionWidget and UdpConnectionWidget.
 *
 * Provides shared IP/Port input fields, settings persistence, and collapsible section layout.
 * Subclasses only need to provide protocol-specific display data and button wiring.
 */
class NetworkConnectionWidget : public BaseConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit NetworkConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~NetworkConnectionWidget() override;

    void setDefaultPort(int port);
    [[nodiscard]] QString getIpAddress() const;
    [[nodiscard]] int getPort() const;

public slots:
    /** @brief Default implementation: maps to Disconnected or connectedState(). */
    void setConnected(bool connected) override;

protected:
    // ---- Settings ----
    void loadSettings() override;
    void saveSettings() override;
    void retranslateUi();

    // ---- UI Setup ----
    /** @brief Set up the common UI: IP/Port inputs, common widgets, section layout. */
    void setupCommonUi();

    /**
     * @brief Template method: calls setupCommonUi() → setupProtocolUi() →
     *        setupButtonConnection() → loadSettings() → updateProtocolUi() → retranslateUi().
     */
    void setupNetworkUi();

    /** @brief Subclass-specific UI setup called after setupCommonUi(). */
    virtual void setupProtocolUi() = 0;

    /** @brief Connect the button click to the appropriate signal. */
    virtual void setupButtonConnection() = 0;

    /** @brief Refresh protocol-specific label text and widget visibility.
     *         Pure virtual here so Network subclasses must implement it;
     *         BaseConnectionWidget provides a default no-op for non-Network
     *         subclasses (e.g. SerialConnectionWidget). */
    void updateProtocolUi() override = 0;

    // ---- Display State (data hooks) ----
    // applyDisplayState() skeleton and inputsLocked() live in BaseConnectionWidget.
    // NetworkConnectionWidget only contributes the per-protocol
    // data via the following pure-virtual hooks.

    /** @brief Return the display info for the given state. */
    [[nodiscard]] StateDisplayInfo getStateDisplayInfo(DisplayState state) const override = 0;

    /** @brief Enable/disable the IP/Port inputs (Network-specific widgets). */
    void applyInputWidgetsState(bool enabled) override;

    /** @brief Whether the given state is an "active" / connected state. */
    [[nodiscard]] virtual bool isActiveState(DisplayState state) const = 0;

    /** @brief The DisplayState that represents "fully connected" for this protocol. */
    [[nodiscard]] virtual DisplayState connectedState() const = 0;

    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    int defaultPort_ = config::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets
