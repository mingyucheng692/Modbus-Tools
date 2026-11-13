// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSpinBox>
#include <QVBoxLayout>
#include <memory>
#include "Config.h"
#include "ui/widgets/modbus/ModbusWorkbenchWidget.h"
#include "tests/mocks/UiTestDoubles.h"
#include "core/common/SettingsKeys.h"

using namespace ui::widgets::modbus;
using namespace ui::application::modbus;
using namespace modbus::address;

class ModbusWorkbenchWidgetTest : public ::testing::Test {
protected:
    void SetUp() override {
        fakeSettings_ = std::make_unique<tests::mocks::FakeSettingsService>();
        widget_ = std::make_unique<ModbusWorkbenchWidget>(fakeSettings_.get());
        widget_->setConfirmWritePollHandler([](uint8_t) { return true; });
    }

    std::unique_ptr<tests::mocks::FakeSettingsService> fakeSettings_;
    std::unique_ptr<ModbusWorkbenchWidget> widget_;
};

TEST_F(ModbusWorkbenchWidgetTest, InitialStateAndDefaults) {
    EXPECT_FALSE(widget_->isRawMode());
    EXPECT_FALSE(widget_->isPolling());
    EXPECT_EQ(widget_->slaveId(), 1);
    EXPECT_EQ(widget_->functionCode(), 0x03);
    EXPECT_EQ(widget_->startAddress(), 0);
    EXPECT_EQ(widget_->quantity(), 10);
    EXPECT_EQ(widget_->radix(), 10);
    EXPECT_EQ(widget_->pollingIntervalMs(), 1000);
    EXPECT_EQ(widget_->addressBase(), AddressBase::Offset0Based);

    const auto spec = widget_->currentSpec();
    EXPECT_FALSE(spec.isRawMode);
    EXPECT_EQ(spec.slaveId, 1);
    EXPECT_EQ(spec.functionCode, 0x03);
    EXPECT_EQ(spec.startAddress, 0);
    EXPECT_EQ(spec.quantity, 10);
}

TEST_F(ModbusWorkbenchWidgetTest, ModeSwitching) {
    widget_->setMode(1);
    EXPECT_TRUE(widget_->isRawMode());

    widget_->setRawText(QStringLiteral("01 03 00 00 00 0A C5 CD"));
    const auto spec = widget_->currentSpec();
    EXPECT_TRUE(spec.isRawMode);
    EXPECT_EQ(spec.rawPduOrAdu, QByteArray::fromHex("01030000000AC5CD"));

    widget_->setMode(0);
    EXPECT_FALSE(widget_->isRawMode());
}

TEST_F(ModbusWorkbenchWidgetTest, RadixToggle_UpdatesInputs) {
    widget_->setStartAddress(16);
    EXPECT_EQ(widget_->startAddress(), 16);
    EXPECT_EQ(widget_->addressPreviewLabel()->text(), QStringLiteral("PDU: 0x0010 (16)"));

    // Verify toggling radix does NOT silently mutate write payload
    widget_->setFunctionCode(0x06);
    widget_->setWritePayload(QStringLiteral("1234"));
    EXPECT_EQ(widget_->findChild<QLineEdit*>(QStringLiteral("workbenchFc06ValueEdit"))->text(), QStringLiteral("1234"));

    widget_->setRadix(16);
    EXPECT_EQ(widget_->radix(), 16);
    EXPECT_EQ(widget_->addressPreviewLabel()->text(), QStringLiteral("PDU: 0x0010 (16)"));
    // Payload remains untouched
    EXPECT_EQ(widget_->findChild<QLineEdit*>(QStringLiteral("workbenchFc06ValueEdit"))->text(), QStringLiteral("1234"));

    widget_->setRadix(10);
    EXPECT_EQ(widget_->radix(), 10);
    EXPECT_EQ(widget_->findChild<QLineEdit*>(QStringLiteral("workbenchFc06ValueEdit"))->text(), QStringLiteral("1234"));
}

