/**
 * @file BaseConnectionWidget.h
 * @brief Header file for BaseConnectionWidget, the base class of connection widgets.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QString>

class QCheckBox;
class QSpinBox;
class QLabel;
class QPushButton;
class QEvent;
class QHBoxLayout;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {
class CollapsibleSection;

/**
 * @brief Connection display states shared by all connection widget implementations.
 */
enum class DisplayState {
    Disconnected = 0,
    Connecting,
    TransportConnected,
    Connected,
    Disconnecting,
    Listening,
    Bound
};

/**
 * @brief Per-state display texts returned by getStateDisplayInfo().
 *
 * Hoisted from NetworkConnectionWidget (P2-44) so SerialConnectionWidget can
 * also participate in the applyDisplayState() template method without
 * duplicating the per-state switch.
 */
struct StateDisplayInfo {
    QString buttonText;
    QString statusText;
    QString statusStyle;
};

/**
 * @class BaseConnectionWidget
 * @brief Common base class for SerialConnectionWidget and NetworkConnectionWidget.
 *
 * Handles common behaviors such as collapsible section logic, auto-reconnect
 * configurations, settings group and settings loading/saving.
 *
 * @par Display-state template method (P2-44)
 *      applyDisplayState() is implemented in this base class as a template
 *      method: it queries getStateDisplayInfo() for the current state's
 *      button/status text and style, applies them to connectBtn_/statusLabel_,
 *      toggles autoReconnectCheck_/reconnectDelaySpin_, then delegates
 *      per-widget input enabling to applyInputWidgetsState() and protocol
 *      refresh to updateProtocolUi() (default no-op). Subclasses supply only
 *      the data + their own widget list, eliminating the per-state switch
 *      duplication that previously lived in SerialConnectionWidget.
 */
class BaseConnectionWidget : public QWidget {
    Q_OBJECT

public:
    using DisplayState = ui::widgets::DisplayState;

    explicit BaseConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~BaseConnectionWidget() override;

    /**
     * @brief Set the settings group key.
     * @param group The settings group name.
     */
    void setSettingsGroup(const QString& group);

    /**
     * @brief Check if auto-reconnect is enabled.
     * @return true if enabled, false otherwise.
     */
    [[nodiscard]] bool autoReconnectEnabled() const noexcept;

    /**
     * @brief Get the reconnect delay in milliseconds.
     * @return The reconnect delay in ms.
     */
    [[nodiscard]] int reconnectDelayMs() const noexcept;

signals:
    void disconnectClicked();

public slots:
    virtual void setConnected(bool connected) = 0;
    void setDisplayState(DisplayState state);

protected:
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;

    void createCommonWidgets(QWidget* parent);
    void setupCommonConnections();
    void loadCommonSettings();
    void saveCommonSettings();
    void retranslateCommonUi();

    /**
     * @brief Create the collapsible section and return the content layout.
     *        Subclasses add their own widgets to the returned layout, then call
     *        finishBaseUi() to finalize the common widgets and connections.
     * @return The content layout of the section widget.
     */
    QHBoxLayout* setupBaseUi();

    /**
     * @brief Finalize the base UI: add common widgets, stretch, section to main layout,
     *        setup common connections, load settings, and retranslate.
     * Called after the subclass has added its widgets to the layout from setupBaseUi().
     */
    void finishBaseUi(const QString& sectionSettingsKey = {});

    /**
     * @brief Retranslate common widgets and protocol-specific labels.
     *        Default implementation calls retranslateCommonUi() + applyDisplayState().
     *        Subclasses override to add their own label translations, calling
     *        BaseConnectionWidget::retranslateUi() first.
     */
    virtual void retranslateUi();

    void changeEvent(QEvent* event) override;

    // ---- Display-state template method (P2-44) ----
    // Skeleton: queries subclass for state-specific data, applies common
    // widgets, delegates input-widget enabling and protocol refresh.
    void applyDisplayState();

    /** @brief Return the display info (button/status text + style) for @p state. */
    [[nodiscard]] virtual StateDisplayInfo getStateDisplayInfo(DisplayState state) const = 0;

    /** @brief Enable/disable subclass-specific input widgets based on @p enabled. */
    virtual void applyInputWidgetsState(bool enabled) = 0;

    /** @brief Refresh protocol-specific labels / visibility. Default is no-op. */
    virtual void updateProtocolUi() {}

    /** @brief True when @p state should lock input widgets (i.e. not Disconnected). */
    [[nodiscard]] static bool inputsLocked(DisplayState state) noexcept;

    infra::config::ISettingsService* settingsService_ = nullptr;
    QString settingsGroup_;

    CollapsibleSection* section_ = nullptr;
    QCheckBox* autoReconnectCheck_ = nullptr;
    QSpinBox* reconnectDelaySpin_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;

    bool isConnected_ = false;
    DisplayState displayState_ = DisplayState::Disconnected;
};

} // namespace ui::widgets
