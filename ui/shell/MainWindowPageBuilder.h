// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <array>
#include <cstddef>

class QStackedWidget;
class QWidget;

namespace infra::config {
class ISettingsService;
}

namespace ui::views::modbus {
class ModbusPage;
}
 
namespace ui::views::tools {
class UtilitiesView;
}

namespace ui::views::analyzer {
class FrameAnalyzerView;
}

namespace ui {

enum class MainPage : std::size_t {
    Modbus = 0,
    NetworkDebugger,
    SerialDebugger,
    FrameAnalyzer,
    Utilities,
    Count
};

constexpr std::size_t kMainPageCount = static_cast<std::size_t>(MainPage::Count);

struct MainWindowPages {
    views::modbus::ModbusPage* modbusView = nullptr;
    views::analyzer::FrameAnalyzerView* frameAnalyzer = nullptr;
    views::tools::UtilitiesView* utilities = nullptr;
    std::array<int, kMainPageCount> pageIndexByNavigationRow = {};

    [[nodiscard]] int indexFor(MainPage page) const;
};

// Builds the main window's stacked pages and registers them with stackedWidget.
[[nodiscard]] MainWindowPages buildMainWindowPages(infra::config::ISettingsService* settingsService,
                                                    QStackedWidget* stackedWidget,
                                                    QWidget* owner);

} // namespace ui