TEST_F(ModbusWorkbenchWidgetTest, AddressBaseAdaptation) {
    widget_->setStartAddress(10);
    EXPECT_EQ(widget_->startAddress(), 10);
    EXPECT_EQ(widget_->addressPreviewLabel()->text(), QStringLiteral("PDU: 0x000A (10)"));

    auto* combo = widget_->addressBaseComboBox();
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->sizeAdjustPolicy(), QComboBox::AdjustToContents);
    EXPECT_EQ(combo->currentIndex(), 0);

    widget_->setAddressBase(AddressBase::PlcAddress1Based);
    EXPECT_EQ(widget_->addressBase(), AddressBase::PlcAddress1Based);
    EXPECT_EQ(combo->currentIndex(), 1);
    // PDU address stays constant across base display changes
    EXPECT_EQ(widget_->startAddress(), 10);
    EXPECT_EQ(widget_->addressPreviewLabel()->text(), QStringLiteral("PDU: 0x000A (10)"));
    EXPECT_EQ(widget_->addressEdit()->text(), QStringLiteral("11"));

    QSignalSpy spy(widget_.get(), &ModbusWorkbenchWidget::addressBaseChanged);
    combo->setCurrentIndex(0);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<AddressBase>(), AddressBase::Offset0Based);
    EXPECT_EQ(widget_->addressBase(), AddressBase::Offset0Based);
    EXPECT_EQ(widget_->startAddress(), 10);
    EXPECT_EQ(widget_->addressPreviewLabel()->text(), QStringLiteral("PDU: 0x000A (10)"));
    EXPECT_EQ(widget_->addressEdit()->text(), QStringLiteral("10"));

    auto* rootLayout = widget_->findChild<QVBoxLayout*>(QStringLiteral("workbenchRootLayout"));
    ASSERT_NE(rootLayout, nullptr);
    int left = 0, top = 0, right = 0, bottom = 0;
    rootLayout->getContentsMargins(&left, &top, &right, &bottom);
    EXPECT_EQ(top, 4);
    EXPECT_EQ(left, 4);
    EXPECT_EQ(right, 4);
    EXPECT_EQ(bottom, 4);
    EXPECT_EQ(rootLayout->spacing(), 6);
}

TEST_F(ModbusWorkbenchWidgetTest, AddressingFormLayoutAndPduPreviewOnInvalidInput) {
    auto* formLayout = widget_->findChild<QFormLayout*>(QStringLiteral("workbenchAddressingFormLayout"));
    ASSERT_NE(formLayout, nullptr);
    EXPECT_EQ(formLayout->rowCount(), 5);

    auto* fcCombo = widget_->findChild<QComboBox*>(QStringLiteral("workbenchFcCombo"));
    ASSERT_NE(fcCombo, nullptr);
    auto* group = widget_->findChild<QGroupBox*>(QStringLiteral("workbenchAddressingGroup"));
    ASSERT_NE(group, nullptr);

    // Form inputs exist
    auto* slaveSpin = widget_->findChild<QSpinBox*>(QStringLiteral("workbenchSlaveIdSpin"));
    auto* addrEdit = widget_->addressEdit();
    auto* qtySpin = widget_->findChild<QSpinBox*>(QStringLiteral("workbenchQuantitySpin"));
    ASSERT_NE(slaveSpin, nullptr);
    ASSERT_NE(addrEdit, nullptr);
    ASSERT_NE(qtySpin, nullptr);

    // Invalid input turns preview into PDU: --
    addrEdit->setText(QStringLiteral("invalid_addr"));
    QMetaObject::invokeMethod(widget_.get(), "onAddressEdited");
    EXPECT_EQ(widget_->addressPreviewLabel()->text(), QStringLiteral("PDU: --"));
}

TEST_F(ModbusWorkbenchWidgetTest, ContinuousAddressOverflowDefense) {
    widget_->setStartAddress(65530);
    widget_->setQuantity(10); // 65530 + 10 = 65540 > 65536

    QSignalSpy spy(widget_.get(), &ModbusWorkbenchWidget::executeRequested);
    // Attempt execution
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_EQ(spy.count(), 0); // Blocked

    // Correct quantity so 65530 + 6 = 65536
    widget_->setQuantity(6);
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ModbusWorkbenchWidgetTest, BroadcastReadForbidden) {
    widget_->setSlaveId(0);
    widget_->setFunctionCode(0x03); // Read holding registers

    QSignalSpy spy(widget_.get(), &ModbusWorkbenchWidget::executeRequested);
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_EQ(spy.count(), 0); // Read broadcast blocked per IEC 61158

    // Switch to write single register (broadcast write allowed)
    widget_->setFunctionCode(0x06);
    widget_->setWritePayload(QStringLiteral("1234"));
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().at(0).value<ModbusExecuteSpec>().slaveId, 0);
}

