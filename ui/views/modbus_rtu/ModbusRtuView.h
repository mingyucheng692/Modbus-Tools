/**
 * @file ModbusRtuView.h
 * @brief Header file for ModbusRtuView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

class QVBoxLayout;
class QLabel;
class QGroupBox;
class QCheckBox;
class QTextEdit;
class QPushButton;
class QEvent;

namespace ui::common {
class ISettingsService;
}

namespace ui::application::modbus {
class RequestSubmissionService;
class PollingController;
class TrafficLogController;
class ModbusSessionPresenter;
}

namespace ui::widgets {
    class SerialConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
    class ControlWidget;
    class CollapsibleSection;
}

namespace ui::views::modbus_rtu {

class ModbusRtuView : public QWidget {
    Q_OBJECT

public:
    explicit ModbusRtuView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ModbusRtuView() override;
    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);
    bool isLinked() const;

signals:
    void pollRequested(uint8_t fc, int addr, int qty);
    void linkageDataReceived(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);
    void linkageToggled(bool active);
    void linkageSourceDisconnected();

private:
    void setupUi();
    void appendReceiveData(const QByteArray& data);
    void appendSendData(const QByteArray& data);
    void refreshReceiveDisplay();
    void refreshSendDisplay();
    QString formatData(const QByteArray& data, bool hex) const;
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
    ui::widgets::CollapsibleSection* dataGroup_ = nullptr;
    QGroupBox* receiveGroup_ = nullptr;
    QGroupBox* sendGroup_ = nullptr;
    QTextEdit* receiveTextEdit_ = nullptr;
    QTextEdit* sendTextEdit_ = nullptr;
    QCheckBox* receiveHexCheck_ = nullptr;
    QCheckBox* sendHexCheck_ = nullptr;
    QPushButton* copyReceiveButton_ = nullptr;
    QPushButton* copySendButton_ = nullptr;
    QPushButton* clearReceiveButton_ = nullptr;
    QPushButton* clearSendButton_ = nullptr;
    QByteArray lastReceiveFrame_;
    QByteArray lastSendFrame_;

    ui::application::modbus::RequestSubmissionService* requestService_ = nullptr;
    ui::application::modbus::PollingController* pollingController_ = nullptr;
    ui::application::modbus::TrafficLogController* trafficLogController_ = nullptr;
    ui::application::modbus::ModbusSessionPresenter* sessionPresenter_ = nullptr;
    bool linked_ = false;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::modbus_rtu
