// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "common/SettingsKeys.h"
#include "mocks/UiTestDoubles.h"
#include "views/serial/SerialPage.h"
#include "widgets/ByteMonitorWidget.h"
#include "widgets/GenericInputWidget.h"
#include "widgets/CollapsibleSection.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QListView>
#include <QScrollArea>
#include <QSignalSpy>
#include <QTextEdit>

namespace ui::views::serial {

namespace {

QCheckBox* findCheckBoxByText(QWidget& root, const QString& text) {
    for (auto* box : root.findChildren<QCheckBox*>()) {
        if (box->text() == text) {
            return box;
        }
    }
    return nullptr;
}

bool byteMonitorContains(SerialPage& view, const QString& needle) {
    auto* monitor = view.findChild<ui::widgets::ByteMonitorWidget*>();
    if (!monitor) return false;
    auto* listView = monitor->findChild<QListView*>();
    if (!listView || !listView->model()) return false;
    for (int r = 0; r < listView->model()->rowCount(); ++r) {
        if (listView->model()->data(listView->model()->index(r, 0)).toString().contains(needle)) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(SerialPageTest, Layout_720x480_NoOuterScrollArea) {
    tests::mocks::FakeSettingsService settings;
    SerialPage view(&settings);
    view.resize(720, 480);
    EXPECT_GE(view.width(), 720);
    EXPECT_GE(view.height(), 480);

    // Page must be added directly to layout; zero outer/parent QScrollArea
    EXPECT_TRUE(view.findChildren<QScrollArea*>().isEmpty());

    auto sections = view.findChildren<ui::widgets::CollapsibleSection*>();
    EXPECT_GE(sections.size(), 2);
}

TEST(SerialPageTest, LoadsAndSavesDtrRtsWithCanonicalKeys) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(core::common::settings_keys::kSerialDtr, true);
    settings.setValue(core::common::settings_keys::kSerialRts, false);

    SerialPage view(&settings);
    QCheckBox* dtrCheck = findCheckBoxByText(view, QStringLiteral("DTR"));
    QCheckBox* rtsCheck = findCheckBoxByText(view, QStringLiteral("RTS"));
    ASSERT_NE(dtrCheck, nullptr);
    ASSERT_NE(rtsCheck, nullptr);
    EXPECT_TRUE(dtrCheck->isChecked());
    EXPECT_FALSE(rtsCheck->isChecked());

    rtsCheck->setChecked(true);
    EXPECT_TRUE(settings.value(core::common::settings_keys::kSerialRts).toBool());
}

TEST(SerialPageTest, AppliesSavedControlLinesOnOpenAndAlertsOnUnexpectedDisconnect) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(core::common::settings_keys::kSerialDtr, true);
    settings.setValue(core::common::settings_keys::kSerialRts, true);

    SerialPage view(&settings);
    int alertCount = 0;
    view.setDisconnectAlertHandler([&alertCount]() { ++alertCount; });

    QSignalSpy controlSpy(&view, &SerialPage::serialControlLinesApplied);
    QSignalSpy disconnectSpy(&view, &SerialPage::unexpectedDisconnectDetected);

    // Closed before ever opening must not trigger unexpected disconnect alert.
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerStateChanged", Qt::DirectConnection,
        Q_ARG(io::ChannelState, io::ChannelState::Closed),
        Q_ARG(quint64, quint64(0))));
    EXPECT_EQ(alertCount, 0);
    EXPECT_EQ(disconnectSpy.count(), 0);

    // Transitioning to Open applies saved DTR/RTS settings.
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerStateChanged", Qt::DirectConnection,
        Q_ARG(io::ChannelState, io::ChannelState::Open),
        Q_ARG(quint64, quint64(0))));
    ASSERT_EQ(controlSpy.count(), 1);
    EXPECT_TRUE(controlSpy.at(0).at(0).toBool());
    EXPECT_TRUE(controlSpy.at(0).at(1).toBool());

    // Passive drop from Open -> Closed triggers unexpected disconnect signal & handler.
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerStateChanged", Qt::DirectConnection,
        Q_ARG(io::ChannelState, io::ChannelState::Closed),
        Q_ARG(quint64, quint64(0))));
    EXPECT_EQ(alertCount, 1);
    EXPECT_EQ(disconnectSpy.count(), 1);
}

