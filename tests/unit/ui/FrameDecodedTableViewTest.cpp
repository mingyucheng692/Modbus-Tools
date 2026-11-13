// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "widgets/FrameDecodedTableView.h"
#include "widgets/ModbusSettingsDialog.h"
#include "widgets/RegisterTypeDelegate.h"
#include "modbus/parser/ModbusFrameParser.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTemporaryDir>
#include <spdlog/spdlog.h>

namespace {

::modbus::parser::ParseResult makeValidHoldingRegisterResult() {
    ::modbus::parser::ParseResult res;
    res.isValid = true;
    res.protocol = ::modbus::parser::ProtocolType::Rtu;
    res.type = ::modbus::parser::FrameType::Response;
    res.slaveId = 1;
    res.functionCode = ::modbus::base::FunctionCode::ReadHoldingRegisters;
    ::modbus::parser::DataItem d0;
    d0.address = 0;
    d0.rawBytes = QByteArray::fromHex("42F6");
    d0.hexString = QStringLiteral("42F6");
    ::modbus::parser::DataItem d1;
    d1.address = 1;
    d1.rawBytes = QByteArray::fromHex("E979");
    d1.hexString = QStringLiteral("E979");
    res.dataItems = {d0, d1};
    res.rawFrame = QByteArray::fromHex("01030442F6E979ABCD");
    return res;
}

} // namespace

TEST(FrameDecodedTableViewTest, DelegateDefaultAndCustomCommitTriggersSingleRecalcAndPreservesDefaultLabel) {
    ui::widgets::FrameDecodedTableView table;
    table.renderData(makeValidHoldingRegisterResult(),
                     ::modbus::analyzer::RegisterDataType::UInt16,
                     ::modbus::base::RegisterOrder::ABCD,
                     ::modbus::address::AddressBase::Offset0Based);
    ASSERT_EQ(table.rowCount(), 2);

    auto* typeItem = table.item(0, 4);
    ASSERT_NE(typeItem, nullptr);
    EXPECT_EQ(typeItem->text(), QStringLiteral("Default (UInt16)"));

    QSignalSpy metadataSpy(&table, &ui::widgets::FrameDecodedTableView::metadataChanged);
    auto* delegate = qobject_cast<ui::widgets::RegisterTypeDelegate*>(table.itemDelegateForColumn(4));
    ASSERT_NE(delegate, nullptr);

    const QModelIndex idx0 = table.model()->index(0, 4);
    std::unique_ptr<QWidget> editor(delegate->createEditor(&table, QStyleOptionViewItem{}, idx0));
    auto* combo = qobject_cast<QComboBox*>(editor.get());
    ASSERT_NE(combo, nullptr);

    // Selecting Default (-1) when already Default should not emit redundant updates.
    combo->setCurrentIndex(combo->findData(-1));
    delegate->setModelData(combo, table.model(), idx0);
    EXPECT_EQ(metadataSpy.count(), 0);
    EXPECT_EQ(table.item(0, 4)->text(), QStringLiteral("Default (UInt16)"));

    // Selecting Float32 emits exactly once.
    const int float32Idx = combo->findData(static_cast<int>(::modbus::analyzer::RegisterDataType::Float32));
    ASSERT_GE(float32Idx, 0);
    combo->setCurrentIndex(float32Idx);
    delegate->setModelData(combo, table.model(), idx0);
    EXPECT_EQ(metadataSpy.count(), 1);
    EXPECT_EQ(table.item(0, 4)->text(), QStringLiteral("Float32"));

    // Resetting back to Default emits once and preserves "Default (UInt16)".
    combo->setCurrentIndex(combo->findData(-1));
    delegate->setModelData(combo, table.model(), table.model()->index(0, 4));
    EXPECT_EQ(metadataSpy.count(), 2);
    EXPECT_EQ(table.item(0, 4)->text(), QStringLiteral("Default (UInt16)"));
}

TEST(FrameDecodedTableViewTest, ScaleValidationRejectsNonFiniteAndZeroValues) {
    ui::widgets::FrameDecodedTableView table;
    table.renderData(makeValidHoldingRegisterResult(),
                     ::modbus::analyzer::RegisterDataType::UInt16,
                     ::modbus::base::RegisterOrder::ABCD,
                     ::modbus::address::AddressBase::Offset0Based);
    ASSERT_EQ(table.rowCount(), 2);

    auto* scaleItem = table.item(0, 5);
    ASSERT_NE(scaleItem, nullptr);
    const QString initialScale = scaleItem->text();

    for (const QString& badScale : {QStringLiteral("nan"), QStringLiteral("inf"), QStringLiteral("-inf"), QStringLiteral("0"), QStringLiteral("0.0")}) {
        scaleItem->setText(badScale);
        EXPECT_EQ(scaleItem->text(), initialScale) << badScale.toStdString();
    }
}

TEST(FrameDecodedTableViewTest, ExportCsvToFileHandlesNullCellsAndWritesCleanly) {
    ui::widgets::FrameDecodedTableView table;
    table.renderData(makeValidHoldingRegisterResult(),
                     ::modbus::analyzer::RegisterDataType::UInt16,
                     ::modbus::base::RegisterOrder::ABCD,
                     ::modbus::address::AddressBase::Offset0Based);
    ASSERT_GE(table.rowCount(), 1);
    // Simulate missing/null item in a cell.
    delete table.takeItem(0, 7);

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString csvPath = tempDir.filePath(QStringLiteral("decoded.csv"));
    QString error;
    EXPECT_TRUE(table.exportCsvToFile(csvPath, &error)) << error.toStdString();

    QFile file(csvPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    EXPECT_TRUE(content.contains(QStringLiteral("42F6")));
}

TEST(ModbusSettingsDialogTest, OutOfRangeLogLevelFallsBackToInfoAndTogglesRetryControls) {
    ui::widgets::ModbusSettingsDialog::Settings initial{
        1000, 3, 100, false, 999, 0
    };
    ui::widgets::ModbusSettingsDialog dialog(initial);

    auto combos = dialog.findChildren<QComboBox*>();
    ASSERT_GE(combos.size(), 2);
    EXPECT_EQ(combos.at(0)->currentData().toInt(), static_cast<int>(spdlog::level::info));
    EXPECT_EQ(dialog.settings().logLevel, static_cast<int>(spdlog::level::info));

    auto* retryCheck = dialog.findChild<QCheckBox*>();
    ASSERT_NE(retryCheck, nullptr);
    auto spinBoxes = dialog.findChildren<QSpinBox*>();
    ASSERT_GE(spinBoxes.size(), 3);
    EXPECT_FALSE(spinBoxes.at(1)->isEnabled());
    EXPECT_FALSE(spinBoxes.at(2)->isEnabled());

    retryCheck->setChecked(true);
    EXPECT_TRUE(spinBoxes.at(1)->isEnabled());
    EXPECT_TRUE(spinBoxes.at(2)->isEnabled());
}
