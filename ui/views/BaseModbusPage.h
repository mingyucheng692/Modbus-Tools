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
#include "../application/modbus/ModbusTypes.h"
#include "../application/modbus/ModbusSessionPresenter.h"

class QVBoxLayout;
class QGroupBox;
class QTextEdit;
class QPushButton;
class QEvent;

namespace core::common {
class ISettingsService;
}

namespace ui::application::modbus {
class ModbusPagePresenter;
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
 * Delegates backend service ownership and signal routing to
 * ModbusPagePresenter (ADR 0004 MVP pattern). The View is responsible
 * only for UI layout, rendering, and user input.
 */
class BaseModbusPage : public QWidget {
    Q_OBJECT

public:
    explicit BaseModbusPage(ui::application::modbus::SessionMode mode,
                            core::common::ISettingsService* settingsService,
                            QWidget* parent = nullptr);
    ~BaseModbusPage() noexcept override;

    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);
    [[nodiscard]] bool isLinked() const;

    void appendTrafficData(bool isTx, const QByteArray& data);

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

    // Non-owning reference to the session presenter (owned by pagePresenter_).
    // Kept for subclass access (requestConnect / requestDisconnect).
    ui::application::modbus::ModbusSessionPresenter* sessionPresenter_ = nullptr;

    core::common::ISettingsService* settingsService_ = nullptr;
    ui::application::modbus::SessionMode sessionMode_;

private:
    void setupViewOnlyConnections();

    ui::application::modbus::ModbusPagePresenter* pagePresenter_ = nullptr;
};

} // namespace ui::views
