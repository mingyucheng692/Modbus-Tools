/**
 * @file ModbusPage.h
 * @brief Single Modbus page supporting TCP/RTU/ASCII protocol switching.
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
#include "../../application/modbus/ModbusTypes.h"
#include "../../application/modbus/ModbusSessionPresenter.h"

class QVBoxLayout;
class QGroupBox;
class QTextEdit;
class QPushButton;
class QComboBox;
class QStackedWidget;
class QEvent;

namespace core::common {
class ISettingsService;
}

namespace ui::application::modbus {
class ModbusPagePresenter;
}

namespace ui::widgets {
class BaseConnectionWidget;
class TcpClientConnectionWidget;
class SerialConnectionWidget;
class FunctionWidget;
class TrafficMonitorWidget;
class ControlWidget;
class CollapsibleSection;
}

namespace ui::views::modbus {

/**
 * @brief Single Modbus debug page with in-page protocol switching.
 *
 * Replaces the former BaseModbusPage + 3 protocol-specific subclasses
 * (ModbusTcpPage / ModbusRtuPage / ModbusAsciiPage). A QComboBox selects
 * the active protocol; a QStackedWidget swaps the connection widget
 * (TcpClientConnectionWidget vs SerialConnectionWidget). The backend
 * services are rebuilt on protocol switch via ModbusPagePresenter::switchMode.
 *
 * Follows ADR 0004 (MVP): the View owns UI widgets; ModbusPagePresenter
 * owns backend services and signal routing.
 */
class ModbusPage : public QWidget {
    Q_OBJECT

public:
    explicit ModbusPage(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusPage() noexcept override;

    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);
    [[nodiscard]] bool isLinked() const;
    void appendTrafficData(bool isTx, const QByteArray& data);

    [[nodiscard]] ui::application::modbus::SessionMode currentMode() const noexcept { return currentMode_; }

signals:
    void linkageDataReceived(const ::modbus::base::Pdu& pdu, ::modbus::parser::ProtocolType protocol, uint16_t addr);
    void linkageToggled(bool active);
    void linkageSourceDisconnected();

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onProtocolChanged(int index);
    void onTcpConnectClicked(const QString& ip, int port);
    void onSerialConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();

private:
    void setupUi();
    void switchToProtocol(ui::application::modbus::SessionMode mode);
    void applyProtocolSettingsGroup(ui::application::modbus::SessionMode mode);
    void setupViewOnlyConnections();
    void retranslateUi();

    void appendReceiveData(const QByteArray& data);
    void appendSendData(const QByteArray& data);
    void refreshReceiveDisplay();
    void refreshSendDisplay();
    [[nodiscard]] QString formatData(const QByteArray& data, bool hex) const;

    // Layout
    QVBoxLayout* mainLayout_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QStackedWidget* connectionStack_ = nullptr;
    ui::widgets::TcpClientConnectionWidget* tcpConnectionWidget_ = nullptr;
    ui::widgets::SerialConnectionWidget* serialConnectionWidget_ = nullptr;
    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr; // currently active

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
    ui::application::modbus::ModbusSessionPresenter* sessionPresenter_ = nullptr;
    ui::application::modbus::ModbusPagePresenter* pagePresenter_ = nullptr;

    core::common::ISettingsService* settingsService_ = nullptr;
    ui::application::modbus::SessionMode currentMode_ = ui::application::modbus::SessionMode::Tcp;
};

} // namespace ui::views::modbus
