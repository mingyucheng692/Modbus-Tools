/**
 * @file ModbusSettingsDialog.cpp
 * @brief Implementation of ModbusSettingsDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusSettingsDialog.h"
#include "../../core/AppConstants.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>

namespace ui::widgets {

ModbusSettingsDialog::ModbusSettingsDialog(const Settings& current, QWidget* parent)
    : QDialog(parent), initialSettings_(current) {
    setupUi();
}

void ModbusSettingsDialog::setupUi() {
    using namespace app::constants;

    setWindowTitle(QCoreApplication::translate("ui::MainWindow", "Modbus Settings"));
    auto* layout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    timeoutSpin_ = new QSpinBox(this);
    timeoutSpin_->setRange(Values::Modbus::kMinTimeoutMs, Values::Modbus::kMaxTimeoutMs);
    timeoutSpin_->setSingleStep(Values::Modbus::kTimeoutStepMs);
    timeoutSpin_->setValue(initialSettings_.timeoutMs);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Request Timeout (ms):"), timeoutSpin_);

    retryEnableCheck_ = new QCheckBox(this);
    retryEnableCheck_->setChecked(initialSettings_.retryEnabled);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Enable Retry:"), retryEnableCheck_);

    retryCountSpin_ = new QSpinBox(this);
    retryCountSpin_->setRange(Values::Modbus::kMinRetryCount, Values::Modbus::kMaxRetryCount);
    retryCountSpin_->setValue(initialSettings_.retries);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Retry Count:"), retryCountSpin_);

    retryIntervalSpin_ = new QSpinBox(this);
    retryIntervalSpin_->setRange(Values::Modbus::kMinRetryIntervalMs, Values::Modbus::kMaxRetryIntervalMs);
    retryIntervalSpin_->setSingleStep(Values::Modbus::kRetryIntervalStepMs);
    retryIntervalSpin_->setValue(initialSettings_.retryIntervalMs);
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Retry Interval (ms):"), retryIntervalSpin_);

    auto updateRetryControls = [this](bool enabled) {
        retryCountSpin_->setEnabled(enabled);
        retryIntervalSpin_->setEnabled(enabled);
    };
    updateRetryControls(initialSettings_.retryEnabled);
    connect(retryEnableCheck_, &QCheckBox::toggled, updateRetryControls);

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
        retryEnableCheck_->isChecked()
    };
}

} // namespace ui::widgets
