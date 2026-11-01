// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <memory>
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
