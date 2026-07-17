#include "shell/MainWindowPageBuilder.h"

#include "../../core/common/ISettingsService.h"
#include "views/generic_serial/GenericSerialView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/modbus/ModbusPage.h"
#include "widgets/FrameAnalyzerWidget.h"

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

MainWindowPages buildMainWindowPages(core::common::ISettingsService* settingsService,
                                     QStackedWidget* stackedWidget,
                                     QWidget* owner) {
    MainWindowPages pages;

    pages.modbusView = new views::modbus::ModbusPage(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Modbus)] =
        stackedWidget->addWidget(createScrollablePage(pages.modbusView, stackedWidget));

    auto* genericTcpView = new views::generic_tcp::GenericTcpView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::GenericTcp)] =
        stackedWidget->addWidget(createScrollablePage(genericTcpView, stackedWidget));

    auto* genericSerialView = new views::generic_serial::GenericSerialView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::GenericSerial)] =
        stackedWidget->addWidget(createScrollablePage(genericSerialView, stackedWidget));

    pages.frameAnalyzer = new widgets::FrameAnalyzerWidget(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::FrameAnalyzer)] =
        stackedWidget->addWidget(createScrollablePage(pages.frameAnalyzer, stackedWidget));

    return pages;
}

} // namespace ui
