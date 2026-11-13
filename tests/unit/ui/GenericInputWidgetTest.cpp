// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "Config.h"
#include "mocks/UiTestDoubles.h"
#include "widgets/GenericInputWidget.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QEvent>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QTranslator>

namespace ui::widgets {

TEST(GenericInputWidgetTest, UnsetIntervalFallsBackToDefaultConfigAndGroupSwitchClearsHistory) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(QStringLiteral("groupA/sendHistory"), QStringList{QStringLiteral("AA BB"), QStringLiteral("CC DD")});

    GenericInputWidget widget(&settings);
    widget.setSettingsGroup(QStringLiteral("groupA"));

    auto* intervalSpin = widget.findChild<QSpinBox*>();
    ASSERT_NE(intervalSpin, nullptr);
    EXPECT_EQ(intervalSpin->value(), ::config::GenericIo::kDefaultInputIntervalMs);

    QComboBox* historyCombo = nullptr;
    for (auto* combo : widget.findChildren<QComboBox*>()) {
        if (combo->isEditable()) {
            historyCombo = combo;
            break;
        }
    }
    ASSERT_NE(historyCombo, nullptr);
    EXPECT_EQ(historyCombo->count(), 2);

    // Switching to another group with no saved history must clear prior items.
    widget.setSettingsGroup(QStringLiteral("groupB"));
    EXPECT_EQ(historyCombo->count(), 0);
}

TEST(GenericInputWidgetTest, InvalidHexBlocksConversionWithoutDataLossAndRespectsLatin1) {
    tests::mocks::FakeSettingsService settings;
    GenericInputWidget widget(&settings);

    auto* inputEdit = widget.findChild<QTextEdit*>();
    ASSERT_NE(inputEdit, nullptr);
    auto radios = widget.findChildren<QRadioButton*>();
    ASSERT_GE(radios.size(), 2);
    QRadioButton* hexRadio = radios.at(0);
    QRadioButton* asciiRadio = radios.at(1);

    QComboBox* encodingCombo = nullptr;
    for (auto* combo : widget.findChildren<QComboBox*>()) {
        if (!combo->isEditable() && combo->count() == 3) {
            encodingCombo = combo;
            break;
        }
    }
    ASSERT_NE(encodingCombo, nullptr);

    // Invalid hex (odd length / non-hex) must keep HEX selected and preserve user text.
    hexRadio->setChecked(true);
    inputEdit->setPlainText(QStringLiteral("12 Z"));
    asciiRadio->click();
    EXPECT_TRUE(hexRadio->isChecked());
    EXPECT_EQ(inputEdit->toPlainText(), QStringLiteral("12 Z"));

    // Valid Latin-1 hex 0xE9 -> 'é' and back to HEX "E9".
    const int latin1Idx = encodingCombo->findData(1);
    ASSERT_GE(latin1Idx, 0);
    encodingCombo->setCurrentIndex(latin1Idx);
    inputEdit->setPlainText(QStringLiteral("E9"));
    asciiRadio->click();
    EXPECT_TRUE(asciiRadio->isChecked());
    EXPECT_EQ(inputEdit->toPlainText(), QString(QChar(0xE9)));

    hexRadio->click();
    EXPECT_TRUE(hexRadio->isChecked());
    EXPECT_EQ(inputEdit->toPlainText(), QStringLiteral("E9"));
}

TEST(GenericInputWidgetTest, RetranslateUiUpdatesComboItemsAndTooltips) {
    tests::mocks::FakeSettingsService settings;
    GenericInputWidget widget(&settings);

    QComboBox* lineEndingCombo = nullptr;
    QComboBox* encodingCombo = nullptr;
    for (auto* combo : widget.findChildren<QComboBox*>()) {
        if (!combo->isEditable() && combo->count() == 4) {
            lineEndingCombo = combo;
        } else if (!combo->isEditable() && combo->count() == 3) {
            encodingCombo = combo;
        }
    }
    ASSERT_NE(lineEndingCombo, nullptr);
    ASSERT_NE(encodingCombo, nullptr);
    EXPECT_FALSE(lineEndingCombo->toolTip().isEmpty());
    EXPECT_FALSE(encodingCombo->toolTip().isEmpty());
    EXPECT_EQ(lineEndingCombo->itemText(0), QStringLiteral("No LF"));

    QTranslator translator;
    const QDir appDir(QCoreApplication::applicationDirPath());
    const bool loaded = translator.load(QStringLiteral(":/i18n/Modbus-Tools_zh_CN.qm")) ||
                        translator.load(appDir.filePath(QStringLiteral("../../translations/Modbus-Tools_zh_CN.qm")));
    ASSERT_TRUE(loaded);
    QApplication::installTranslator(&translator);
    QEvent langEvent(QEvent::LanguageChange);
    QApplication::sendEvent(&widget, &langEvent);

    EXPECT_NE(lineEndingCombo->itemText(0), QStringLiteral("No LF"));
    EXPECT_FALSE(lineEndingCombo->toolTip().isEmpty());
    EXPECT_FALSE(encodingCombo->toolTip().isEmpty());
    QApplication::removeTranslator(&translator);
}

} // namespace ui::widgets
