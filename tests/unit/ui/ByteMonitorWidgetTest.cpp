// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "ui/widgets/ByteMonitorWidget.h"
#include "ui/widgets/LogListModel.h"
#include "core/Config.h"
#include "mocks/UiTestDoubles.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <functional>

namespace {

void pumpUntil(const std::function<bool()>& condition, int timeoutMs = 1000) {
    QElapsedTimer timer;
    timer.start();
    while (!condition() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

QPushButton* findButtonByText(QWidget& parent, const QString& text) {
    for (auto* btn : parent.findChildren<QPushButton*>()) {
        if (btn->text() == text) {
            return btn;
        }
    }
    return nullptr;
}

TEST(LogListModelTest, IsQObjectAndTruncatesOnReplace) {
    ui::widgets::LogListModel model(3);
    EXPECT_NE(qobject_cast<ui::widgets::LogListModel*>(&model), nullptr);

    QList<ui::widgets::LogEntry> batch;
    for (int i = 0; i < 5; ++i) {
        ui::widgets::LogEntry entry;
        entry.text = QStringLiteral("line-%1").arg(i);
        entry.message = entry.text;
        batch.append(entry);
    }
    model.replaceEntries(batch);
    ASSERT_EQ(model.rowCount(), 3);
    EXPECT_EQ(model.lineAt(0), QStringLiteral("line-2"));
    EXPECT_EQ(model.lineAt(2), QStringLiteral("line-4"));
    EXPECT_EQ(model.entryAt(1).message, QStringLiteral("line-3"));
}

TEST(ByteMonitorWidgetTest, ClientTagExcludedFromHexFormattingAndByteStats) {
    tests::mocks::FakeSettingsService settings;
    ui::widgets::ByteMonitorWidget widget(&settings);
    auto* listView = widget.findChild<QListView*>();
    ASSERT_NE(listView, nullptr);
    auto* model = qobject_cast<ui::widgets::LogListModel*>(listView->model());
    ASSERT_NE(model, nullptr);

    widget.appendMessageWithClient(true, QByteArray::fromHex("4142"), 7);
    pumpUntil([&]() { return model->rowCount() >= 1; });

    ASSERT_EQ(model->rowCount(), 1);
    const QString hexLine = model->lineAt(0);
    EXPECT_TRUE(hexLine.contains(QStringLiteral("[Client 7] 41 42")));
    EXPECT_FALSE(hexLine.contains(QStringLiteral("5B 43 6C")));

    const auto entry = model->entryAt(0);
    EXPECT_EQ(entry.rawBytes, QByteArray::fromHex("4142"));
    EXPECT_EQ(entry.clientLabel, QStringLiteral("[Client 7]"));

    bool foundExactTxStats = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("TX: 2 B")) {
            foundExactTxStats = true;
        }
    }
    EXPECT_TRUE(foundExactTxStats);

    auto* asciiBtn = findButtonByText(widget, QStringLiteral("ASCII"));
    auto* hexBtn = findButtonByText(widget, QStringLiteral("HEX"));
    ASSERT_NE(asciiBtn, nullptr);
    ASSERT_NE(hexBtn, nullptr);

    asciiBtn->click();
    EXPECT_TRUE(model->lineAt(0).contains(QStringLiteral("[Client 7] AB")));

    hexBtn->click();
    EXPECT_TRUE(model->lineAt(0).contains(QStringLiteral("[Client 7] 41 42")));
}

TEST(ByteMonitorWidgetTest, PauseFreezesModelAndCapsPendingFlood) {
    tests::mocks::FakeSettingsService settings;
    ui::widgets::ByteMonitorWidget widget(&settings);
    auto* listView = widget.findChild<QListView*>();
    ASSERT_NE(listView, nullptr);
    auto* model = qobject_cast<ui::widgets::LogListModel*>(listView->model());
    ASSERT_NE(model, nullptr);

    widget.appendInfo(QStringLiteral("initial"));
    pumpUntil([&]() { return model->rowCount() == 1; });

    auto* pauseBtn = findButtonByText(widget, QStringLiteral("Pause"));
    ASSERT_NE(pauseBtn, nullptr);
    pauseBtn->click();

    constexpr int kFloodCount = 2500;
    for (int i = 0; i < kFloodCount; ++i) {
        widget.appendInfo(QStringLiteral("flood-%1").arg(i));
    }
    pumpUntil([]() { return false; }, 160);
    EXPECT_EQ(model->rowCount(), 1);

    pauseBtn->click();
    EXPECT_GE(model->rowCount(), 2);
    EXPECT_LE(model->rowCount(), config::Ui::kTrafficMonitorMaxBlockCount + 2);
    EXPECT_TRUE(model->lineAt(1).contains(QStringLiteral("dropped")));
}

TEST(ByteMonitorWidgetTest, DestructorStopsTimerWithoutFlushingPending) {
    tests::mocks::FakeSettingsService settings;
    auto widget = std::make_unique<ui::widgets::ByteMonitorWidget>(&settings);
    widget->appendTx(QByteArray::fromHex("010203"));
    widget.reset();
    QCoreApplication::processEvents();
    SUCCEED();
}

} // namespace
