#pragma once

#include <array>
#include <cstddef>

class QStackedWidget;
class QWidget;

namespace core::common {
class ISettingsService;
}

namespace ui::views::modbus {
class ModbusPage;
}

namespace ui::widgets {
class FrameAnalyzerWidget;
}

namespace ui {

enum class MainPage : std::size_t {
    Modbus = 0,
    GenericTcp,
    GenericSerial,
    FrameAnalyzer,
    Count
};

constexpr std::size_t kMainPageCount = static_cast<std::size_t>(MainPage::Count);

struct MainWindowPages {
    views::modbus::ModbusPage* modbusView = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer = nullptr;
    std::array<int, kMainPageCount> pageIndexByNavigationRow = {};

    [[nodiscard]] int indexFor(MainPage page) const;
};

class MainWindowPageBuilder final {
public:
    explicit MainWindowPageBuilder(core::common::ISettingsService* settingsService);

    [[nodiscard]] MainWindowPages build(QStackedWidget* stackedWidget, QWidget* owner) const;

private:
    core::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui
