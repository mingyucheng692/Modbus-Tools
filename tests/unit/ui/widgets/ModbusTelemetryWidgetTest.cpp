// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QSignalSpy>
#include <QApplication>
#include <QTableView>
#include <QTemporaryDir>
#include "ui/widgets/modbus/ModbusTelemetryWidget.h"

using namespace ui::widgets::modbus;
using namespace ui::common;

class ModbusTelemetryWidgetTest : public ::testing::Test {
protected:
    void SetUp() override {
        widget_ = std::make_unique<ModbusTelemetryWidget>();
    }

    std::unique_ptr<ModbusTelemetryWidget> widget_;
};

TEST_F(ModbusTelemetryWidgetTest, InitialState) {
    EXPECT_TRUE(widget_->isAutoScrollEnabled());
    EXPECT_TRUE(widget_->isLinkageActive());
    EXPECT_NE(widget_->tableModel(), nullptr);
    EXPECT_NE(widget_->tableView(), nullptr);
    EXPECT_EQ(widget_->tableModel()->rowCount(), 0);
}

TEST_F(ModbusTelemetryWidgetTest, AppendEventAndProcessBatch) {
    TrafficEvent ev;
    ev.direction = TrafficDirection::Tx;
    ev.payload = QByteArray("\x01\x03\x00\x00\x00\x02", 6);

    widget_->appendTrafficEvent(ev);
    // Trigger batch timer slot directly to simulate timeout
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");

    EXPECT_EQ(widget_->tableModel()->rowCount(), 1);
    const auto* r = widget_->tableModel()->rowAt(0);
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->event.direction, TrafficDirection::Tx);
    EXPECT_EQ(r->event.payload, ev.payload);
}

TEST_F(ModbusTelemetryWidgetTest, QueueOverflowHardCeiling) {
    // Append 1005 events, exceeding kMaxPendingEvents (1000)
    for (int i = 0; i < 1005; ++i) {
        TrafficEvent ev;
        ev.summary = QString("Event %1").arg(i);
        widget_->appendTrafficEvent(ev);
    }

    // Process one batch of 200 items
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    EXPECT_EQ(widget_->tableModel()->rowCount(), 200);
}

TEST_F(ModbusTelemetryWidgetTest, LinkageStateAndSignal) {
    QSignalSpy spy(widget_.get(), &ModbusTelemetryWidget::linkageToggled);

    widget_->setLinkageState(false);
    EXPECT_FALSE(widget_->isLinkageActive());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toBool(), false);

    widget_->setLinkageState(true);
    EXPECT_TRUE(widget_->isLinkageActive());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toBool(), true);
}

TEST_F(ModbusTelemetryWidgetTest, MetricsUpdate) {
    widget_->updateMetrics(100, 95, 5, 12);
    // Verify clear resets internal state
    widget_->clear();
    EXPECT_EQ(widget_->tableModel()->rowCount(), 0);
}

TEST_F(ModbusTelemetryWidgetTest, AutoScrollToggle) {
    widget_->setAutoScrollEnabled(false);
    EXPECT_FALSE(widget_->isAutoScrollEnabled());
    widget_->setAutoScrollEnabled(true);
    EXPECT_TRUE(widget_->isAutoScrollEnabled());
}

TEST_F(ModbusTelemetryWidgetTest, InspectorFormatsDecodedValuesWithoutPrintfBug) {
    const QByteArray tcpPayload = QByteArray::fromHex("00010000000701030404D2162E");
    TrafficEvent ev;
    ev.direction = TrafficDirection::Rx;
    ev.payload = tcpPayload;

    widget_->setProtocolType(::modbus::parser::ProtocolType::Tcp);
    widget_->appendTrafficEvent(ev);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");

    ASSERT_EQ(widget_->tableModel()->rowCount(), 1);
    widget_->tableView()->selectRow(0);

    const QString details = widget_->inspectorDetailsText();
    EXPECT_TRUE(details.contains(QStringLiteral("[TID: 0x0001]")));
    EXPECT_TRUE(details.contains(QStringLiteral("[PID: 0x0000]")));
    EXPECT_TRUE(details.contains(QStringLiteral("[UID: 1]")));
    EXPECT_TRUE(details.contains(QStringLiteral("[FC: 0x03")));

    EXPECT_FALSE(details.contains(QStringLiteral("%-3d")));

    EXPECT_TRUE(details.contains(QStringLiteral("[+00] 1234")));
    EXPECT_TRUE(details.contains(QStringLiteral("[+01] 5678")));
}

TEST_F(ModbusTelemetryWidgetTest, BatchTimerRunsOnlyWhileEventsAreQueued) {
    EXPECT_FALSE(widget_->isBatchTimerActive());

    ::ui::common::TrafficEvent evt;
    evt.direction = ::ui::common::TrafficDirection::None;
    evt.summary = QStringLiteral("System notice");
    widget_->appendTrafficEvent(evt);
    EXPECT_TRUE(widget_->isBatchTimerActive());

    QElapsedTimer deadline;
    deadline.start();
    while (widget_->isBatchTimerActive() && deadline.elapsed() < 2000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }
    EXPECT_FALSE(widget_->isBatchTimerActive());
}

TEST_F(ModbusTelemetryWidgetTest, CopySelectedSysRowAndExportCsvSafety) {
    ::ui::common::TrafficEvent evt;
    evt.direction = ::ui::common::TrafficDirection::None;
    evt.summary = QStringLiteral("Line \"A\", ok");
    widget_->appendTrafficEvent(evt);

    QElapsedTimer deadline;
    deadline.start();
    while (widget_->isBatchTimerActive() && deadline.elapsed() < 2000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    auto* table = widget_->findChild<QTableView*>();
    ASSERT_NE(table, nullptr);
    ASSERT_EQ(table->model()->rowCount(), 1);
    table->selectRow(0);

    QSignalSpy copySpy(widget_.get(), &ModbusTelemetryWidget::rowCopied);
    ASSERT_TRUE(QMetaObject::invokeMethod(widget_.get(), "onCopySelectedRow", Qt::DirectConnection));
    ASSERT_EQ(copySpy.count(), 1);
    const QString copiedText = copySpy.takeFirst().at(0).toString();
    EXPECT_TRUE(copiedText.contains(QStringLiteral("SYS |")));
    EXPECT_TRUE(copiedText.contains(QStringLiteral("Line \"A\", ok")));
    EXPECT_FALSE(copiedText.contains(QStringLiteral("RX")));

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString csvPath = tempDir.filePath(QStringLiteral("telemetry.csv"));
    QString error;
    EXPECT_TRUE(widget_->exportToFile(csvPath, &error)) << error.toStdString();

    QFile csvFile(csvPath);
    ASSERT_TRUE(csvFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString csvContent = QString::fromUtf8(csvFile.readAll());
    EXPECT_TRUE(csvContent.contains(QStringLiteral("\"Line \"\"A\"\", ok\"")));

    EXPECT_FALSE(widget_->exportToFile(tempDir.path(), &error));
    EXPECT_FALSE(error.isEmpty());
}