TEST(SerialPageTest, ActiveDisconnect_OpenToClosingToClosedSuppressesDisconnectAlert) {
    tests::mocks::FakeSettingsService settings;
    SerialPage view(&settings);
    int alertCount = 0;
    view.setDisconnectAlertHandler([&alertCount]() { ++alertCount; });

    QSignalSpy disconnectSpy(&view, &SerialPage::unexpectedDisconnectDetected);

    // Open channel
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerStateChanged", Qt::DirectConnection,
        Q_ARG(io::ChannelState, io::ChannelState::Open),
        Q_ARG(quint64, quint64(0))));
    EXPECT_TRUE(view.isConnected());

    // Active disconnect requested
    view.onDisconnectClicked();

    // Worker transitions Open -> Closing -> Closed
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerStateChanged", Qt::DirectConnection,
        Q_ARG(io::ChannelState, io::ChannelState::Closing),
        Q_ARG(quint64, quint64(0))));
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerStateChanged", Qt::DirectConnection,
        Q_ARG(io::ChannelState, io::ChannelState::Closed),
        Q_ARG(quint64, quint64(0))));

    // Zero unexpected disconnect alerts
    EXPECT_EQ(alertCount, 0);
    EXPECT_EQ(disconnectSpy.count(), 0);
    EXPECT_FALSE(view.isConnected());
}

TEST(SerialPageTest, LocalizesWorkerErrorWrappingOsDetail) {
    tests::mocks::FakeSettingsService settings;
    SerialPage view(&settings);

    ASSERT_TRUE(QMetaObject::invokeMethod(
        &view, "onWorkerError", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("COM9")),
        Q_ARG(io::ChannelErrorCode, io::ChannelErrorCode::PermissionDenied),
        Q_ARG(QString, QStringLiteral("Access is denied"))));

    QElapsedTimer deadline;
    deadline.start();
    while (!(byteMonitorContains(view, QStringLiteral("Permission denied")) &&
             byteMonitorContains(view, QStringLiteral("Access is denied"))) &&
           deadline.elapsed() < 2000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    EXPECT_TRUE(byteMonitorContains(view, QStringLiteral("Permission denied")));
    EXPECT_TRUE(byteMonitorContains(view, QStringLiteral("Access is denied")));
}

TEST(SerialPageTest, HexValidation_OddLengthAndInvalidCharsBlockSend) {
    tests::mocks::FakeSettingsService settings;
    SerialPage view(&settings);
    auto* inputWidget = view.findChild<ui::widgets::GenericInputWidget*>();
    ASSERT_NE(inputWidget, nullptr);
    auto* textEdit = inputWidget->findChild<QTextEdit*>();
    ASSERT_NE(textEdit, nullptr);

    // Enable send controls to test validation pipeline
    inputWidget->setSendControlsEnabled(true);

    QSignalSpy sendSpy(inputWidget, &ui::widgets::GenericInputWidget::sendRequested);
    QSignalSpy failSpy(inputWidget, &ui::widgets::GenericInputWidget::validationFailed);

    // Odd-length hex text
    textEdit->setPlainText(QStringLiteral("123"));
    QMetaObject::invokeMethod(inputWidget, "onSendClicked");
    EXPECT_EQ(sendSpy.count(), 0);
    EXPECT_GE(failSpy.count(), 1);

    // Non-hex characters
    failSpy.clear();
    textEdit->setPlainText(QStringLiteral("FF GG"));
    QMetaObject::invokeMethod(inputWidget, "onSendClicked");
    EXPECT_EQ(sendSpy.count(), 0);
    EXPECT_GE(failSpy.count(), 1);

    // Valid hex
    textEdit->setPlainText(QStringLiteral("AA BB"));
    QMetaObject::invokeMethod(inputWidget, "onSendClicked");
    ASSERT_EQ(sendSpy.count(), 1);
    EXPECT_EQ(sendSpy.at(0).at(0).toByteArray(), QByteArray::fromHex("AABB"));
}

} // namespace ui::views::serial