TEST_F(ModbusWorkbenchWidgetTest, WriteStackDynamicSwitching) {
    // FC 05: Single Coil
    widget_->setFunctionCode(0x05);
    widget_->setFc05Value(true);
    auto spec05 = widget_->currentSpec();
    EXPECT_EQ(spec05.writeData, QByteArray::fromHex("FF00"));
    EXPECT_EQ(spec05.quantity, 1);

    widget_->setFc05Value(false);
    spec05 = widget_->currentSpec();
    EXPECT_EQ(spec05.writeData, QByteArray::fromHex("0000"));

    // FC 06: Single Register
    widget_->setFunctionCode(0x06);
    widget_->setWritePayload(QStringLiteral("258"));
    auto spec06 = widget_->currentSpec();
    EXPECT_EQ(spec06.quantity, 1);
    EXPECT_EQ(spec06.writeData.toHex().toUpper(), QByteArray("0102"));

    // FC 0F: Multiple Coils
    widget_->setFunctionCode(0x0F);
    widget_->setQuantity(5);
    widget_->setWritePayload(QStringLiteral("10110"));
    auto spec0f = widget_->currentSpec();
    EXPECT_EQ(spec0f.quantity, 5);
    EXPECT_FALSE(spec0f.writeData.isEmpty());

    // FC 10: Multiple Registers
    widget_->setFunctionCode(0x10);
    widget_->setQuantity(2);
    widget_->setWritePayload(QStringLiteral("10 20"));
    auto spec10 = widget_->currentSpec();
    EXPECT_EQ(spec10.quantity, 2);
    EXPECT_EQ(spec10.writeData.size(), 4);
}

TEST_F(ModbusWorkbenchWidgetTest, Defense1_WritePollHighRiskConfirmation) {
    widget_->setFunctionCode(0x06);
    widget_->setWritePayload(QStringLiteral("100"));

    bool hookCalled = false;
    uint8_t hookFc = 0;

    // 1. User cancels confirmation
    widget_->setConfirmWritePollHandler([&](uint8_t fc) {
        hookCalled = true;
        hookFc = fc;
        return false;
    });

    widget_->setPollingChecked(true);
    EXPECT_TRUE(hookCalled);
    EXPECT_EQ(hookFc, 0x06);
    EXPECT_FALSE(widget_->isPolling());

    // 2. User confirms
    hookCalled = false;
    widget_->setConfirmWritePollHandler([&](uint8_t fc) {
        hookCalled = true;
        hookFc = fc;
        return true;
    });

    QSignalSpy spy(widget_.get(), &ModbusWorkbenchWidget::pollToggleRequested);
    widget_->setPollingChecked(true);
    EXPECT_TRUE(hookCalled);
    EXPECT_TRUE(widget_->isPolling());
    ASSERT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.first().at(0).toBool()); // start == true
    EXPECT_TRUE(spy.first().at(2).toBool()); // userConfirmedWritePoll == true
}

TEST_F(ModbusWorkbenchWidgetTest, Defense2_RuntimePollCutoffOnWriteSwitch) {
    // Start polling on FC 03 (Read)
    widget_->setFunctionCode(0x03);
    QSignalSpy spy(widget_.get(), &ModbusWorkbenchWidget::pollToggleRequested);
    QSignalSpy logSpy(widget_.get(), &ModbusWorkbenchWidget::logMessageRequested);

    widget_->setPollingChecked(true);
    EXPECT_TRUE(widget_->isPolling());
    EXPECT_EQ(spy.count(), 1);

    // User switches to FC 06 (Write) while polling is active
    widget_->setFunctionCode(0x06);
    EXPECT_FALSE(widget_->isPolling());
    EXPECT_EQ(spy.count(), 2);
    EXPECT_FALSE(spy.last().at(0).toBool()); // stop emitted
    EXPECT_GE(logSpy.count(), 1);
}

