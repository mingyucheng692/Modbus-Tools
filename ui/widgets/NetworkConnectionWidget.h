/**
 * @file NetworkConnectionWidget.h
 * @brief Base class for network connection widgets (TCP Client, TCP Server, UDP).
 *
 * Uses the Template Method pattern: the base class owns the display-state update
 * skeleton (applyDisplayState, setupNetworkUi, setConnected), while subclasses
 * supply only the state-specific data via getStateDisplayInfo(), isActiveState(),
 * connectedState(), and setupButtonConnection().
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

namespace core::common {
class ISettingsService;
}

namespace ui::widgets {

/**
 * @brief Per-state display texts and visibility flags.
 *
 * Returned by getStateDisplayInfo() to decouple the applyDisplayState() skeleton
 * from subclass-specific labels.
 */
struct StateDisplayInfo {
    QString buttonText;
    QString statusText;
    QString statusStyle;
};

/**
 * @class NetworkConnectionWidget
 * @brief Common base for TcpClientConnectionWidget, TcpServerConnectionWidget, and UdpConnectionWidget.
 *
 * Provides shared IP/Port input fields, settings persistence, and collapsible section layout.
 * Subclasses only need to provide protocol-specific display data and button wiring.
 */
class NetworkConnectionWidget : public BaseConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit NetworkConnectionWidget(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
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
    void changeEvent(QEvent* event) override;

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

    /** @brief Refresh protocol-specific label text and widget visibility. */
    virtual void updateProtocolUi() = 0;

    // ---- Display State (Template Method) ----
    /**
     * @brief Skeleton implemented in base class. Calls getStateDisplayInfo() for
     *        the current displayState_, then updates common widgets.
     */
    void applyDisplayState() override final;

    /** @brief Return the display info for the given state. */
    [[nodiscard]] virtual StateDisplayInfo getStateDisplayInfo(DisplayState state) const = 0;

    /** @brief Whether the given state is an "active" / connected state. */
    [[nodiscard]] virtual bool isActiveState(DisplayState state) const = 0;

    /** @brief The DisplayState that represents "fully connected" for this protocol. */
    [[nodiscard]] virtual DisplayState connectedState() const = 0;

    // ---- Common helpers ----
    /** @brief Shared by all subclasses: locked unless Disconnected. */
    [[nodiscard]] static bool inputsLocked(DisplayState state);

    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    int defaultPort_ = app::constants::Values::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets