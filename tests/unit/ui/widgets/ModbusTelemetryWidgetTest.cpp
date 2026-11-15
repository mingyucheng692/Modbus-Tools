// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QLabel>
#include <QPushButton>
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

    bool foundOverflowLabel = false;
    for (auto* lbl : widget_->findChildren<QLabel*>()) {
        if (lbl->text() == QStringLiteral("Queue overflow: 5 dropped")) {
            foundOverflowLabel = !lbl->isHidden();
            break;
        }
    }
    EXPECT_TRUE(foundOverflowLabel);

    // Process one batch of 200 items; oldest 5 (Event 0..4) must have been evicted
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    EXPECT_EQ(widget_->tableModel()->rowCount(), 200);
    ASSERT_NE(widget_->tableModel()->rowAt(0), nullptr);
    EXPECT_EQ(widget_->tableModel()->rowAt(0)->event.summary, QStringLiteral("Event 5"));
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
    bool foundUpdatedMetrics = false;
    for (auto* lbl : widget_->findChildren<QLabel*>()) {
        if (lbl->text() == QStringLiteral("TX: 100 | RX: 95 | Error: 5 | RTT: 12 ms")) {
            foundUpdatedMetrics = true;
            break;
        }
    }
    EXPECT_TRUE(foundUpdatedMetrics);

    widget_->updateMetrics(0, 0, 0, 0);
    bool foundResetMetrics = false;
    for (auto* lbl : widget_->findChildren<QLabel*>()) {
        if (lbl->text() == QStringLiteral("TX: 0 | RX: 0 | Error: 0 | RTT: 0 ms")) {
            foundResetMetrics = true;
            break;
        }
    }
    EXPECT_TRUE(foundResetMetrics);

    // Verify clear resets table and pending state
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

TEST_F(ModbusTelemetryWidgetTest, AutoScrollButtonLabelIsConstant) {
    auto* btn = widget_->autoScrollButton();
    ASSERT_NE(btn, nullptr);
    const QString textOn = btn->text();
    EXPECT_FALSE(textOn.contains(QStringLiteral(": ON")));
    EXPECT_FALSE(textOn.contains(QStringLiteral(": OFF")));

    widget_->setAutoScrollEnabled(false);
    const QString textOff = btn->text();
    EXPECT_EQ(textOn, textOff);
    EXPECT_FALSE(textOff.contains(QStringLiteral(": ON")));
    EXPECT_FALSE(textOff.contains(QStringLiteral(": OFF")));
}

TEST_F(ModbusTelemetryWidgetTest, ResponsiveToolbarTwoRowMode) {
    // Narrow layout (< 460px) triggers two-row mode
    widget_->updateResponsiveToolbar(350);
    EXPECT_TRUE(widget_->isTwoRowToolbar());

    // Wide layout (>= 490px) triggers one-row mode
    widget_->updateResponsiveToolbar(600);
    EXPECT_FALSE(widget_->isTwoRowToolbar());

    // Hysteresis deadband: at 475px, if already in wide mode, stays wide
    widget_->updateResponsiveToolbar(475);
    EXPECT_FALSE(widget_->isTwoRowToolbar());

    // At 475px, if in narrow mode, stays narrow
    widget_->updateResponsiveToolbar(400);
    EXPECT_TRUE(widget_->isTwoRowToolbar());
    widget_->updateResponsiveToolbar(475);
    EXPECT_TRUE(widget_->isTwoRowToolbar());

    // Verify all buttons retain sufficient size
    widget_->resize(320, 500);
    widget_->show();
    QCoreApplication::processEvents();

    auto* autoScrollBtn = widget_->autoScrollButton();
    auto* clearBtn = widget_->clearButton();
    auto* exportBtn = widget_->exportButton();
    auto* liveBtn = widget_->liveAnalyzerButton();

    ASSERT_NE(autoScrollBtn, nullptr);
    ASSERT_NE(clearBtn, nullptr);
    ASSERT_NE(exportBtn, nullptr);
    ASSERT_NE(liveBtn, nullptr);

    EXPECT_GE(autoScrollBtn->width(), autoScrollBtn->sizeHint().width());
    EXPECT_GE(clearBtn->width(), clearBtn->sizeHint().width());
    EXPECT_GE(exportBtn->width(), exportBtn->sizeHint().width());
    EXPECT_GE(liveBtn->width(), liveBtn->sizeHint().width());
}

TEST_F(ModbusTelemetryWidgetTest, KpiMetricsRibbonAndReset) {
    widget_->updateMetrics(15, 12, 3, 45);

    auto* txLbl = widget_->txStatsLabel();
    auto* rxLbl = widget_->rxStatsLabel();
    auto* errLbl = widget_->errStatsLabel();
    auto* rttLbl = widget_->rttStatsLabel();
    auto* resetBtn = widget_->resetStatsButton();

    ASSERT_NE(txLbl, nullptr);
    ASSERT_NE(rxLbl, nullptr);
    ASSERT_NE(errLbl, nullptr);
    ASSERT_NE(rttLbl, nullptr);
    ASSERT_NE(resetBtn, nullptr);

    EXPECT_TRUE(txLbl->text().contains(QStringLiteral("15")));
    EXPECT_TRUE(rxLbl->text().contains(QStringLiteral("12")));
    EXPECT_TRUE(errLbl->text().contains(QStringLiteral("3")));
    EXPECT_TRUE(rttLbl->text().contains(QStringLiteral("45 ms")));

    QSignalSpy resetSpy(widget_.get(), &ModbusTelemetryWidget::resetMetricsRequested);
    resetBtn->click();
    EXPECT_EQ(resetSpy.count(), 1);
}

TEST_F(ModbusTelemetryWidgetTest, InspectInAnalyzerWorkflow) {
    auto* inspectBtn = widget_->inspectInAnalyzerButton();
    ASSERT_NE(inspectBtn, nullptr);
    EXPECT_FALSE(inspectBtn->isEnabled());

    const QByteArray testPayload = QByteArray::fromHex("0103020064B844");
    TrafficEvent ev;
    ev.direction = TrafficDirection::Rx;
    ev.payload = testPayload;

    widget_->appendTrafficEvent(ev);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    ASSERT_EQ(widget_->tableModel()->rowCount(), 1);

    // Selecting the row enables the Inspect button
    widget_->tableView()->selectRow(0);
    EXPECT_TRUE(inspectBtn->isEnabled());

    // Clicking inspect button emits inspectInAnalyzerRequested
    QSignalSpy inspectSpy(widget_.get(), &ModbusTelemetryWidget::inspectInAnalyzerRequested);
    inspectBtn->click();
    ASSERT_EQ(inspectSpy.count(), 1);
    const QString hexArg = inspectSpy.takeFirst().at(0).toString();
    EXPECT_EQ(hexArg, QString::fromLatin1(testPayload.toHex(' ').toUpper()));

    // Double-clicking the row also emits inspectInAnalyzerRequested
    emit widget_->tableView()->doubleClicked(widget_->tableModel()->index(0, 0));
    ASSERT_EQ(inspectSpy.count(), 1);
    EXPECT_EQ(inspectSpy.takeFirst().at(0).toString(), QString::fromLatin1(testPayload.toHex(' ').toUpper()));

    // Deselect row disables inspect button
    widget_->tableView()->clearSelection();
    EXPECT_FALSE(inspectBtn->isEnabled());
}