TEST_F(ModbusWorkbenchWidgetTest, Defense3_WritePayloadValidationError) {
    widget_->setFunctionCode(0x0F);
    widget_->setQuantity(8);
    widget_->setWritePayload(QStringLiteral("101")); // Only 3 bits for 8 expected

    QSignalSpy spy(widget_.get(), &ModbusWorkbenchWidget::executeRequested);
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_EQ(spy.count(), 0); // Rejected due to length mismatch

    // Fix bit length
    widget_->setWritePayload(QStringLiteral("10101010")); // 8 bits
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ModbusWorkbenchWidgetTest, Defense4_BroadcastPollHardDisabled) {
    widget_->setSlaveId(0);
    // Polling cannot be started when slave ID is 0
    widget_->setPollingChecked(true);
    EXPECT_FALSE(widget_->isPolling());
}

TEST_F(ModbusWorkbenchWidgetTest, RawFrameUtilities) {
    widget_->setMode(1);
    widget_->setRawText(QStringLiteral("01 03 00 00 00 0A"));

    // Add CRC16
    QMetaObject::invokeMethod(widget_.get(), "onAddCrcClicked");
    auto specCrc = widget_->currentSpec();
    EXPECT_EQ(specCrc.rawPduOrAdu.size(), 8); // 6 + 2 bytes CRC

    // Clean Hex
    widget_->setRawText(QStringLiteral("01030000000a"));
    QMetaObject::invokeMethod(widget_.get(), "onCleanHexClicked");
    auto specClean = widget_->currentSpec();
    EXPECT_EQ(specClean.rawPduOrAdu, QByteArray::fromHex("01030000000A"));
}

TEST_F(ModbusWorkbenchWidgetTest, SettingsPersistenceAndSafeDefaults) {
    widget_->setSlaveId(5);
    widget_->setFunctionCode(0x10); // Multiple registers write!
    widget_->setQuantity(25);
    widget_->setPollingChecked(true);

    // Destroy current widget, triggering saveSettings()
    widget_.reset();

    // Verify polling was not persisted
    EXPECT_FALSE(fakeSettings_->contains(QStringLiteral("modbus/workbench/isPolling")));

    // Reconstruct widget with same settings
    widget_ = std::make_unique<ModbusWorkbenchWidget>(fakeSettings_.get());
    widget_->setConfirmWritePollHandler([](uint8_t) { return true; });

    EXPECT_EQ(widget_->slaveId(), 5);
    EXPECT_EQ(widget_->quantity(), 25);
    // Safe defaults check: write function code is restored to FC 03
    EXPECT_EQ(widget_->functionCode(), 0x03);
    EXPECT_FALSE(widget_->isPolling());
}

TEST_F(ModbusWorkbenchWidgetTest, OperatorPokaYoke_ShortcutsAndAppearance) {
    auto* execBtn = widget_->executeButton();
    ASSERT_NE(execBtn, nullptr);

    // Default FC 03 (Read): text is "Execute (F5)", shortcut is F5, no warning border
    EXPECT_EQ(execBtn->text(), QStringLiteral("Execute (F5)"));
    EXPECT_EQ(execBtn->shortcut(), QKeySequence(Qt::Key_F5));
    EXPECT_TRUE(execBtn->styleSheet().isEmpty());

    // Switch to Write FC 06: text becomes "Write (F5)", shortcut F5, amber warning border
    widget_->setFunctionCode(0x06);
    EXPECT_EQ(execBtn->text(), QStringLiteral("Write (F5)"));
    EXPECT_EQ(execBtn->shortcut(), QKeySequence(Qt::Key_F5));
    EXPECT_TRUE(execBtn->styleSheet().contains(QStringLiteral("#e65100")));

    // Switch to Write FC 10: also has amber warning border
    widget_->setFunctionCode(0x10);
    EXPECT_EQ(execBtn->text(), QStringLiteral("Write (F5)"));
    EXPECT_TRUE(execBtn->styleSheet().contains(QStringLiteral("#e65100")));

    // Switch back to Read FC 04: restores "Execute (F5)", no border
    widget_->setFunctionCode(0x04);
    EXPECT_EQ(execBtn->text(), QStringLiteral("Execute (F5)"));
    EXPECT_TRUE(execBtn->styleSheet().isEmpty());

    // Switch to FC 03 and start polling: text becomes "Stop Polling (Esc)", shortcut is Esc
    widget_->setFunctionCode(0x03);
    widget_->setPollingChecked(true);
    EXPECT_TRUE(widget_->isPolling());
    EXPECT_EQ(execBtn->text(), QStringLiteral("Stop Polling (Esc)"));
    EXPECT_EQ(execBtn->shortcut(), QKeySequence(Qt::Key_Escape));
    EXPECT_TRUE(execBtn->styleSheet().isEmpty());

    // Stop polling via execute button click (as triggered by shortcut)
    QMetaObject::invokeMethod(widget_.get(), "onExecuteClicked");
    EXPECT_FALSE(widget_->isPolling());
    EXPECT_EQ(execBtn->text(), QStringLiteral("Execute (F5)"));
    EXPECT_EQ(execBtn->shortcut(), QKeySequence(Qt::Key_F5));
}

TEST_F(ModbusWorkbenchWidgetTest, OperatorPokaYoke_GuardHintAndTooltips) {
    auto* hint = widget_->guardHintLabel();
    ASSERT_NE(hint, nullptr);
    auto* execBtn = widget_->executeButton();
    ASSERT_NE(execBtn, nullptr);
    auto* rawBtn = widget_->findChild<QPushButton*>(QStringLiteral("workbenchRawExecuteBtn"));
    ASSERT_NE(rawBtn, nullptr);
    auto* pollCheck = widget_->findChild<QCheckBox*>(QStringLiteral("workbenchPollCheck"));
    ASSERT_NE(pollCheck, nullptr);

    // Initially when guards are unset/empty, hint is hidden
    EXPECT_TRUE(hint->isHidden());

    // Disconnected state: guards disabled with tooltipMessage
    ExecutionGuardState guards;
    guards.executeEnabled = false;
    guards.pollingEnabled = false;
    guards.rawSendEnabled = false;
    guards.tooltipMessage = QStringLiteral("Connect device to send commands.");
    widget_->setExecutionGuards(guards);

    // Hint label becomes visible with message
    EXPECT_FALSE(hint->isHidden());
    EXPECT_EQ(hint->text(), QStringLiteral("Connect device to send commands."));

    // Tooltips propagated to action buttons
    EXPECT_EQ(execBtn->toolTip(), QStringLiteral("Connect device to send commands."));
    EXPECT_EQ(rawBtn->toolTip(), QStringLiteral("Connect device to send commands."));
    EXPECT_EQ(pollCheck->toolTip(), QStringLiteral("Connect device to send commands."));

    // Connected state: execute enabled, tooltip cleared
    guards.executeEnabled = true;
    guards.pollingEnabled = true;
    guards.rawSendEnabled = true;
    guards.tooltipMessage.clear();
    widget_->setExecutionGuards(guards);

    // Hint label hidden, tooltips cleared
    EXPECT_TRUE(hint->isHidden());
    EXPECT_TRUE(execBtn->toolTip().isEmpty());
    EXPECT_TRUE(rawBtn->toolTip().isEmpty());
    EXPECT_TRUE(pollCheck->toolTip().isEmpty());

    // When slaveId is 0, broadcast tooltip takes precedence on pollCheck
    widget_->setSlaveId(0);
    EXPECT_FALSE(pollCheck->toolTip().isEmpty());
    EXPECT_NE(pollCheck->toolTip(), QStringLiteral("Connect device to send commands."));
}

TEST_F(ModbusWorkbenchWidgetTest, RequestData_PersistentVisibilityAndZeroCoordinateShift) {
    widget_->resize(400, 600);
    widget_->show();

    auto* writeGroup = widget_->writeGroup();
    ASSERT_NE(writeGroup, nullptr);
    EXPECT_FALSE(writeGroup->isHidden());

    auto* execBtn = widget_->executeButton();
    ASSERT_NE(execBtn, nullptr);

    widget_->setFunctionCode(0x03);
    EXPECT_FALSE(writeGroup->isHidden());
    const int yRead = execBtn->geometry().y();

    const std::vector<uint8_t> fcs = {0x01, 0x02, 0x04, 0x05, 0x06, 0x0F, 0x10};
    for (uint8_t fc : fcs) {
        widget_->setFunctionCode(fc);
        EXPECT_FALSE(writeGroup->isHidden());
        const int yCurrent = execBtn->geometry().y();
        EXPECT_EQ(yCurrent, yRead) << "FC " << static_cast<int>(fc) << " execution button y shifted!";
    }
}

