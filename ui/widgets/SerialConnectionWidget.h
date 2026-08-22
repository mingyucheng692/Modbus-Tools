/**
 * @file SerialConnectionWidget.h
 * @brief Header file for SerialConnectionWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "BaseConnectionWidget.h"
#include <QSerialPort>
#include "../../../infra/io/SerialConfig.h" // For SerialConfig

class QComboBox;
class QPushButton;
class QLabel;

class QString;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

class SerialConnectionWidget : public BaseConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit SerialConnectionWidget(infra::config::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~SerialConnectionWidget() override;

    [[nodiscard]] io::SerialConfig getConfig() const;

signals:
    void connectClicked(const io::SerialConfig& config);

public slots:
    void setConnected(bool connected) override;
    void refreshPorts();

protected:
    void loadSettings() override;
    void saveSettings() override;

    // Display-state template method hooks (P2-44): Serial supplies the
    // per-state display strings and its own input-widget list; the skeleton
    // lives in BaseConnectionWidget::applyDisplayState().
    [[nodiscard]] StateDisplayInfo getStateDisplayInfo(DisplayState state) const override;
    void applyInputWidgetsState(bool enabled) override;

private:
    void setupUi();
    void retranslateUi();

    QLabel* portLabel_ = nullptr;
    QComboBox* portCombo_ = nullptr;
    QPushButton* refreshBtn_ = nullptr;
    QLabel* baudLabel_ = nullptr;
    QComboBox* baudCombo_ = nullptr;
    QLabel* dataBitsLabel_ = nullptr;
    QComboBox* dataBitsCombo_ = nullptr;
    QLabel* parityLabel_ = nullptr;
    QComboBox* parityCombo_ = nullptr;
    QLabel* stopBitsLabel_ = nullptr;
    QComboBox* stopBitsCombo_ = nullptr;
    QLabel* flowControlLabel_ = nullptr;
    QComboBox* flowControlCombo_ = nullptr;
};

} // namespace ui::widgets
