/**
 * @file TrafficMonitorWidget.h
 * @brief Header file for TrafficMonitorWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QByteArray>

class QListWidget;
class QCheckBox;
class QPushButton;
class QEvent;
class QString;

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {
class CollapsibleSection;

class TrafficMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrafficMonitorWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~TrafficMonitorWidget() override;

    void appendTx(const QByteArray& data);
    void appendRx(const QByteArray& data);
    void appendInfo(const QString& message);
    void appendWarning(const QString& message);
    void clear();
    void setSettingsGroup(const QString& group);

private slots:
    void onSaveClicked();
    void onCopyClicked();

private:
    void setupUi();
    QString formatData(const QByteArray& data) const;
    void loadSettings();
    void saveSettings();
    void syncCollapsedGeometry(bool expanded);
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    CollapsibleSection* section_ = nullptr;
    QListWidget* logList_ = nullptr;
    QCheckBox* autoScrollCheck_ = nullptr;
    QCheckBox* showTxCheck_ = nullptr;
    QCheckBox* showRxCheck_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* saveBtn_ = nullptr;

    QString settingsGroup_;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets
