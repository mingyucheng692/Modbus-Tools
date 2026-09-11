#include "shell/MainWindowPageBuilder.h"

#include "infra/config/ISettingsService.h"
#include "views/generic_serial/GenericSerialView.h"
#include "views/network/NetworkDebuggerView.h"
#include "views/modbus/ModbusPage.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "views/tools/UtilitiesView.h"

#include <QFrame>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>

namespace {

QWidget* createScrollablePage(QWidget* page, QStackedWidget* stackedWidget) {
    auto* scrollArea = new QScrollArea(stackedWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    scrollArea->setWidget(page);
    return scrollArea;
}

} // namespace

namespace ui {

int MainWindowPages::indexFor(MainPage page) const {
    return pageIndexByNavigationRow[static_cast<std::size_t>(page)];
}

MainWindowPages buildMainWindowPages(infra::config::ISettingsService* settingsService,
                                     QStackedWidget* stackedWidget,
                                     QWidget* owner) {
    MainWindowPages pages;

    pages.modbusView = new views::modbus::ModbusPage(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Modbus)] =
        stackedWidget->addWidget(createScrollablePage(pages.modbusView, stackedWidget));

    auto* networkDebuggerView = new views::network::NetworkDebuggerView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::NetworkDebugger)] =
        stackedWidget->addWidget(createScrollablePage(networkDebuggerView, stackedWidget));

    auto* genericSerialView = new views::generic_serial::GenericSerialView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::GenericSerial)] =
        stackedWidget->addWidget(createScrollablePage(genericSerialView, stackedWidget));

    pages.frameAnalyzer = new widgets::FrameAnalyzerWidget(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::FrameAnalyzer)] =
        stackedWidget->addWidget(createScrollablePage(pages.frameAnalyzer, stackedWidget));

    pages.utilities = new views::tools::UtilitiesView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Utilities)] =
        stackedWidget->addWidget(createScrollablePage(pages.utilities, stackedWidget));

    return pages;
}

} // namespace ui
