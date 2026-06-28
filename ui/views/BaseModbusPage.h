/**
 * @file BaseModbusPage.h
 * @brief Header file for BaseModbusPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "../application/modbus/ModbusSessionPresenter.h"

class QVBoxLayout;
class QGroupBox;
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
class RequestCoordinator;
}

namespace ui::widgets {
class BaseConnectionWidget;
class FunctionWidget;
class TrafficMonitorWidget;
class ControlWidget;
class CollapsibleSection;
}

namespace ui::views {

/**
 * @class BaseModbusPage
 * @brief Common base class for ModbusRtuPage and ModbusTcpPage.
 *
 * Consolidates the common UI layout, control logic, traffic logging, and service objects lifecycle.
 */
class BaseModbusPage : public QWidget {
    Q_OBJECT

public:
    explicit BaseModbusPage(ui::application::modbus::SessionMode mode,
                            ui::common::ISettingsService* settingsService,
                            QWidget* parent = nullptr);
    ~BaseModbusPage() noexcept override;

    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);
    [[nodiscard]] bool isLinked() const;

signals:

    void linkageDataReceived(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);
    void linkageToggled(bool active);
    void linkageSourceDisconnected();

protected:
    void setupUi(ui::widgets::BaseConnectionWidget* connWidget);
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    // Data monitor helpers
    void appendReceiveData(const QByteArray& data);
    void appendSendData(const QByteArray& data);
    void refreshReceiveDisplay();
    void refreshSendDisplay();
    [[nodiscard]] QString formatData(const QByteArray& data, bool hex) const;

    // UI Components
    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
    ui::widgets::CollapsibleSection* dataGroup_ = nullptr;
    QGroupBox* receiveGroup_ = nullptr;
    QGroupBox* sendGroup_ = nullptr;
    QTextEdit* receiveTextEdit_ = nullptr;
    QTextEdit* sendTextEdit_ = nullptr;
    QPushButton* copyReceiveButton_ = nullptr;
    QPushButton* copySendButton_ = nullptr;
    QPushButton* clearReceiveButton_ = nullptr;
    QPushButton* clearSendButton_ = nullptr;
    QByteArray lastReceiveFrame_;
    QByteArray lastSendFrame_;

    // Backend Services
    ui::application::modbus::RequestSubmissionService* requestService_ = nullptr;
    ui::application::modbus::PollingController* pollingController_ = nullptr;
    ui::application::modbus::TrafficLogController* trafficLogController_ = nullptr;
    ui::application::modbus::ModbusSessionPresenter* sessionPresenter_ = nullptr;
    ui::application::modbus::RequestCoordinator* requestCoordinator_ = nullptr;

    bool linked_ = false;
    ui::common::ISettingsService* settingsService_ = nullptr;
    ui::application::modbus::SessionMode sessionMode_;

private:
    void setupCommonConnections();
};

} // namespace ui::views
