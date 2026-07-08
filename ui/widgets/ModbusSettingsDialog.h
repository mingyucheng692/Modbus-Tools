/**
 * @file ModbusSettingsDialog.h
 * @brief Header file for ModbusSettingsDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QDialog>

class QSpinBox;
class QCheckBox;
class QComboBox;

namespace ui::widgets {

/**
 * @brief Dialog for configuring global Modbus communication settings.
 */
class ModbusSettingsDialog : public QDialog {
    Q_OBJECT

public:
    struct Settings {
        int timeoutMs;
        int retries;
        int retryIntervalMs;
        bool retryEnabled;
        int logLevel;  // 0=Debug, 1=Info, 2=Warning, 3=Error
    };

    explicit ModbusSettingsDialog(const Settings& current, QWidget* parent = nullptr);

    Settings settings() const;

private:
    void setupUi();

    Settings initialSettings_;
    QSpinBox* timeoutSpin_ = nullptr;
    QSpinBox* retryCountSpin_ = nullptr;
    QSpinBox* retryIntervalSpin_ = nullptr;
    QCheckBox* retryEnableCheck_ = nullptr;
    QComboBox* logLevelCombo_ = nullptr;
};

} // namespace ui::widgets
