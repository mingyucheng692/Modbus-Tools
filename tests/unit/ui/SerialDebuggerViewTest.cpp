// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "common/SettingsKeys.h"
#include "mocks/UiTestDoubles.h"
#include "views/serial/SerialDebuggerView.h"
#include "widgets/ByteMonitorWidget.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QListView>
#include <QSignalSpy>

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

bool byteMonitorContains(SerialDebuggerView& view, const QString& needle) {
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

TEST(SerialDebuggerViewTest, LoadsAndSavesDtrRtsWithCanonicalKeys) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(core::common::settings_keys::kSerialDebuggerDtr, true);
    settings.setValue(core::common::settings_keys::kSerialDebuggerRts, false);

    SerialDebuggerView view(&settings);
    QCheckBox* dtrCheck = findCheckBoxByText(view, QStringLiteral("DTR"));
    QCheckBox* rtsCheck = findCheckBoxByText(view, QStringLiteral("RTS"));
    ASSERT_NE(dtrCheck, nullptr);
    ASSERT_NE(rtsCheck, nullptr);
    EXPECT_TRUE(dtrCheck->isChecked());
    EXPECT_FALSE(rtsCheck->isChecked());

    rtsCheck->setChecked(true);
    EXPECT_TRUE(settings.value(core::common::settings_keys::kSerialDebuggerRts).toBool());
}

TEST(SerialDebuggerViewTest, AppliesSavedControlLinesOnOpenAndAlertsOnUnexpectedDisconnect) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(core::common::settings_keys::kSerialDebuggerDtr, true);
    settings.setValue(core::common::settings_keys::kSerialDebuggerRts, true);

    SerialDebuggerView view(&settings);
    int alertCount = 0;
    view.setDisconnectAlertHandler([&alertCount]() { ++alertCount; });

    QSignalSpy controlSpy(&view, &SerialDebuggerView::serialControlLinesApplied);
    QSignalSpy disconnectSpy(&view, &SerialDebuggerView::unexpectedDisconnectDetected);

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

TEST(SerialDebuggerViewTest, LocalizesWorkerErrorWrappingOsDetail) {
    tests::mocks::FakeSettingsService settings;
    SerialDebuggerView view(&settings);

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

} // namespace ui::views::serial
