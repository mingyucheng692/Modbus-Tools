/**
 * @file ModbusRtuPage.h
 * @brief Header file for ModbusRtuPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../BaseModbusPage.h"

namespace core::common {
class ISettingsService;
}

namespace ui::views::modbus_rtu {

class ModbusRtuPage : public BaseModbusPage {
    Q_OBJECT

public:
    explicit ModbusRtuPage(core::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ModbusRtuPage() noexcept override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
};

} // namespace ui::views::modbus_rtu