TEST_F(ModbusWorkbenchWidgetTest, DataRadix_DecoupledFromAddressRadix) {
    widget_->setFunctionCode(0x06);
    widget_->setDataRadix(10);
    EXPECT_EQ(widget_->dataRadix(), 10);

    widget_->setRadix(16);
    EXPECT_EQ(widget_->radix(), 16);
    EXPECT_EQ(widget_->dataRadix(), 10);

    widget_->setRadix(10);
    EXPECT_EQ(widget_->radix(), 10);
    EXPECT_EQ(widget_->dataRadix(), 10);
}

TEST_F(ModbusWorkbenchWidgetTest, DataRadix_CoilLockBinAndRegisterRadixRestore) {
    widget_->setFunctionCode(0x06);
    widget_->setDataRadix(16);
    EXPECT_EQ(widget_->dataRadix(), 16);

    widget_->setFunctionCode(0x05);
    EXPECT_EQ(widget_->dataRadix(), 2);
    auto* decBtn = widget_->findChild<QPushButton*>(QStringLiteral("workbenchDataRadixDecBtn"));
    auto* hexBtn = widget_->findChild<QPushButton*>(QStringLiteral("workbenchDataRadixHexBtn"));
    auto* binBtn = widget_->findChild<QPushButton*>(QStringLiteral("workbenchDataRadixBinBtn"));
    ASSERT_NE(decBtn, nullptr);
    ASSERT_NE(hexBtn, nullptr);
    ASSERT_NE(binBtn, nullptr);
    EXPECT_FALSE(decBtn->isEnabled());
    EXPECT_FALSE(hexBtn->isEnabled());
    EXPECT_TRUE(binBtn->isEnabled());
    EXPECT_TRUE(binBtn->isChecked());

    widget_->setFunctionCode(0x0F);
    EXPECT_EQ(widget_->dataRadix(), 2);
    EXPECT_FALSE(decBtn->isEnabled());

    widget_->setFunctionCode(0x06);
    EXPECT_EQ(widget_->dataRadix(), 16);
    EXPECT_TRUE(decBtn->isEnabled());
    EXPECT_TRUE(hexBtn->isEnabled());
    EXPECT_TRUE(binBtn->isEnabled());
    EXPECT_TRUE(hexBtn->isChecked());
}

TEST_F(ModbusWorkbenchWidgetTest, DataRadix_InPlaceConversion) {
    widget_->setFunctionCode(0x06);
    widget_->setDataRadix(10);
    auto* edit = widget_->findChild<QLineEdit*>(QStringLiteral("workbenchFc06ValueEdit"));
    ASSERT_NE(edit, nullptr);
    edit->setText(QStringLiteral("1234"));

    widget_->setDataRadix(16);
    EXPECT_EQ(widget_->dataRadix(), 16);
    EXPECT_EQ(edit->text().toUpper(), QStringLiteral("4D2"));
    auto specHex = widget_->currentSpec();
    EXPECT_EQ(specHex.writeData, QByteArray::fromHex("04D2"));

    widget_->setDataRadix(2);
    EXPECT_EQ(widget_->dataRadix(), 2);
    EXPECT_EQ(edit->text(), QStringLiteral("0000010011010010"));
    auto specBin = widget_->currentSpec();
    EXPECT_EQ(specBin.writeData, QByteArray::fromHex("04D2"));

    edit->setText(QStringLiteral("invalid_val"));
    widget_->setDataRadix(10);
    EXPECT_EQ(edit->text(), QStringLiteral("invalid_val"));
}

TEST_F(ModbusWorkbenchWidgetTest, Validation_WriteErrorDisplayedInWriteGroupWithoutAddressRedBorder) {
    widget_->setFunctionCode(0x0F);
    widget_->setQuantity(8);
    auto* bitsEdit = widget_->findChild<QLineEdit*>(QStringLiteral("workbenchFc0fBitsEdit"));
    auto* addrEdit = widget_->findChild<QLineEdit*>(QStringLiteral("workbenchAddressEdit"));
    auto* writeWarning = widget_->writeWarningLabel();
    auto* addrWarning = widget_->addressWarningLabel();
    ASSERT_NE(bitsEdit, nullptr);
    ASSERT_NE(addrEdit, nullptr);
    ASSERT_NE(writeWarning, nullptr);
    ASSERT_NE(addrWarning, nullptr);

    EXPECT_TRUE(writeWarning->isHidden());
    EXPECT_TRUE(addrWarning->isHidden());
    EXPECT_FALSE(addrEdit->styleSheet().contains(QStringLiteral("#d32f2f")));

    bitsEdit->setText(QStringLiteral("1010"));
    emit bitsEdit->textEdited(QStringLiteral("1010"));

    EXPECT_FALSE(writeWarning->isHidden());
    EXPECT_TRUE(addrWarning->isHidden());
    EXPECT_FALSE(addrEdit->styleSheet().contains(QStringLiteral("#d32f2f")));
    EXPECT_TRUE(writeWarning->text().contains(QStringLiteral("4")));
}

TEST_F(ModbusWorkbenchWidgetTest, FunctionCode_ShortCaptionAndCanonicalEnglishTooltips) {
    auto* fcCombo = widget_->findChild<QComboBox*>(QStringLiteral("workbenchFcCombo"));
    ASSERT_NE(fcCombo, nullptr);
    EXPECT_EQ(fcCombo->count(), 8);

    static constexpr const char* kExpectedTooltips[] = {
        "0x01 Read Coils", "0x02 Read Discrete Inputs",
        "0x03 Read Holding Registers", "0x04 Read Input Registers",
        "0x05 Write Single Coil", "0x06 Write Single Register",
        "0x0F Write Multiple Coils", "0x10 Write Multiple Registers"
    };

    for (int i = 0; i < 8; ++i) {
        const QString tip = fcCombo->itemData(i, Qt::ToolTipRole).toString();
        EXPECT_EQ(tip, QString::fromLatin1(kExpectedTooltips[i]));
    }
}

TEST_F(ModbusWorkbenchWidgetTest, StartupState_DefaultFc03DisablesDataRadixButtons) {
    ModbusWorkbenchWidget standalone(nullptr);
    EXPECT_EQ(standalone.functionCode(), 0x03);
    auto* decBtn = standalone.findChild<QPushButton*>(QStringLiteral("workbenchDataRadixDecBtn"));
    auto* hexBtn = standalone.findChild<QPushButton*>(QStringLiteral("workbenchDataRadixHexBtn"));
    auto* binBtn = standalone.findChild<QPushButton*>(QStringLiteral("workbenchDataRadixBinBtn"));
    ASSERT_NE(decBtn, nullptr);
    ASSERT_NE(hexBtn, nullptr);
    ASSERT_NE(binBtn, nullptr);
    EXPECT_FALSE(decBtn->isEnabled());
    EXPECT_FALSE(hexBtn->isEnabled());
    EXPECT_FALSE(binBtn->isEnabled());
}

TEST_F(ModbusWorkbenchWidgetTest, LoadSettings_HighCoilQuantityNotClampedTo125) {
    tests::mocks::FakeSettingsService persisted;
    persisted.setValue(core::common::settings_keys::kModbusWorkbenchFunctionCode, 0x01);
    persisted.setValue(core::common::settings_keys::kModbusWorkbenchQuantity, 1500);

    ModbusWorkbenchWidget restored(&persisted);
    EXPECT_EQ(restored.functionCode(), 0x01);
    EXPECT_EQ(restored.quantity(), 1500);
}

TEST_F(ModbusWorkbenchWidgetTest, PollIntervalSpin_UsesConfigPollingMinMax) {
    auto* pollSpin = widget_->findChild<QSpinBox*>(QStringLiteral("workbenchPollIntervalSpin"));
    ASSERT_NE(pollSpin, nullptr);
    EXPECT_EQ(pollSpin->minimum(), config::Polling::kMinIntervalMs);
    EXPECT_EQ(pollSpin->maximum(), config::Polling::kMaxIntervalMs);
    EXPECT_EQ(pollSpin->singleStep(), config::Polling::kIntervalStepMs);
}

