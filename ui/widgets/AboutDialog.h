/**
 * @file AboutDialog.h
 * @brief Header file for AboutDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QDialog>

class QLabel;
class QDialogButtonBox;

namespace ui::widgets {

/**
 * @brief Dialog showing application information and links.
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    void setupUi();
};

} // namespace ui::widgets
