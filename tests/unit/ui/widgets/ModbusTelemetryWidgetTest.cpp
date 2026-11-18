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
#include <QCheckBox>
#include <QComboBox>
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
    ASSERT_NE(widget_->txStatsLabel(), nullptr);
    ASSERT_NE(widget_->rxStatsLabel(), nullptr);
    ASSERT_NE(widget_->errStatsLabel(), nullptr);
    ASSERT_NE(widget_->rttStatsLabel(), nullptr);

    EXPECT_EQ(widget_->txStatsLabel()->text(), QStringLiteral("TX: 100"));
    EXPECT_EQ(widget_->rxStatsLabel()->text(), QStringLiteral("RX: 95"));
    EXPECT_EQ(widget_->errStatsLabel()->text(), QStringLiteral("ERR: 5"));
    EXPECT_EQ(widget_->rttStatsLabel()->text(), QStringLiteral("RTT: 12 ms"));

    // Legacy metricsLabel_ is completely removed
    EXPECT_EQ(widget_->findChild<QLabel*>(QStringLiteral("telemetryMetricsLabel")), nullptr);

    widget_->updateMetrics(0, 0, 0, 0);
    EXPECT_EQ(widget_->txStatsLabel()->text(), QStringLiteral("TX: 0"));
    EXPECT_EQ(widget_->rxStatsLabel()->text(), QStringLiteral("RX: 0"));
    EXPECT_EQ(widget_->errStatsLabel()->text(), QStringLiteral("ERR: 0"));
    EXPECT_EQ(widget_->rttStatsLabel()->text(), QStringLiteral("RTT: --"));

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
    const int collapse = widget_->singleRowToolbarRequiredWidth();
    const int expand = collapse + 30;

    // Narrow layout (< collapse) triggers two-row mode
    widget_->updateResponsiveToolbar(collapse - 50);
    EXPECT_TRUE(widget_->isTwoRowToolbar());

    // Wide layout (>= expand) triggers one-row mode
    widget_->updateResponsiveToolbar(expand + 50);
    EXPECT_FALSE(widget_->isTwoRowToolbar());

    // Hysteresis deadband: at collapse + 15, if already in wide mode, stays wide
    widget_->updateResponsiveToolbar(collapse + 15);
    EXPECT_FALSE(widget_->isTwoRowToolbar());

    // At collapse + 15, if in narrow mode, stays narrow
    widget_->updateResponsiveToolbar(collapse - 50);
    EXPECT_TRUE(widget_->isTwoRowToolbar());
    widget_->updateResponsiveToolbar(collapse + 15);
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

TEST_F(ModbusTelemetryWidgetTest, InPlaceDecodeAndByteOrder) {
    widget_->setProtocolType(::modbus::parser::ProtocolType::Rtu);

    // 0x03 Response: 2 registers (4 bytes: 41 20 00 00 -> 10.0 in Float32 ABCD)
    // ADU: [UID: 01] [FC: 03] [ByteCount: 04] [41 20 00 00] [CRC: EF C5]
    const QByteArray payload = QByteArray::fromHex("01030441200000EFC5");
    TrafficEvent ev;
    ev.direction = TrafficDirection::Rx;
    ev.payload = payload;

    widget_->appendTrafficEvent(ev);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    ASSERT_EQ(widget_->tableModel()->rowCount(), 1);

    widget_->tableView()->selectRow(0);

    auto* decodeCombo = widget_->decodeTypeCombo();
    auto* orderCombo = widget_->orderCombo();
    ASSERT_NE(decodeCombo, nullptr);
    ASSERT_NE(orderCombo, nullptr);

    EXPECT_TRUE(decodeCombo->isEnabled());
    EXPECT_TRUE(orderCombo->isEnabled());

    // Switch to Float32 ABCD
    widget_->setDecodeType(::modbus::analyzer::RegisterDataType::Float32);
    widget_->setRegisterOrder(::modbus::base::RegisterOrder::ABCD);

    QString text = widget_->inspectorDetailsText();
    EXPECT_TRUE(text.contains(QStringLiteral("Float32")));
    EXPECT_TRUE(text.contains(QStringLiteral("ABCD")));
    EXPECT_TRUE(text.contains(QStringLiteral("10")));

    // Switch to CDAB
    widget_->setRegisterOrder(::modbus::base::RegisterOrder::CDAB);
    text = widget_->inspectorDetailsText();
    EXPECT_TRUE(text.contains(QStringLiteral("CDAB")));
}

TEST_F(ModbusTelemetryWidgetTest, SemanticGuardMatrix) {
    widget_->setProtocolType(::modbus::parser::ProtocolType::Rtu);

    auto* decodeCombo = widget_->decodeTypeCombo();
    auto* orderCombo = widget_->orderCombo();
    ASSERT_NE(decodeCombo, nullptr);
    ASSERT_NE(orderCombo, nullptr);

    // 1. 0x06 Write Single Register: Decode is enabled (16-bit), Order is disabled
    TrafficEvent ev06;
    ev06.direction = TrafficDirection::Tx;
    ev06.payload = QByteArray::fromHex("010600010064D9E1"); // Write reg 1 with 100
    widget_->appendTrafficEvent(ev06);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    widget_->tableView()->selectRow(0);

    EXPECT_TRUE(decodeCombo->isEnabled());
    EXPECT_FALSE(orderCombo->isEnabled());

    // 2. 0x01 Read Coils RX: Bits operation -> Decode and Order both disabled
    widget_->clear();
    TrafficEvent ev01;
    ev01.direction = TrafficDirection::Rx;
    ev01.payload = QByteArray::fromHex("01010105918B"); // 1 byte coils (Coil 0=1, Coil 1=0, Coil 2=1)
    widget_->appendTrafficEvent(ev01);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    widget_->tableView()->selectRow(0);

    EXPECT_FALSE(decodeCombo->isEnabled());
    EXPECT_FALSE(orderCombo->isEnabled());
    EXPECT_TRUE(widget_->inspectorDetailsText().contains(QStringLiteral("ON")));
    EXPECT_TRUE(widget_->inspectorDetailsText().contains(QStringLiteral("OFF")));

    // 3. 0x03 Read Request TX: No register data payload -> Decode and Order disabled
    widget_->clear();
    TrafficEvent ev03Tx;
    ev03Tx.direction = TrafficDirection::Tx;
    ev03Tx.payload = QByteArray::fromHex("010300000002C40B");
    widget_->appendTrafficEvent(ev03Tx);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    widget_->tableView()->selectRow(0);

    EXPECT_FALSE(decodeCombo->isEnabled());
    EXPECT_FALSE(orderCombo->isEnabled());

    // 4. 0x10 Write Multiple Registers Echo RX: No data payload -> Decode and Order disabled
    widget_->clear();
    TrafficEvent ev10Rx;
    ev10Rx.direction = TrafficDirection::Rx;
    ev10Rx.payload = QByteArray::fromHex("01100000000241C8");
    widget_->appendTrafficEvent(ev10Rx);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    widget_->tableView()->selectRow(0);

    EXPECT_FALSE(decodeCombo->isEnabled());
    EXPECT_FALSE(orderCombo->isEnabled());

    // 5. Exception Response (0x83): No data payload -> Decode and Order disabled
    widget_->clear();
    TrafficEvent evEx;
    evEx.direction = TrafficDirection::Rx;
    evEx.payload = QByteArray::fromHex("018302C0F1");
    widget_->appendTrafficEvent(evEx);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    widget_->tableView()->selectRow(0);

    EXPECT_FALSE(decodeCombo->isEnabled());
    EXPECT_FALSE(orderCombo->isEnabled());
}

TEST_F(ModbusTelemetryWidgetTest, MetadataScaleAndDescriptionRendering) {
    widget_->setProtocolType(::modbus::parser::ProtocolType::Rtu);

    // 0x03 Response with register value = 200 (0x00C8) at address 0
    TrafficEvent ev;
    ev.direction = TrafficDirection::Rx;
    ev.payload = QByteArray::fromHex("01030200C8B9D2");
    widget_->appendTrafficEvent(ev);
    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");

    // Inject metadata: scale = 0.5, description = "WaterTemperature"
    QMap<uint16_t, ::modbus::analyzer::DataMetadata> metaMap;
    ::modbus::analyzer::DataMetadata meta;
    meta.scale = 0.5;
    meta.description = QStringLiteral("WaterTemperature");
    metaMap.insert(0, meta);
    widget_->setMetadataMap(metaMap);

    widget_->tableView()->selectRow(0);
    const QString details = widget_->inspectorDetailsText();

    EXPECT_TRUE(details.contains(QStringLiteral("WaterTemperature")));
    EXPECT_TRUE(details.contains(QStringLiteral("100"))); // 200 * 0.5 = 100
}

TEST_F(ModbusTelemetryWidgetTest, FilterComboboxSeverityFiltering) {
    widget_->setProtocolType(::modbus::parser::ProtocolType::Rtu);

    auto* combo = widget_->filterComboBox();
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->count(), 7);
    EXPECT_EQ(combo->itemText(0), QStringLiteral("All"));
    EXPECT_EQ(combo->itemText(1), QStringLiteral("TX"));
    EXPECT_EQ(combo->itemText(2), QStringLiteral("RX"));
    EXPECT_EQ(combo->itemText(3), QStringLiteral("SYS"));
    EXPECT_EQ(combo->itemText(4), QStringLiteral("Err & Warn"));
    EXPECT_EQ(combo->itemText(5), QStringLiteral("Errors"));
    EXPECT_EQ(combo->itemText(6), QStringLiteral("Warnings"));

    // 1. TX event
    TrafficEvent evTx;
    evTx.direction = TrafficDirection::Tx;
    evTx.payload = QByteArray::fromHex("010300000002C40B");
    widget_->appendTrafficEvent(evTx);

    // 2. RX success event (CRC 0x322A -> little-endian bytes: 2A 32)
    TrafficEvent evRx;
    evRx.direction = TrafficDirection::Rx;
    evRx.payload = QByteArray::fromHex("010304000100022A32");
    widget_->appendTrafficEvent(evRx);

    // 3. SYS info event
    TrafficEvent evInfo;
    evInfo.direction = TrafficDirection::None;
    evInfo.level = TrafficEventLevel::Info;
    evInfo.summary = QStringLiteral("Poll Summary: 1 ok, 0 err");
    widget_->appendTrafficEvent(evInfo);

    // 4. SYS warning event
    TrafficEvent evWarn;
    evWarn.direction = TrafficDirection::None;
    evWarn.level = TrafficEventLevel::Warning;
    evWarn.summary = QStringLiteral("Poll Warning: 1 consecutive failure(s): Request timed out");
    widget_->appendTrafficEvent(evWarn);

    // 5. SYS error event
    TrafficEvent evErr;
    evErr.direction = TrafficDirection::None;
    evErr.level = TrafficEventLevel::Error;
    evErr.summary = QStringLiteral("Error: Request timed out");
    widget_->appendTrafficEvent(evErr);

    // 6. RX Exception event
    TrafficEvent evEx;
    evEx.direction = TrafficDirection::Rx;
    evEx.payload = QByteArray::fromHex("018302C0F1");
    widget_->appendTrafficEvent(evEx);

    QMetaObject::invokeMethod(widget_.get(), "processBatchTimeout");
    ASSERT_EQ(widget_->tableModel()->rowCount(), 6);

    auto* proxy = widget_->tableView()->model();
    ASSERT_NE(proxy, nullptr);

    // Index 0: All -> 6
    combo->setCurrentIndex(0);
    EXPECT_EQ(proxy->rowCount(), 6);

    // Index 1: TX -> 1
    combo->setCurrentIndex(1);
    EXPECT_EQ(proxy->rowCount(), 1);

    // Index 2: RX -> 2 (evRx, evEx)
    combo->setCurrentIndex(2);
    EXPECT_EQ(proxy->rowCount(), 2);

    // Index 3: SYS -> 3 (evInfo, evWarn, evErr)
    combo->setCurrentIndex(3);
    EXPECT_EQ(proxy->rowCount(), 3);

    // Index 4: Err & Warn -> 3 (evWarn, evErr, evEx)
    combo->setCurrentIndex(4);
    EXPECT_EQ(proxy->rowCount(), 3);

    // Index 5: Errors -> 2 (evErr, evEx; evWarn excluded)
    combo->setCurrentIndex(5);
    EXPECT_EQ(proxy->rowCount(), 2);

    // Index 6: Warnings -> 1 (evWarn; evErr and evEx excluded)
    combo->setCurrentIndex(6);
    EXPECT_EQ(proxy->rowCount(), 1);
}

TEST_F(ModbusTelemetryWidgetTest, PollFramesToggleAndTooltip) {
    auto* btn = widget_->pollFramesButton();
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(btn->objectName(), QStringLiteral("telemetryPollFramesBtn"));
    EXPECT_EQ(btn->text(), QStringLiteral("Poll Frames"));
    EXPECT_TRUE(btn->isCheckable());
    EXPECT_FALSE(btn->isChecked());
    EXPECT_FALSE(btn->toolTip().isEmpty());

    QSignalSpy spy(widget_.get(), &ModbusTelemetryWidget::pollFramesToggled);
    btn->setChecked(true);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());

    btn->setChecked(false);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.takeFirst().at(0).toBool());
}

