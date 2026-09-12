/**
 * @file test_ModbusFrameBuilder.cpp
 * @brief Phase 8.4 black-box builder, persistence, translation and analyzer regression tests.
 *
 * Copyright (c) 2025 - present mingyucheng692
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "views/converter/ModbusFrameBuilderWidget.h"
#include "views/tools/UtilitiesView.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "application/analyzer/FrameAnalyzerPresenter.h"
#include "infra/config/SettingsService.h"
#include "infra/platform/PathResolver.h"
#include "mocks/UiTestDoubles.h"
#include "MainWindow.h"
#include "common/ThemeController.h"
#include "views/network/NetworkDebuggerView.h"
#include "widgets/TcpConnectionWidget.h"
#include "widgets/UdpConnectionWidget.h"
#include "widgets/GenericInputWidget.h"
#include "widgets/ByteMonitorWidget.h"
#include "widgets/ServerClientPanel.h"

#include <QListView>
#include <QListWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QEventLoop>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <functional>

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDir>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QTranslator>
#include <QTreeWidget>
#include <memory>
#include <tuple>

namespace {
using Builder = ui::views::converter::ModbusFrameBuilderWidget;
using Utilities = ui::views::tools::UtilitiesView;
using Analyzer = ui::widgets::FrameAnalyzerWidget;
using Presenter = ui::application::analyzer::FrameAnalyzerPresenter;

// Wait for the actual scheduled signal, not a sleep or an arbitrary event pump.
void settle(Builder& widget) {
    auto* timer = widget.findChild<QTimer*>();
    ASSERT_NE(timer, nullptr);
    if (timer->isActive()) {
        QSignalSpy updated(timer, &QTimer::timeout);
        ASSERT_TRUE(updated.wait(2000));
        EXPECT_EQ(updated.count(), 1);
    }
}

QComboBox* combo(Builder& widget, const char* name) {
    return widget.findChild<QComboBox*>(QString::fromLatin1(name));
}
QLineEdit* edit(Builder& widget, const char* name) {
    return widget.findChild<QLineEdit*>(QString::fromLatin1(name));
}
QPushButton* button(Builder& widget, const char* name) {
    return widget.findChild<QPushButton*>(QString::fromLatin1(name));
}
void configure(Builder& widget, int functionIndex, int format, const QString& data, int quantity = 2) {
    combo(widget, "builderFunction")->setCurrentIndex(functionIndex);
    combo(widget, "builderDataFormat")->setCurrentIndex(format);
    widget.findChild<QSpinBox*>(QStringLiteral("builderQuantity"))->setValue(quantity);
    edit(widget, "builderWriteData")->setText(data);
    settle(widget);
}
QByteArray rtuBytes(Builder& widget) {
    return QByteArray::fromHex(edit(widget, "builderOutput")->text().toLatin1());
}
void expectRejected(Builder& widget) {
    EXPECT_TRUE(edit(widget, "builderOutput")->text().isEmpty());
    EXPECT_FALSE(widget.findChild<QLabel*>(QStringLiteral("builderError"))->isHidden());
    EXPECT_EQ(widget.findChild<QTableWidget*>()->rowCount(), 0);
    for (const char* name : {"builderInspect", "builderCopySpaced", "builderCopyCompact", "builderCopyCArray"}) {
        EXPECT_FALSE(button(widget, name)->isEnabled());
    }
}

class FrameBuilderTest : public ::testing::Test {
protected:
    tests::mocks::FakeSettingsService settings;
    std::unique_ptr<Builder> widget;
    void SetUp() override { widget = std::make_unique<Builder>(&settings); }
};

TEST_F(FrameBuilderTest, DefaultRead_HasReferenceCrc) {
    EXPECT_EQ(edit(*widget, "builderOutput")->text(), QStringLiteral("01 03 00 00 00 02 C4 0B"));
    EXPECT_EQ(widget->findChild<QTableWidget*>()->rowCount(), 5);
}

TEST_F(FrameBuilderTest, NoSettingsService_UsesSameDefaultFunction) {
    Builder standalone(nullptr);
    EXPECT_EQ(edit(standalone, "builderOutput")->text(), QStringLiteral("01 03 00 00 00 02 C4 0B"));
}

class InvalidCoilTest : public FrameBuilderTest, public ::testing::WithParamInterface<const char*> {};
TEST_P(InvalidCoilTest, InvalidInput_DisablesAllActions) {
    configure(*widget, 4, 0, QString::fromLatin1(GetParam()));
    expectRejected(*widget);
}
INSTANTIATE_TEST_SUITE_P(DefensiveInput, InvalidCoilTest,
    ::testing::Values("", "garbage", "2", "-1", "0x", "FF00junk", "FFFF", "00 01"));

class ValidCoilTest : public FrameBuilderTest,
                      public ::testing::WithParamInterface<std::tuple<int, const char*, bool>> {};
TEST_P(ValidCoilTest, ExplicitValue_ProducesCanonicalPayload) {
    const auto [format, input, on] = GetParam();
    configure(*widget, 4, format, QString::fromLatin1(input));
    EXPECT_EQ(rtuBytes(*widget).mid(4, 2), QByteArray::fromHex(on ? "FF00" : "0000"));
}
INSTANTIATE_TEST_SUITE_P(Formats, ValidCoilTest, ::testing::Values(
    std::make_tuple(0, "ON", true), std::make_tuple(0, "OFF", false),
    std::make_tuple(0, "1", true), std::make_tuple(0, "0", false),
    std::make_tuple(0, "0xFF00", true), std::make_tuple(0, "0000", false),
    std::make_tuple(1, "65280", true), std::make_tuple(2, "1111111100000000", true)));

class RegisterFormatTest : public FrameBuilderTest,
                           public ::testing::WithParamInterface<std::tuple<int, const char*>> {};
TEST_P(RegisterFormatTest, MultipleRegisters_AgreeAcrossFormats) {
    const auto [format, input] = GetParam();
    configure(*widget, 7, format, QString::fromLatin1(input));
    EXPECT_EQ(rtuBytes(*widget).mid(6, 5), QByteArray::fromHex("0412345678"));
}
INSTANTIATE_TEST_SUITE_P(Formats, RegisterFormatTest, ::testing::Values(
    std::make_tuple(0, "12 34 56 78"), std::make_tuple(0, "0x1234 0x5678"),
    std::make_tuple(1, "4660, 22136"), std::make_tuple(2, "0001001000110100 0101011001111000")));

TEST_F(FrameBuilderTest, SingleRegisterBinary_IsMsbFirstNotCoilOrder) {
    configure(*widget, 5, 2, QStringLiteral("1000000000000001"));
    EXPECT_EQ(rtuBytes(*widget).mid(4, 2), QByteArray::fromHex("8001"));
    configure(*widget, 5, 2, QStringLiteral("10"));
    EXPECT_EQ(rtuBytes(*widget).mid(4, 2), QByteArray::fromHex("0002"));
}

class InvalidRegisterTest : public FrameBuilderTest,
                            public ::testing::WithParamInterface<std::tuple<int, const char*>> {};
TEST_P(InvalidRegisterTest, InvalidData_IsNotSilentlySanitized) {
    const auto [format, input] = GetParam();
    configure(*widget, 5, format, QString::fromLatin1(input));
    expectRejected(*widget);
}
INSTANTIATE_TEST_SUITE_P(DefensiveInput, InvalidRegisterTest, ::testing::Values(
    std::make_tuple(0, "12GG34"), std::make_tuple(0, "0x"), std::make_tuple(0, "123456"),
    std::make_tuple(1, "65536"), std::make_tuple(1, "-1"), std::make_tuple(1, "0x10"),
    std::make_tuple(2, "102"), std::make_tuple(2, "11111111111111111")));

TEST_F(FrameBuilderTest, MultipleRegistersBinary_RejectsCountMismatch) {
    configure(*widget, 7, 2, QStringLiteral("0000000000000001"), 2);
    expectRejected(*widget);
}

TEST_F(FrameBuilderTest, MultipleCoils_FormatsUseLsbFirstAndZeroPadding) {
    for (int format : {1, 2}) {
        configure(*widget, 6, format, QStringLiteral("1 0 1 1"), 4);
        EXPECT_EQ(rtuBytes(*widget).mid(6, 2), QByteArray::fromHex("010D"));
    }
    configure(*widget, 6, 0, QStringLiteral("FD"), 4);
    EXPECT_EQ(rtuBytes(*widget).mid(6, 2), QByteArray::fromHex("010D"));
}

TEST_F(FrameBuilderTest, MultipleCoils_InvalidDecimalAndBinaryAreRejected) {
    configure(*widget, 6, 1, QStringLiteral("1 2 0 1"), 4);
    expectRejected(*widget);
    configure(*widget, 6, 2, QStringLiteral("1x011"), 4);
    expectRejected(*widget);
    configure(*widget, 6, 1, QStringLiteral("1011"), 4);
    expectRejected(*widget);
}

TEST_F(FrameBuilderTest, AddressRangeOverflow_IsRejected) {
    edit(*widget, "builderAddress")->setText(QStringLiteral("65535"));
    settle(*widget);
    expectRejected(*widget);
    widget->findChild<QSpinBox*>()->setValue(1);
    settle(*widget);
    EXPECT_FALSE(edit(*widget, "builderOutput")->text().isEmpty());
}

TEST_F(FrameBuilderTest, OneBasedAddress_MapsToWireOffset) {
    combo(*widget, "builderAddressBase")->setCurrentIndex(1);
    edit(*widget, "builderAddress")->setText(QStringLiteral("1"));
    settle(*widget);
    EXPECT_EQ(rtuBytes(*widget).mid(2, 2), QByteArray::fromHex("0000"));
    edit(*widget, "builderAddress")->setText(QStringLiteral("0"));
    settle(*widget);
    expectRejected(*widget);
}

TEST_F(FrameBuilderTest, SlaveIdBoundary_IsProtocolSpecific) {
    for (const auto& value : {QStringLiteral("0"), QStringLiteral("248"), QStringLiteral("oops")}) {
        edit(*widget, "builderSlaveId")->setText(value);
        settle(*widget);
        expectRejected(*widget);
    }
    combo(*widget, "builderProtocol")->setCurrentIndex(1);
    edit(*widget, "builderSlaveId")->setText(QStringLiteral("255"));
    settle(*widget);
    EXPECT_FALSE(edit(*widget, "builderOutput")->text().isEmpty());
}

TEST_F(FrameBuilderTest, PendingRebuild_NeverCopiesStaleData) {
    auto* timer = widget->findChild<QTimer*>();
    QSignalSpy updated(timer, &QTimer::timeout);
    edit(*widget, "builderAddress")->setText(QStringLiteral("1"));
    edit(*widget, "builderAddress")->setText(QStringLiteral("2"));
    EXPECT_TRUE(edit(*widget, "builderOutput")->text().isEmpty());
    EXPECT_FALSE(button(*widget, "builderInspect")->isEnabled());
    ASSERT_TRUE(updated.wait(2000));
    EXPECT_EQ(updated.count(), 1);
    EXPECT_EQ(rtuBytes(*widget).mid(2, 2), QByteArray::fromHex("0002"));
}

TEST_F(FrameBuilderTest, CopyFormats_ContainExactFrame) {
    button(*widget, "builderCopySpaced")->click();
    EXPECT_EQ(QApplication::clipboard()->text(), QStringLiteral("01 03 00 00 00 02 C4 0B"));
    button(*widget, "builderCopyCompact")->click();
    EXPECT_EQ(QApplication::clipboard()->text(), QStringLiteral("010300000002C40B"));
    button(*widget, "builderCopyCArray")->click();
    EXPECT_EQ(QApplication::clipboard()->text(),
              QStringLiteral("const uint8_t frame[8] = { 0X01, 0X03, 0X00, 0X00, 0X00, 0X02, 0XC4, 0X0B };"));
    combo(*widget, "builderProtocol")->setCurrentIndex(2);
    settle(*widget);
    button(*widget, "builderCopySpaced")->click();
    EXPECT_EQ(QByteArray::fromHex(QApplication::clipboard()->text().toLatin1()), QByteArray(":010300000002FA\r\n"));
    button(*widget, "builderCopyCompact")->click();
    EXPECT_EQ(QApplication::clipboard()->text(), QStringLiteral(":010300000002FA"));
    button(*widget, "builderCopyCArray")->click();
    EXPECT_EQ(QApplication::clipboard()->text(), QStringLiteral("const char frame[] = \":010300000002FA\\r\\n\";"));
}

TEST(FrameBuilderPersistence, RecreatedService_RestoresAllFieldsFromDisk) {
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());
    infra::platform::PathResolver resolver(sandbox.path());
    QString expected;
    {
        infra::config::SettingsService settings(resolver);
        Builder widget(&settings);
        combo(widget, "builderProtocol")->setCurrentIndex(2);
        edit(widget, "builderSlaveId")->setText(QStringLiteral("17"));
        combo(widget, "builderAddressBase")->setCurrentIndex(1);
        edit(widget, "builderAddress")->setText(QStringLiteral("108"));
        configure(widget, 7, 1, QStringLiteral("4660 22136"), 2);
        expected = edit(widget, "builderOutput")->text();
        settings.sync();
    }
    infra::config::SettingsService settings(resolver);
    Builder restored(&settings);
    EXPECT_EQ(combo(restored, "builderProtocol")->currentIndex(), 2);
    EXPECT_EQ(combo(restored, "builderFunction")->currentIndex(), 7);
    EXPECT_EQ(combo(restored, "builderDataFormat")->currentIndex(), 1);
    EXPECT_EQ(combo(restored, "builderAddressBase")->currentIndex(), 1);
    EXPECT_EQ(edit(restored, "builderSlaveId")->text(), QStringLiteral("17"));
    EXPECT_EQ(edit(restored, "builderAddress")->text(), QStringLiteral("108"));
    EXPECT_EQ(edit(restored, "builderWriteData")->text(), QStringLiteral("4660 22136"));
    EXPECT_EQ(restored.findChild<QSpinBox*>()->value(), 2);
    EXPECT_EQ(edit(restored, "builderOutput")->text(), expected);
}

class BuilderAnalyzerRoundTrip : public ::testing::TestWithParam<std::tuple<int, int>> {};
TEST_P(BuilderAnalyzerRoundTrip, UtilitiesSignal_ParsesAllProtocolsAndFunctions) {
    const auto [protocol, function] = GetParam();
    tests::mocks::FakeSettingsService settings;
    Utilities tools(&settings);
    auto* widget = tools.findChild<Builder*>();
    ASSERT_NE(widget, nullptr);
    ASSERT_EQ(tools.findChild<QTabWidget*>()->count(), 2);
    combo(*widget, "builderProtocol")->setCurrentIndex(protocol);
    const QString data = function == 4 ? QStringLiteral("ON") :
                         function == 5 ? QStringLiteral("1234") :
                         function == 6 ? QStringLiteral("03") : QStringLiteral("1234 5678");
    configure(*widget, function, 0, data);
    ASSERT_FALSE(edit(*widget, "builderOutput")->text().isEmpty());

    Analyzer analyzer(&settings);
    // A stale manual protocol and response-address field must not block Inspect.
    analyzer.findChild<QComboBox*>()->setCurrentIndex(protocol == 1 ? 2 : 1);
    analyzer.findChild<QLineEdit*>()->setText(QStringLiteral("invalid"));
    auto* presenter = analyzer.findChild<Presenter*>();
    ASSERT_NE(presenter, nullptr);
    QSignalSpy parsed(presenter, &Presenter::parseFinished);
    QSignalSpy forwarded(&tools, &Utilities::inspectInAnalyzerRequested);
    QObject::connect(&tools, &Utilities::inspectInAnalyzerRequested,
                     &analyzer, &Analyzer::loadAndParseHex);
    button(*widget, "builderInspect")->click();
    EXPECT_EQ(forwarded.count(), 1);
    ASSERT_TRUE(parsed.wait(3000));
    const auto result = qvariant_cast<modbus::parser::ParseResult>(parsed.at(0).at(0));
    EXPECT_TRUE(result.isValid) << result.error.toStdString();
    EXPECT_EQ(analyzer.findChild<QPlainTextEdit*>()->toPlainText(), edit(*widget, "builderOutput")->text());
    EXPECT_GT(analyzer.findChild<QTreeWidget*>()->topLevelItemCount(), 0);
}
INSTANTIATE_TEST_SUITE_P(ProtocolFunctionMatrix, BuilderAnalyzerRoundTrip,
    ::testing::Combine(::testing::Range(0, 3), ::testing::Range(0, 8)));

class BuilderTranslationTest : public ::testing::TestWithParam<const char*> {};
TEST_P(BuilderTranslationTest, LanguageChange_UpdatesControlsAndKeepsFrame) {
    tests::mocks::FakeSettingsService settings;
    Utilities tools(&settings);
    auto* widget = tools.findChild<Builder*>();
    ASSERT_NE(widget, nullptr);
    const QString original = edit(*widget, "builderOutput")->text();
    QTranslator translator;
    const QString file = QStringLiteral("Modbus-Tools_%1.qm").arg(QString::fromLatin1(GetParam()));
    const QDir appDir(QCoreApplication::applicationDirPath());
    const bool loaded = translator.load(QStringLiteral(":/i18n/") + file) ||
                        translator.load(appDir.filePath(QStringLiteral("../../translations/") + file));
    ASSERT_TRUE(loaded);
    QApplication::installTranslator(&translator);
    QEvent event(QEvent::LanguageChange);
    QApplication::sendEvent(&tools, &event);
    QApplication::sendEvent(widget, &event);
    EXPECT_NE(tools.findChild<QTabWidget*>()->tabText(1), QStringLiteral("Frame Builder"));
    EXPECT_NE(button(*widget, "builderInspect")->text(), QStringLiteral("Inspect in Frame Analyzer"));
    bool translatedFunctionLabel = false;
    for (auto* label : widget->findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("功能码:") || label->text() == QStringLiteral("功能碼:")) translatedFunctionLabel = true;
    }
    EXPECT_TRUE(translatedFunctionLabel);
    EXPECT_EQ(edit(*widget, "builderOutput")->text(), original);
    configure(*widget, 4, 0, QStringLiteral("invalid"));
    const QString localizedError = widget->findChild<QLabel*>(QStringLiteral("builderError"))->text();
    EXPECT_FALSE(localizedError.startsWith(QStringLiteral("Invalid coil")));
    QApplication::removeTranslator(&translator);
    QApplication::sendEvent(&tools, &event);
    QApplication::sendEvent(widget, &event);
    EXPECT_EQ(tools.findChild<QTabWidget*>()->tabText(1), QStringLiteral("Frame Builder"));
}
INSTANTIATE_TEST_SUITE_P(Locales, BuilderTranslationTest, ::testing::Values("zh_CN", "zh_TW"));

// Bounded event-loop waiting keeps network tests deterministic without sleeping.
bool await(const std::function<bool()>& ready) {
    if (ready()) return true;
    QEventLoop loop;
    QTimer probe;
    QTimer deadline;
    deadline.setSingleShot(true);
    QObject::connect(&probe, &QTimer::timeout, &loop, [&]() { if (ready()) loop.quit(); });
    QObject::connect(&deadline, &QTimer::timeout, &loop, &QEventLoop::quit);
    probe.start(5);
    deadline.start(3000);
    loop.exec();
    return ready();
}

TEST(UtilitiesNavigation, Inspect_ChangesMainWindowPageAndParsesFrame) {
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());
    infra::platform::PathResolver resolver(sandbox.path());
    infra::config::SettingsService settings(resolver);
    settings.setValue(QStringLiteral("app/disclaimerAccepted"), true);
    settings.setValue(QStringLiteral("app/updateCheckFrequency"), QStringLiteral("never"));
    settings.setValue(QStringLiteral("app/language"), QStringLiteral("en_US"));
    ui::common::ThemeController theme(settings);
    ui::MainWindow window(&settings, &theme, resolver);
    auto* tools = window.findChild<Utilities*>();
    auto* analyzer = window.findChild<Analyzer*>();
    QStackedWidget* stack = nullptr;
    for (auto* candidate : window.findChildren<QStackedWidget*>()) {
        if (candidate->count() == 5 && candidate->widget(3)->isAncestorOf(analyzer)) stack = candidate;
    }
    ASSERT_NE(tools, nullptr);
    ASSERT_NE(analyzer, nullptr);
    ASSERT_NE(stack, nullptr);
    auto* builder = tools->findChild<Builder*>();
    auto* presenter = analyzer->findChild<Presenter*>();
    ASSERT_NE(builder, nullptr);
    ASSERT_NE(presenter, nullptr);
    QSignalSpy parsed(presenter, &Presenter::parseFinished);
    button(*builder, "builderInspect")->click();
    EXPECT_EQ(stack->currentIndex(), 3);
    EXPECT_TRUE(stack->currentWidget()->isAncestorOf(analyzer));
    ASSERT_TRUE(parsed.wait(3000));
    EXPECT_TRUE(qvariant_cast<modbus::parser::ParseResult>(parsed.at(0).at(0)).isValid);
}

using Network = ui::views::network::NetworkDebuggerView;

bool monitorContains(Network& view, const QString& fragment) {
    auto* monitor = view.findChild<ui::widgets::ByteMonitorWidget*>();
    if (!monitor) return false;
    auto* list = monitor->findChild<QListView*>();
    if (!list || !list->model()) return false;
    for (int row = 0; row < list->model()->rowCount(); ++row) {
        if (list->model()->data(list->model()->index(row, 0)).toString().contains(fragment)) return true;
    }
    return false;
}

TEST(NetworkDebuggerLoopback, TcpClient_SendReceiveAndDisconnect) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    ui::widgets::TcpConnectionWidget* connection = nullptr;
    for (auto* candidate : view.findChildren<ui::widgets::TcpConnectionWidget*>()) {
        if (candidate->role() == ui::widgets::TcpRole::Client) connection = candidate;
    }
    auto* input = view.findChild<ui::widgets::GenericInputWidget*>();
    ASSERT_NE(connection, nullptr);
    ASSERT_NE(input, nullptr);
    connection->connectClicked(QStringLiteral("127.0.0.1"), server.serverPort());
    ASSERT_TRUE(await([&]() { return server.hasPendingConnections() && monitorContains(view, QStringLiteral("Connected")); }));
    std::unique_ptr<QTcpSocket> peer(server.nextPendingConnection());
    input->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer->bytesAvailable() >= 4; }));
    EXPECT_EQ(peer->readAll(), QByteArray::fromHex("12345678"));
    peer->write(QByteArray::fromHex("DEADBEEF"));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    connection->disconnectClicked();
    EXPECT_TRUE(await([&]() { return peer->state() == QAbstractSocket::UnconnectedState; }));
}

TEST(NetworkDebuggerLoopback, TcpServer_SendReceiveAndStop) {
    QTcpServer reservation;
    ASSERT_TRUE(reservation.listen(QHostAddress::LocalHost, 0));
    const int port = reservation.serverPort();
    reservation.close();
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.findChild<QComboBox*>()->setCurrentIndex(1);
    ui::widgets::TcpConnectionWidget* connection = nullptr;
    for (auto* candidate : view.findChildren<ui::widgets::TcpConnectionWidget*>()) {
        if (candidate->role() == ui::widgets::TcpRole::Server) connection = candidate;
    }
    ASSERT_NE(connection, nullptr);
    connection->startListenClicked(QStringLiteral("127.0.0.1"), port);
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Listening")); }));
    QTcpSocket peer;
    peer.connectToHost(QHostAddress::LocalHost, port);
    auto* panel = view.findChild<ui::widgets::ServerClientPanel*>();
    ASSERT_NE(panel, nullptr);
    ASSERT_TRUE(await([&]() { return panel->hasClients() && peer.state() == QAbstractSocket::ConnectedState; }));
    panel->findChild<QCheckBox*>()->setChecked(true);
    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer.bytesAvailable() >= 4; }));
    EXPECT_EQ(peer.readAll(), QByteArray::fromHex("12345678"));
    peer.write(QByteArray::fromHex("DEADBEEF"));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    connection->stopListenClicked();
    EXPECT_TRUE(await([&]() { return peer.state() == QAbstractSocket::UnconnectedState; }));
}

TEST(NetworkDebuggerLoopback, Udp_SendReceiveAndUnbind) {
    QUdpSocket peer;
    ASSERT_TRUE(peer.bind(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.findChild<QComboBox*>()->setCurrentIndex(2);
    auto* connection = view.findChild<ui::widgets::UdpConnectionWidget*>();
    ASSERT_NE(connection, nullptr);
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Connected")); }));
    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer.hasPendingDatagrams(); }));
    const QNetworkDatagram received = peer.receiveDatagram();
    EXPECT_EQ(received.data(), QByteArray::fromHex("12345678"));
    peer.writeDatagram(QByteArray::fromHex("DEADBEEF"), received.senderAddress(), received.senderPort());
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Closed")); }));
}

} // namespace
