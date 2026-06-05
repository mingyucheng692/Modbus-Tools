/**
 * @file ModbusTcpPage.cpp
 * @brief Implementation of ModbusTcpPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusTcpPage.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include <spdlog/spdlog.h>

namespace ui::views::modbus_tcp {

ModbusTcpPage::ModbusTcpPage(ui::common::ISettingsService* settingsService, QWidget *parent)
    : BaseModbusPage(ui::application::modbus::SessionMode::Tcp, settingsService, parent) {
    
    auto* connWidget = new widgets::TcpConnectionWidget(settingsService_, this);
    connWidget->setSettingsGroup(QStringLiteral("modbus/tcp"));
    
    setupUi(connWidget);

    connect(connWidget, &widgets::TcpConnectionWidget::connectClicked,
            this, &ModbusTcpPage::onConnectClicked);
    connect(connWidget, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &ModbusTcpPage::onDisconnectClicked);
}

ModbusTcpPage::~ModbusTcpPage() noexcept = default;

void ModbusTcpPage::onConnectClicked(const QString& ip, int port) {
    spdlog::info("ModbusTcpPage: Connect requested to {}:{}", ip.toStdString(), port);

    modbus::base::ModbusConfig config;
    config.mode = modbus::base::ModbusMode::TCP;
    config.ipAddress = ip;
    config.port = port;
    config.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
    config.timeoutMs = 1000;
    config.retries = 0;
    config.retryIntervalMs = 200;

    sessionPresenter_->connectTcp(ip, port, config);
}

void ModbusTcpPage::onDisconnectClicked() {
    spdlog::info("ModbusTcpPage: Disconnect requested");
    sessionPresenter_->requestDisconnect();
}

} // namespace ui::views::modbus_tcp
