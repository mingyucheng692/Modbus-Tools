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

/**
 * @brief Builds the main window's stacked pages (Modbus, GenericTcp,
 *        GenericSerial, FrameAnalyzer) and registers them with @p stackedWidget.
 *
 * Replaces the former MainWindowPageBuilder class — a stateless 20-LOC factory
 * that did not earn its existence as a class.
 *
 * @param settingsService Settings service passed to each page constructor.
 * @param stackedWidget   Parent for scroll-area wrappers and page widgets.
 * @param owner           QObject parent for the created pages.
 * @return MainWindowPages with pointers and navigation-row indices.
 */
[[nodiscard]] MainWindowPages buildMainWindowPages(core::common::ISettingsService* settingsService,
                                                    QStackedWidget* stackedWidget,
                                                    QWidget* owner);

} // namespace ui
