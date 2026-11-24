// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "shell/MainWindowPageBuilder.h"

#include "infra/config/ISettingsService.h"
#include "views/serial/SerialPage.h"
#include "views/network/NetworkPage.h"
#include "views/modbus/ModbusPage.h"
#include "application/modbus/ModbusPagePresenter.h"
#include "views/analyzer/FrameAnalyzerView.h"
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
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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
    auto* modbusPresenter = new application::modbus::ModbusPagePresenter(
        pages.modbusView, ::modbus::base::ModbusMode::TCP, pages.modbusView);
    pages.modbusView->bindPresenter(modbusPresenter);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Modbus)] =
        stackedWidget->addWidget(pages.modbusView);

    pages.networkPage = new views::network::NetworkPage(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Network)] =
        stackedWidget->addWidget(pages.networkPage);

    pages.serialPage = new views::serial::SerialPage(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Serial)] =
        stackedWidget->addWidget(pages.serialPage);

    pages.frameAnalyzer = new views::analyzer::FrameAnalyzerView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::FrameAnalyzer)] =
        stackedWidget->addWidget(createScrollablePage(pages.frameAnalyzer, stackedWidget));

    pages.utilities = new views::tools::UtilitiesView(settingsService, owner);
    pages.pageIndexByNavigationRow[static_cast<std::size_t>(MainPage::Utilities)] =
        stackedWidget->addWidget(createScrollablePage(pages.utilities, stackedWidget));

    return pages;
}

} // namespace ui
