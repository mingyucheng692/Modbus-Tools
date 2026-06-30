#pragma once

#include "../BaseModbusPage.h"

namespace ui::views::modbus_ascii {

class ModbusAsciiPage : public BaseModbusPage {
    Q_OBJECT

public:
    explicit ModbusAsciiPage(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusAsciiPage() noexcept override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
};

} // namespace ui::views::modbus_ascii
