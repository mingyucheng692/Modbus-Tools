/**
 * @file GenericInputWidget.h
 * @brief Header file for GenericInputWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QTimer>

class QTextEdit;
class QRadioButton;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QPushButton;
class QProgressBar;
class QEvent;
class QString;

namespace core::common {
class ISettingsService;
}

namespace ui::widgets {

class GenericInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit GenericInputWidget(core::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericInputWidget() override;

    void setSettingsGroup(const QString& group);

signals:
    void sendRequested(const QByteArray& data);

private slots:
    void onSendClicked();
    void onAutoSendToggled(bool checked);
    void onTimerTimeout();
    void onFormatChanged();
    void onHistoryActivated(int index);

private:
    void setupUi();
    QByteArray getData() const;
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    void addToHistory(const QString& text);
    void saveHistory();
    void loadHistory();

    QTextEdit* inputEdit_ = nullptr;
    QRadioButton* hexRadio_ = nullptr;
    QRadioButton* asciiRadio_ = nullptr;
    QCheckBox* autoSendCheck_ = nullptr;
    QSpinBox* intervalSpin_ = nullptr;
    QPushButton* sendBtn_ = nullptr;
    QComboBox* lineEndingCombo_ = nullptr;
    QComboBox* encodingCombo_ = nullptr;
    QComboBox* sendHistoryCombo_ = nullptr;

    QTimer* autoSendTimer_ = nullptr;
    QString settingsGroup_;
    core::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets
