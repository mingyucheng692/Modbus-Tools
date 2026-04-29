#include "shell/MainWindowPageBuilder.h"

#include "common/ISettingsService.h"
#include "views/generic_serial/GenericSerialView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "views/modbus_tcp/ModbusTcpView.h"
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

MainWindowPageBuilder::MainWindowPageBuilder(common::ISettingsService* settingsService)
    : settingsService_(settingsService) {}

MainWindowPages MainWindowPageBuilder::build(QStackedWidget* stackedWidget, QWidget* owner) const {
    MainWindowPages pages;

    pages.modbusTcpView = new views::modbus_tcp::ModbusTcpView(settingsService_, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::ModbusTcp)] =
        stackedWidget->addWidget(createScrollablePage(pages.modbusTcpView, stackedWidget));

    pages.modbusRtuView = new views::modbus_rtu::ModbusRtuView(settingsService_, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::ModbusRtu)] =
        stackedWidget->addWidget(createScrollablePage(pages.modbusRtuView, stackedWidget));

    auto* genericTcpView = new views::generic_tcp::GenericTcpView(settingsService_, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::GenericTcp)] =
        stackedWidget->addWidget(createScrollablePage(genericTcpView, stackedWidget));

    auto* genericSerialView = new views::generic_serial::GenericSerialView(settingsService_, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::GenericSerial)] =
        stackedWidget->addWidget(createScrollablePage(genericSerialView, stackedWidget));

    pages.frameAnalyzer = new widgets::FrameAnalyzerWidget(settingsService_, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::FrameAnalyzer)] =
        stackedWidget->addWidget(createScrollablePage(pages.frameAnalyzer, stackedWidget));

    return pages;
}

} // namespace ui
