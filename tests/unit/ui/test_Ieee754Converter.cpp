/**
 * @file test_Ieee754Converter.cpp
 * @brief Unit tests for Ieee754ConverterWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "views/converter/Ieee754ConverterWidget.h"

#include <QApplication>
#include <QLineEdit>
#include <QTableWidget>
#include <QRadioButton>
#include <QPushButton>
#include <QClipboard>

namespace ui::views::converter {

class Ieee754ConverterTest : public ::testing::Test {
protected:
    void SetUp() override {
        widget_ = std::make_unique<Ieee754ConverterWidget>();
    }

    void TearDown() override {
        widget_.reset();
    }

    std::unique_ptr<Ieee754ConverterWidget> widget_;
};

TEST_F(Ieee754ConverterTest, InitialState_DefaultValuesAreValid) {
    auto* floatEdit = widget_->findChild<QLineEdit*>();
    ASSERT_NE(floatEdit, nullptr);

    auto* matrixTable = widget_->findChild<QTableWidget*>();
    ASSERT_NE(matrixTable, nullptr);
    EXPECT_EQ(matrixTable->rowCount(), 4);
    EXPECT_EQ(matrixTable->columnCount(), 7);

    // Initial default is 123.456
    auto* itemAbcdHex = matrixTable->item(0, 1);
    ASSERT_NE(itemAbcdHex, nullptr);
    EXPECT_EQ(itemAbcdHex->text(), QStringLiteral("42 F6 E9 79"));

    // Check CDAB word swap: E9 79 42 F6
    auto* itemCdabHex = matrixTable->item(1, 1);
    ASSERT_NE(itemCdabHex, nullptr);
    EXPECT_EQ(itemCdabHex->text(), QStringLiteral("E9 79 42 F6"));
}

TEST_F(Ieee754ConverterTest, FloatInput_DerivesHexAndIntegers) {
    auto lineEdits = widget_->findChildren<QLineEdit*>();
    ASSERT_GE(lineEdits.size(), 4);

    QLineEdit* floatEdit = lineEdits[0];
    QLineEdit* hexEdit = lineEdits[1];
    QLineEdit* signedEdit = lineEdits[2];
    QLineEdit* unsignedEdit = lineEdits[3];

    // Input 100.0f
    floatEdit->setText(QStringLiteral("100.0"));
    QMetaObject::invokeMethod(floatEdit, "textEdited", Q_ARG(QString, QStringLiteral("100.0")));

    EXPECT_EQ(hexEdit->text(), QStringLiteral("42 C8 00 00"));
    EXPECT_EQ(signedEdit->text(), QStringLiteral("1120403456"));
    EXPECT_EQ(unsignedEdit->text(), QStringLiteral("1120403456"));

    auto* matrixTable = widget_->findChild<QTableWidget*>();
    ASSERT_NE(matrixTable, nullptr);

    // ABCD hex is 42 C8 00 00
    EXPECT_EQ(matrixTable->item(0, 1)->text(), QStringLiteral("42 C8 00 00"));
    // CDAB hex is 00 00 42 C8
    EXPECT_EQ(matrixTable->item(1, 1)->text(), QStringLiteral("00 00 42 C8"));
    // BADC hex is C8 42 00 00
    EXPECT_EQ(matrixTable->item(2, 1)->text(), QStringLiteral("C8 42 00 00"));
    // DCBA hex is 00 00 C8 42
    EXPECT_EQ(matrixTable->item(3, 1)->text(), QStringLiteral("00 00 C8 42"));
}

TEST_F(Ieee754ConverterTest, HexInput_DerivesFloatAndIntegers) {
    auto lineEdits = widget_->findChildren<QLineEdit*>();
    ASSERT_GE(lineEdits.size(), 4);

    QLineEdit* floatEdit = lineEdits[0];
    QLineEdit* hexEdit = lineEdits[1];

    // Input -1.0f in Hex (0xBF800000)
    hexEdit->setText(QStringLiteral("BF 80 00 00"));
    QMetaObject::invokeMethod(hexEdit, "textEdited", Q_ARG(QString, QStringLiteral("BF 80 00 00")));

    EXPECT_EQ(floatEdit->text(), QStringLiteral("-1"));

    auto* matrixTable = widget_->findChild<QTableWidget*>();
    ASSERT_NE(matrixTable, nullptr);
    EXPECT_EQ(matrixTable->item(0, 1)->text(), QStringLiteral("BF 80 00 00"));
}

TEST_F(Ieee754ConverterTest, PrecisionSwitch_Float64ModeCalculatesCorrectly) {
    auto radios = widget_->findChildren<QRadioButton*>();
    ASSERT_GE(radios.size(), 2);

    QRadioButton* radioFloat64 = radios[1];
    radioFloat64->click();

    auto lineEdits = widget_->findChildren<QLineEdit*>();
    ASSERT_GE(lineEdits.size(), 4);

    QLineEdit* floatEdit = lineEdits[0];
    QLineEdit* hexEdit = lineEdits[1];

    floatEdit->setText(QStringLiteral("1.0"));
    QMetaObject::invokeMethod(floatEdit, "textEdited", Q_ARG(QString, QStringLiteral("1.0")));

    // 1.0 in IEEE 754 Double is 0x3FF0000000000000
    EXPECT_EQ(hexEdit->text(), QStringLiteral("3F F0 00 00 00 00 00 00"));

    auto* matrixTable = widget_->findChild<QTableWidget*>();
    ASSERT_NE(matrixTable, nullptr);
    EXPECT_EQ(matrixTable->item(0, 1)->text(), QStringLiteral("3F F0 00 00 00 00 00 00"));
}

TEST_F(Ieee754ConverterTest, SpecialValues_NaNAndInfinity) {
    auto lineEdits = widget_->findChildren<QLineEdit*>();
    ASSERT_GE(lineEdits.size(), 4);

    QLineEdit* floatEdit = lineEdits[0];
    QLineEdit* hexEdit = lineEdits[1];

    // Infinity
    floatEdit->setText(QStringLiteral("+inf"));
    QMetaObject::invokeMethod(floatEdit, "textEdited", Q_ARG(QString, QStringLiteral("+inf")));
    EXPECT_EQ(hexEdit->text(), QStringLiteral("7F 80 00 00"));

    // NaN
    floatEdit->setText(QStringLiteral("nan"));
    QMetaObject::invokeMethod(floatEdit, "textEdited", Q_ARG(QString, QStringLiteral("nan")));
    QString nanHex = hexEdit->text().remove(QLatin1Char(' '));
    bool ok = false;
    uint32_t nanVal = nanHex.toUInt(&ok, 16);
    EXPECT_TRUE(ok);
    EXPECT_EQ((nanVal >> 23) & 0xFF, 0xFF);
    EXPECT_NE(nanVal & 0x7FFFFF, 0);
}

} // namespace ui::views::converter
