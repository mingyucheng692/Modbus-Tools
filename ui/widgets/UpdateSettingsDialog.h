/**
 * @file UpdateSettingsDialog.h
 * @brief Header file for UpdateSettingsDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QDialog>

class QComboBox;

namespace ui::widgets {

class UpdateSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateSettingsDialog(const QString& currentFrequency, QWidget* parent = nullptr);
    QString selectedFrequency() const;

private:
    void setupUi();
    void applyFrequency(const QString& frequency);

    QComboBox* frequencyCombo_ = nullptr;
};

} // namespace ui::widgets
