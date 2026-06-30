#pragma once

#include "../BaseModbusPage.h"

namespace ui::views::modbus_ascii {

class ModbusAsciiPage : public BaseModbusPage {
public:
    explicit ModbusAsciiPage(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusAsciiPage() noexcept override;

private:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
};

} // namespace ui::views::modbus_ascii
