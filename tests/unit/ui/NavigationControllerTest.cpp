// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "../../../core/Config.h"
#include "../../../ui/shell/NavigationController.h"
#include "../../../ui/MainWindow.h"
#include "../../../ui/common/ThemeController.h"
#include "../../../ui/views/tools/UtilitiesView.h"
#include "../../../ui/views/converter/ModbusFrameBuilderWidget.h"
#include "../../../ui/views/analyzer/FrameAnalyzerView.h"
#include "../../../ui/application/analyzer/FrameAnalyzerPresenter.h"
#include "../../../infra/platform/PathResolver.h"
#include "../../mocks/UiTestDoubles.h"

#include <QListWidget>
#include <QPushButton>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QToolButton>
#include <QWidget>

namespace {

TEST(NavigationControllerTest, BindToStackMapsRowsToPages) {
    QListWidget navigationList;
    QWidget navigationPane;
    QToolButton navigationToggleButton;
    ui::shell::NavigationController controller(&navigationList, &navigationPane, &navigationToggleButton);
    controller.initialize({QStringLiteral("TCP"), QStringLiteral("RTU")});

    QStackedWidget stackedWidget;
    QWidget firstPage;
    QWidget secondPage;
    stackedWidget.addWidget(&firstPage);
    stackedWidget.addWidget(&secondPage);

    controller.bindToStack(&stackedWidget, {1, 0});
    EXPECT_EQ(stackedWidget.currentIndex(), 1);

    navigationList.setCurrentRow(1);
    EXPECT_EQ(stackedWidget.currentIndex(), 0);
}

TEST(NavigationControllerTest, CollapseStateToggleUpdatesPaneAndLabels) {
    QListWidget navigationList;
    QWidget navigationPane;
    QToolButton navigationToggleButton;
    ui::shell::NavigationController controller(&navigationList, &navigationPane, &navigationToggleButton);
    controller.initialize({QStringLiteral("TCP"), QStringLiteral("RTU")});

    controller.setCollapsed(true);
    EXPECT_TRUE(controller.isCollapsed());
    EXPECT_EQ(navigationPane.width(), config::Ui::kNavigationCollapsedWidth);
    ASSERT_NE(navigationList.item(0), nullptr);
    EXPECT_TRUE(navigationList.item(0)->text().isEmpty());
    EXPECT_EQ(navigationToggleButton.text(), QStringLiteral("≫"));

    controller.setCollapsed(false);
    EXPECT_FALSE(controller.isCollapsed());
    EXPECT_EQ(navigationList.item(0)->text(), navigationList.item(0)->toolTip());
    EXPECT_EQ(navigationToggleButton.text(), QStringLiteral("≪"));
}

TEST(NavigationControllerTest, UtilitiesInspectChangesMainWindowPageAndParsesFrame) {
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());
    infra::platform::PathResolver resolver(sandbox.path());
    tests::mocks::FakeSettingsService settings;
    settings.setValue(QStringLiteral("app/disclaimerAccepted"), true);
    settings.setValue(QStringLiteral("app/updateCheckFrequency"), QStringLiteral("never"));
    settings.setValue(QStringLiteral("app/language"), QStringLiteral("en_US"));
    ui::common::ThemeController theme(settings);
    ui::MainWindow window(&settings, &theme, resolver);

    auto* tools = window.findChild<ui::views::tools::UtilitiesView*>();
    auto* analyzer = window.findChild<ui::views::analyzer::FrameAnalyzerView*>();
    QStackedWidget* stack = nullptr;
    for (auto* candidate : window.findChildren<QStackedWidget*>()) {
        if (candidate->count() == 5 && candidate->widget(3)->isAncestorOf(analyzer)) {
            stack = candidate;
        }
    }
    ASSERT_NE(tools, nullptr);
    ASSERT_NE(analyzer, nullptr);
    ASSERT_NE(stack, nullptr);
    auto* builder = tools->findChild<ui::views::converter::ModbusFrameBuilderWidget*>();
    auto* presenter = analyzer->findChild<ui::application::analyzer::FrameAnalyzerPresenter*>();
    ASSERT_NE(builder, nullptr);
    ASSERT_NE(presenter, nullptr);
    auto* inspectBtn = builder->findChild<QPushButton*>(QStringLiteral("builderInspect"));
    ASSERT_NE(inspectBtn, nullptr);

    QSignalSpy parsed(presenter, &ui::application::analyzer::FrameAnalyzerPresenter::parseFinished);
    inspectBtn->click();
    EXPECT_EQ(stack->currentIndex(), 3);
    EXPECT_TRUE(stack->currentWidget()->isAncestorOf(analyzer));
    ASSERT_TRUE(parsed.wait(3000));
    EXPECT_TRUE(qvariant_cast<modbus::parser::ParseResult>(parsed.at(0).at(0)).isValid);
}

} // namespace
