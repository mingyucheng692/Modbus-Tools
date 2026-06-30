/**
 * @file ModbusRtuPage.cpp
 * @brief Implementation of ModbusRtuPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusRtuPage.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include <spdlog/spdlog.h>

namespace ui::views::modbus_rtu {

ModbusRtuPage::ModbusRtuPage(ui::common::ISettingsService* settingsService, QWidget *parent)
    : BaseModbusPage(ui::application::modbus::SessionMode::Rtu, settingsService, parent) {
    
    auto* connWidget = new widgets::SerialConnectionWidget(settingsService_, this);
    connWidget->setSettingsGroup(QStringLiteral("modbus/rtu/serial"));
    
    setupUi(connWidget);

    connect(connWidget, &widgets::SerialConnectionWidget::connectClicked,
            this, &ModbusRtuPage::onConnectClicked);
    connect(connWidget, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &ModbusRtuPage::onDisconnectClicked);
}

ModbusRtuPage::~ModbusRtuPage() noexcept = default;

void ModbusRtuPage::onConnectClicked(const io::SerialConfig& config) {
    spdlog::info("ModbusRtuPage: Connect requested to {}", config.portName.toStdString());

    modbus::base::ModbusConfig modbusConfig;
    modbusConfig.mode = modbus::base::ModbusMode::RTU;
    modbusConfig.portName = config.portName;
    modbusConfig.baudRate = config.baudRate;
    modbusConfig.dataBits = config.dataBits;
    modbusConfig.stopBits = static_cast<int>(config.stopBits);
    modbusConfig.parity = static_cast<int>(config.parity);
    modbusConfig.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
    modbusConfig.timeoutMs = 1000;
    modbusConfig.retries = 0;
    modbusConfig.retryIntervalMs = 200;

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = modbusConfig;
    spec.serialConfig = config;
    sessionPresenter_->requestConnect(spec);
}

void ModbusRtuPage::onDisconnectClicked() {
    spdlog::info("ModbusRtuPage: Disconnect requested");
    sessionPresenter_->requestDisconnect();
}

} // namespace ui::views::modbus_rtu
