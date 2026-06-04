#include <gtest/gtest.h>

#include "../../../ui/widgets/ServerClientPanel.h"

#include <QCheckBox>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSignalSpy>

namespace {

void registerTestMetaTypes()
{
    qRegisterMetaType<QList<int>>("QList<int>");
}

} // namespace

TEST(ServerClientPanel, TracksClientsAndSelectionState)
{
    ui::widgets::ServerClientPanel panel;

    auto* list = panel.findChild<QListWidget*>(QStringLiteral("serverClientList"));
    auto* summaryLabel = panel.findChild<QLabel*>(QStringLiteral("serverClientSummaryLabel"));
    auto* broadcastCheck = panel.findChild<QCheckBox*>(QStringLiteral("serverBroadcastCheck"));

    ASSERT_NE(list, nullptr);
    ASSERT_NE(summaryLabel, nullptr);
    ASSERT_NE(broadcastCheck, nullptr);

    panel.addOrUpdateClient(1, QStringLiteral("127.0.0.1:502"));
    EXPECT_TRUE(panel.hasClients());
    ASSERT_EQ(list->count(), 1);
    EXPECT_EQ(panel.selectedClientIds(), QList<int>({1}));
    EXPECT_TRUE(summaryLabel->text().contains(QStringLiteral("Connected Clients: 1")));

    panel.addOrUpdateClient(2, QStringLiteral("127.0.0.1:503"));
    EXPECT_EQ(panel.allClientIds(), QList<int>({1, 2}));

    list->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
    EXPECT_EQ(panel.selectedClientIds(), QList<int>({2}));

    broadcastCheck->setChecked(true);
    EXPECT_TRUE(panel.broadcastEnabled());
    EXPECT_FALSE(list->isEnabled());
    EXPECT_TRUE(summaryLabel->text().contains(QStringLiteral("all clients")));

    EXPECT_TRUE(panel.removeClient(1));
    EXPECT_EQ(list->count(), 1);

    panel.clearClients();
    EXPECT_FALSE(panel.hasClients());
    EXPECT_EQ(list->count(), 0);
    EXPECT_FALSE(panel.broadcastEnabled());
}

TEST(ServerClientPanel, DisconnectActionsEmitExpectedSignals)
{
    registerTestMetaTypes();

    ui::widgets::ServerClientPanel panel;
    panel.addOrUpdateClient(10, QStringLiteral("192.168.0.10:502"));
    panel.addOrUpdateClient(11, QStringLiteral("192.168.0.11:502"));

    auto* list = panel.findChild<QListWidget*>(QStringLiteral("serverClientList"));
    auto* disconnectSelectedButton =
        panel.findChild<QPushButton*>(QStringLiteral("serverDisconnectSelectedButton"));
    auto* disconnectAllButton =
        panel.findChild<QPushButton*>(QStringLiteral("serverDisconnectAllButton"));

    ASSERT_NE(list, nullptr);
    ASSERT_NE(disconnectSelectedButton, nullptr);
    ASSERT_NE(disconnectAllButton, nullptr);

    QSignalSpy selectedSpy(&panel, &ui::widgets::ServerClientPanel::disconnectClientsRequested);
    QSignalSpy allSpy(&panel, &ui::widgets::ServerClientPanel::disconnectAllClientsRequested);

    list->selectAll();
    disconnectSelectedButton->click();

    ASSERT_EQ(selectedSpy.count(), 1);
    const QList<QVariant> selectedArguments = selectedSpy.takeFirst();
    ASSERT_EQ(selectedArguments.size(), 1);
    EXPECT_EQ(selectedArguments.at(0).value<QList<int>>(), QList<int>({10, 11}));

    disconnectAllButton->click();
    EXPECT_EQ(allSpy.count(), 1);
}
