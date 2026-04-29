/**
 * @file UpdateSettingsDialog.cpp
 * @brief Implementation of UpdateSettingsDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateSettingsDialog.h"

#include "../../core/AppConstants.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

namespace ui::widgets {

UpdateSettingsDialog::UpdateSettingsDialog(const QString& currentFrequency, QWidget* parent)
    : QDialog(parent) {
    setupUi();
    applyFrequency(currentFrequency);
}

QString UpdateSettingsDialog::selectedFrequency() const {
    return frequencyCombo_->currentData().toString();
}

void UpdateSettingsDialog::setupUi() {
    setWindowTitle(QCoreApplication::translate("ui::MainWindow", "Update Settings"));
    auto layout = new QVBoxLayout(this);
    auto formLayout = new QFormLayout();

    frequencyCombo_ = new QComboBox(this);
    frequencyCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Every Startup"),
                             QLatin1String(app::constants::Values::App::kUpdateCheckStartup));
    frequencyCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Weekly"),
                             QLatin1String(app::constants::Values::App::kUpdateCheckWeekly));
    frequencyCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Monthly"),
                             QLatin1String(app::constants::Values::App::kUpdateCheckMonthly));
    frequencyCombo_->addItem(QCoreApplication::translate("ui::MainWindow", "Disable Update Check"),
                             QLatin1String(app::constants::Values::App::kUpdateCheckNever));
    formLayout->addRow(QCoreApplication::translate("ui::MainWindow", "Update Check Frequency:"), frequencyCombo_);

    layout->addLayout(formLayout);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void UpdateSettingsDialog::applyFrequency(const QString& frequency) {
    const int targetIndex = frequencyCombo_->findData(frequency);
    frequencyCombo_->setCurrentIndex(targetIndex >= 0 ? targetIndex : 0);
}

} // namespace ui::widgets
