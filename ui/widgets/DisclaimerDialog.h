/**
 * @file DisclaimerDialog.h
 * @brief Header file for DisclaimerDialog.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QDialog>

class QLabel;
class QPushButton;
class QFrame;

namespace ui::widgets {

class DisclaimerDialog : public QDialog {
    Q_OBJECT

public:
    explicit DisclaimerDialog(QWidget* parent = nullptr);

private:
    void setupUi();
    void applyLanguage(const QString& lang);

    QLabel* titleLabel_ = nullptr;
    QLabel* languageLabel_ = nullptr;
    QPushButton* langEnButton_ = nullptr;
    QPushButton* langZhCnButton_ = nullptr;
    QPushButton* langZhTwButton_ = nullptr;
    QFrame* separatorLine_ = nullptr;
    QLabel* contentLabel_ = nullptr;
    QPushButton* detailButton_ = nullptr;
    QPushButton* acceptButton_ = nullptr;
    QPushButton* exitButton_ = nullptr;
    QString currentLang_;
};

}
