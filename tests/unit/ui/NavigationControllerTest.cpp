#include <gtest/gtest.h>

#include "../../../core/AppConstants.h"
#include "../../../ui/shell/NavigationController.h"

#include <QCoreApplication>
#include <QListWidget>
#include <QStackedWidget>
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
    QCoreApplication::processEvents();
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
    EXPECT_EQ(navigationPane.width(), app::constants::Values::Ui::kNavigationCollapsedWidth);
    ASSERT_NE(navigationList.item(0), nullptr);
    EXPECT_TRUE(navigationList.item(0)->text().isEmpty());
    EXPECT_EQ(navigationToggleButton.text(), QStringLiteral("≫"));

    controller.setCollapsed(false);
    EXPECT_FALSE(controller.isCollapsed());
    EXPECT_EQ(navigationList.item(0)->text(), navigationList.item(0)->toolTip());
    EXPECT_EQ(navigationToggleButton.text(), QStringLiteral("≪"));
}

} // namespace
