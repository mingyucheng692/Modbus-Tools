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
#include "modbus/base/ModbusAddressMapping.h"

class QCheckBox;
class QSpinBox;
class QLineEdit;
class QComboBox;
class QLabel;
class QTimer;
class QEvent;
class QString;
class QPoint;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

class ControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ControlWidget(infra::config::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ControlWidget() override;

    void recordTx();
    void recordRx(int rttMs);
    void recordError();
    void setSettingsGroup(const QString& group);
    void setLinked(bool active);
    void setPollingEnabled(bool enabled);
    void setInteractionsEnabled(bool enabled);
    int pollingIntervalMs() const;
    void setAddressBase(modbus::address::AddressBase base);

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
    infra::config::ISettingsService* settingsService_ = nullptr;

    modbus::address::AddressBase addressBase_ = modbus::address::AddressBase::Offset0Based;
    void updateAddressPlaceholder();
};

} // namespace ui::widgets
