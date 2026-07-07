#pragma once

#include "../BaseModbusPage.h"

namespace core::common {
class ISettingsService;
}

namespace ui::views::modbus_ascii {

class ModbusAsciiPage : public BaseModbusPage {
public:
    explicit ModbusAsciiPage(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusAsciiPage() noexcept override;

private:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
};

} // namespace ui::views::modbus_ascii