TEST_F(ModbusWorkbenchWidgetTest, Polling_AutoStopsOnParameterChanges) {
    QSignalSpy stopSpy(widget_.get(), &ModbusWorkbenchWidget::pollToggleRequested);
    auto* addrEdit = widget_->findChild<QLineEdit*>(QStringLiteral("workbenchAddressEdit"));
    ASSERT_NE(addrEdit, nullptr);

    widget_->setFunctionCode(0x03);
    widget_->setPollingChecked(true);
    ASSERT_TRUE(widget_->isPolling());
    stopSpy.clear();
    widget_->setQuantity(20);
    EXPECT_FALSE(widget_->isPolling());
    ASSERT_EQ(stopSpy.count(), 1);
    EXPECT_FALSE(stopSpy.last().at(0).toBool());

    widget_->setPollingChecked(true);
    ASSERT_TRUE(widget_->isPolling());
    stopSpy.clear();
    addrEdit->setText(QStringLiteral("5"));
    emit addrEdit->textEdited(QStringLiteral("5"));
    EXPECT_FALSE(widget_->isPolling());
    ASSERT_EQ(stopSpy.count(), 1);

    widget_->setPollingChecked(true);
    ASSERT_TRUE(widget_->isPolling());
    stopSpy.clear();
    widget_->setSlaveId(2);
    EXPECT_FALSE(widget_->isPolling());
    ASSERT_EQ(stopSpy.count(), 1);

    widget_->setPollingChecked(true);
    ASSERT_TRUE(widget_->isPolling());
    stopSpy.clear();
    widget_->setFunctionCode(0x04);
    EXPECT_FALSE(widget_->isPolling());
    ASSERT_EQ(stopSpy.count(), 1);

    widget_->setFunctionCode(0x06);
    widget_->setWritePayload(QStringLiteral("42"));
    widget_->setPollingChecked(true);
    ASSERT_TRUE(widget_->isPolling());
    stopSpy.clear();
    auto* fc06Edit = widget_->findChild<QLineEdit*>(QStringLiteral("workbenchFc06ValueEdit"));
    ASSERT_NE(fc06Edit, nullptr);
    fc06Edit->setText(QStringLiteral("99"));
    emit fc06Edit->textEdited(QStringLiteral("99"));
    EXPECT_FALSE(widget_->isPolling());
    ASSERT_EQ(stopSpy.count(), 1);
}

TEST_F(ModbusWorkbenchWidgetTest, Polling_StopButtonAndShortcutWorkInStandardAndRawModes) {
    auto* execBtn = widget_->findChild<QPushButton*>(QStringLiteral("workbenchExecuteBtn"));
    auto* stopShortcut = widget_->findChild<QShortcut*>(QStringLiteral("workbenchStopPollShortcut"));
    ASSERT_NE(execBtn, nullptr);
    ASSERT_NE(stopShortcut, nullptr);

    widget_->setPollingChecked(true);
    ASSERT_TRUE(widget_->isPolling());
    EXPECT_TRUE(execBtn->isEnabled());
    EXPECT_FALSE(stopShortcut->isEnabled());

    // Switch to Raw mode while polling stays active: shortcut enables and stops polling cleanly.
    widget_->setMode(1);
    EXPECT_TRUE(widget_->isRawMode());
    EXPECT_TRUE(execBtn->isEnabled());
    EXPECT_TRUE(stopShortcut->isEnabled());

    QSignalSpy stopSpy(widget_.get(), &ModbusWorkbenchWidget::pollToggleRequested);
    emit stopShortcut->activated();
    EXPECT_FALSE(widget_->isPolling());
    ASSERT_EQ(stopSpy.count(), 1);
    EXPECT_FALSE(stopSpy.last().at(0).toBool());
    EXPECT_FALSE(stopShortcut->isEnabled());
}

TEST_F(ModbusWorkbenchWidgetTest, Validation_EmptyFc0fDisablesExecuteWithActionableHintWithoutPrematureWarning) {
    auto* execBtn = widget_->findChild<QPushButton*>(QStringLiteral("workbenchExecuteBtn"));
    auto* writeWarning = widget_->writeWarningLabel();
    auto* hintLabel = widget_->guardHintLabel();
    ASSERT_NE(execBtn, nullptr);
    ASSERT_NE(writeWarning, nullptr);
    ASSERT_NE(hintLabel, nullptr);

    widget_->setFunctionCode(0x0F);
    EXPECT_FALSE(execBtn->isEnabled());
    EXPECT_TRUE(writeWarning->isHidden());
    EXPECT_FALSE(hintLabel->isHidden());
    EXPECT_FALSE(hintLabel->text().isEmpty());
    EXPECT_EQ(execBtn->toolTip(), hintLabel->text());
}


