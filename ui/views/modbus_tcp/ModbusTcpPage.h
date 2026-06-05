/**
 * @file ModbusTcpPage.h
 * @brief Header file for ModbusTcpPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../BaseModbusPage.h"

namespace ui::views::modbus_tcp {

class ModbusTcpPage : public BaseModbusPage {
    Q_OBJECT

public:
    explicit ModbusTcpPage(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ModbusTcpPage() noexcept override;

private slots:
    void onConnectClicked(const QString& ip, int port);
    void onDisconnectClicked();
};

} // namespace ui::views::modbus_tcp