TEST_F(ModbusTelemetryWidgetTest, TwoRowToolbar_AdjacentSpacingIsUniform) {
    widget_->show();
    widget_->resize(420, 500);
    QCoreApplication::processEvents();

    EXPECT_TRUE(widget_->isTwoRowToolbar());
    auto* combo = widget_->filterComboBox();
    auto* pollBtn = widget_->pollFramesButton();
    auto* autoScroll = widget_->autoScrollButton();
    auto* clearBtn = widget_->clearButton();
    auto* exportBtn = widget_->exportButton();
    auto* liveBtn = widget_->liveAnalyzerButton();

    ASSERT_NE(combo, nullptr);
    ASSERT_NE(pollBtn, nullptr);
    ASSERT_NE(autoScroll, nullptr);
    ASSERT_NE(clearBtn, nullptr);
    ASSERT_NE(exportBtn, nullptr);
    ASSERT_NE(liveBtn, nullptr);

    // Row 1 adjacent spacing
    const int gapComboToPoll = pollBtn->geometry().left() - combo->geometry().right();
    const int gapPollToScroll = autoScroll->geometry().left() - pollBtn->geometry().right();
    EXPECT_GT(gapComboToPoll, 0);
    EXPECT_LE(gapComboToPoll, 12);
    EXPECT_GT(gapPollToScroll, 0);
    EXPECT_LE(gapPollToScroll, 12);

    // Row 2 adjacent spacing (eliminates previous wide empty gap)
    const int gapClearToExport = exportBtn->geometry().left() - clearBtn->geometry().right();
    const int gapExportToLive = liveBtn->geometry().left() - exportBtn->geometry().right();
    EXPECT_GT(gapClearToExport, 0);
    EXPECT_LE(gapClearToExport, 12);
    EXPECT_GT(gapExportToLive, 0);
    EXPECT_LE(gapExportToLive, 12);
}

TEST_F(ModbusTelemetryWidgetTest, OrderComboWidth_RemainsCompactAtLargeWidths) {
    widget_->resize(1000, 600);
    widget_->show();
    QCoreApplication::processEvents();

    auto* orderCombo = widget_->orderCombo();
    ASSERT_NE(orderCombo, nullptr);
    EXPECT_LE(orderCombo->width(), orderCombo->sizeHint().width() + 8);
}

TEST_F(ModbusTelemetryWidgetTest, ToolbarAndHeader_ContinuousSweepZeroClipping) {
    widget_->show();

    // Continuously sweep from 260px to 720px in 10px steps (including 460px~580px critical band)
    for (int width = 260; width <= 720; width += 10) {
        widget_->resize(width, 500);
        QCoreApplication::processEvents();

        // Check toolbar buttons and controls
        auto* toolbar = widget_->findChild<QWidget*>(QStringLiteral("telemetryToolbar"));
        ASSERT_NE(toolbar, nullptr);
        for (auto* btn : toolbar->findChildren<QPushButton*>()) {
            if (btn->isVisible()) {
                EXPECT_GE(btn->width(), btn->minimumSizeHint().width())
                    << "Button " << btn->objectName().toStdString() << " clipped at width " << width;
            }
        }
        for (auto* cb : toolbar->findChildren<QComboBox*>()) {
            if (cb->isVisible()) {
                EXPECT_GE(cb->width(), cb->minimumSizeHint().width())
                    << "Combo " << cb->objectName().toStdString() << " clipped at width " << width;
            }
        }
        for (auto* chk : toolbar->findChildren<QCheckBox*>()) {
            if (chk->isVisible()) {
                EXPECT_GE(chk->width(), chk->minimumSizeHint().width())
                    << "CheckBox " << chk->objectName().toStdString() << " clipped at width " << width;
            }
        }

        // Check inspector header controls
        auto* header = widget_->findChild<QWidget*>(QStringLiteral("inspectorHeaderWidget"));
        if (!header && widget_->orderCombo()) {
            header = widget_->orderCombo()->parentWidget();
        }
        if (header) {
            for (auto* btn : header->findChildren<QPushButton*>()) {
                if (btn->isVisible()) {
                    EXPECT_GE(btn->width(), btn->minimumSizeHint().width())
                        << "Header button " << btn->objectName().toStdString() << " clipped at width " << width;
                }
            }
            for (auto* cb : header->findChildren<QComboBox*>()) {
                if (cb->isVisible()) {
                    EXPECT_GE(cb->width(), cb->minimumSizeHint().width())
                        << "Header combo " << cb->objectName().toStdString() << " clipped at width " << width;
                }
            }
        }
    }
}


