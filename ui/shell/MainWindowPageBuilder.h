#pragma once

#include <array>
#include <cstddef>

class QStackedWidget;
class QWidget;

namespace ui::common {
class ISettingsService;
}

namespace ui::views::modbus_tcp {
class ModbusTcpPage;
}

namespace ui::views::modbus_rtu {
class ModbusRtuPage;
}

namespace ui::widgets {
class FrameAnalyzerWidget;
}

namespace ui {

enum class MainPage : std::size_t {
    ModbusTcp = 0,
    ModbusRtu,
    GenericTcp,
    GenericSerial,
    FrameAnalyzer,
    Count
};

constexpr std::size_t kMainPageCount = static_cast<std::size_t>(MainPage::Count);

struct MainWindowPages {
    views::modbus_tcp::ModbusTcpPage* modbusTcpView = nullptr;
    views::modbus_rtu::ModbusRtuPage* modbusRtuView = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer = nullptr;
    std::array<int, kMainPageCount> pageIndexByNavigationRow = {};

    [[nodiscard]] int indexFor(MainPage page) const;
};

class MainWindowPageBuilder final {
public:
    explicit MainWindowPageBuilder(common::ISettingsService* settingsService);

    [[nodiscard]] MainWindowPages build(QStackedWidget* stackedWidget, QWidget* owner) const;

private:
    common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui
