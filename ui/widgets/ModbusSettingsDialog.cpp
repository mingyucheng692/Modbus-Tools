/**
 * @file ModbusSettingsDialog.cpp
 * @brief Implementation of ModbusSettingsDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusSettingsDialog.h"
#include "../../core/Config.h"
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <spdlog/spdlog.h>

namespace ui::widgets {

ModbusSettingsDialog::ModbusSettingsDialog(const Settings& current, QWidget* parent)
    : QDialog(parent), initialSettings_(current) {
    setupUi();
}

void ModbusSettingsDialog::setupUi() {
    setWindowTitle(QCoreApplication::translate("ui::MainWindow", "Modbus Settings"));
    auto* layout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    timeoutSpin_ = new QSpinBox(this);
    timeoutSpin_->setRange(config::Modbus::kMinTimeoutMs, config::Modbus::kMaxTimeoutMs);
    timeoutSpin_->setSingleStep(config::Modbus::kTimeoutStepMs);
    timeoutSpin_->setValue(initialSettings_.timeoutMs);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Request Timeout (ms):"), timeoutSpin_);

    retryEnableCheck_ = new QCheckBox(this);
    retryEnableCheck_->setChecked(initialSettings_.retryEnabled);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Enable Retry:"), retryEnableCheck_);

    retryCountSpin_ = new QSpinBox(this);
    retryCountSpin_->setRange(config::Modbus::kMinRetryCount, config::Modbus::kMaxRetryCount);
    retryCountSpin_->setValue(initialSettings_.retries);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Retry Count:"), retryCountSpin_);

    retryIntervalSpin_ = new QSpinBox(this);
    retryIntervalSpin_->setRange(config::Modbus::kMinRetryIntervalMs, config::Modbus::kMaxRetryIntervalMs);
    retryIntervalSpin_->setSingleStep(config::Modbus::kRetryIntervalStepMs);
    retryIntervalSpin_->setValue(initialSettings_.retryIntervalMs);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Retry Interval (ms):"), retryIntervalSpin_);

    auto updateRetryControls = [this](bool enabled) {
        retryCountSpin_->setEnabled(enabled);
        retryIntervalSpin_->setEnabled(enabled);
    };
    updateRetryControls(initialSettings_.retryEnabled);
    connect(retryEnableCheck_, &QCheckBox::toggled, updateRetryControls);

    logLevelCombo_ = new QComboBox(this);
    logLevelCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Debug"), static_cast<int>(spdlog::level::debug));
    logLevelCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Info"), static_cast<int>(spdlog::level::info));
    logLevelCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Warning"), static_cast<int>(spdlog::level::warn));
    logLevelCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Error"), static_cast<int>(spdlog::level::err));
    const int logIdx = logLevelCombo_->findData(initialSettings_.logLevel);
    logLevelCombo_->setCurrentIndex(logIdx >= 0 ? logIdx : static_cast<int>(spdlog::level::info));
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Log Level:"), logLevelCombo_);

    layout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ModbusSettingsDialog::Settings ModbusSettingsDialog::settings() const {
    return {
        timeoutSpin_->value(),
        retryCountSpin_->value(),
        retryIntervalSpin_->value(),
        retryEnableCheck_->isChecked(),
        logLevelCombo_->currentData().toInt()
    };
}

} // namespace ui::widgets
