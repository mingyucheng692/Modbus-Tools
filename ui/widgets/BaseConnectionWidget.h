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
#include "IConnectionWidget.h"

class QCheckBox;
class QSpinBox;
class QLabel;
class QPushButton;
class QEvent;

namespace core::common {
class ISettingsService;
}

namespace ui::widgets {
class CollapsibleSection;

/**
 * @class BaseConnectionWidget
 * @brief Common base class for SerialConnectionWidget and NetworkConnectionWidget.
 *
 * Handles common behaviors such as collapsible section logic, auto-reconnect configurations,
 * settings group and settings loading/saving.
 */
class BaseConnectionWidget : public QWidget, public IConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = ui::widgets::DisplayState;

    explicit BaseConnectionWidget(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
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
    void setDisplayState(DisplayState state) override;

protected:
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;

    void createCommonWidgets(QWidget* parent);
    void setupCommonConnections();
    void loadCommonSettings();
    void saveCommonSettings();
    void retranslateCommonUi();

    virtual void applyDisplayState() = 0;

    core::common::ISettingsService* settingsService_ = nullptr;
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
