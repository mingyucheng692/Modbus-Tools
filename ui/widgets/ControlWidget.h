/**
 * @file ControlWidget.h
 * @brief Header file for ControlWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <functional>

class QCheckBox;
class QSpinBox;
class QLineEdit;
class QComboBox;
class QLabel;
class QTimer;
class QEvent;
class QString;
class QPoint;

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {

class ControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ControlWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ControlWidget() override;

    void recordTx();
    void recordRx(int rttMs);
    void recordError();
    void resetStats();
    void setSettingsGroup(const QString& group);
    void setLinked(bool active);
    void setPollingEnabled(bool enabled);
    void setConnectionValidator(const std::function<bool()>& validator);
    int pollingIntervalMs() const;

signals:
    // Poll Requested: Function Code, Address, Quantity
    void pollRequested(uint8_t functionCode, int address, int quantity);
    void linkToggled(bool active);
    
    /**
     * @brief Request to log a message in the parent view.
     * @param message The message text.
     * @param isError True if it is an error message.
     */
    void logMessageRequested(const QString& message, bool isError);

private:
    void setupUi();
    void onTimer();
    void updateStatsLabel();
    bool isRttSegmentHovered(const QPoint& localPos) const;
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    QLabel* intervalLabel_ = nullptr;
    QLabel* fcLabel_ = nullptr;
    QLabel* addrLabel_ = nullptr;
    QLabel* qtyLabel_ = nullptr;
    QCheckBox* enablePollCheck_ = nullptr;
    QSpinBox* intervalSpin_ = nullptr;
    QComboBox* fcCombo_ = nullptr;
    QLineEdit* addrEdit_ = nullptr;
    QSpinBox* qtySpin_ = nullptr;
    QCheckBox* linkCheck_ = nullptr;
    
    QLabel* statsLabel_ = nullptr;
    
    QTimer* pollTimer_ = nullptr;

    // Stats
    int txCount_ = 0;
    int rxCount_ = 0;
    int errorCount_ = 0;
    int lastRtt_ = 0;
    bool hasLastRtt_ = false;

    QString settingsGroup_;
    ui::common::ISettingsService* settingsService_ = nullptr;
    std::function<bool()> connectionValidator_;

    bool skipAddrZeroWarning_ = false;
};

} // namespace ui::widgets
