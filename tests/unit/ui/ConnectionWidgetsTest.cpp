// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "ui/widgets/NetworkConnectionWidget.h"
#include "ui/widgets/SerialConnectionWidget.h"
#include "ui/widgets/TcpConnectionWidget.h"
#include "ui/widgets/UdpConnectionWidget.h"
#include "ui/widgets/CollapsibleSection.h"
#include "ui/widgets/modbus/ModbusWorkbenchWidget.h"
#include "ui/widgets/modbus/ModbusTelemetryWidget.h"
#include "ui/views/modbus/ModbusPage.h"
#include "mocks/UiTestDoubles.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTranslator>

namespace {

class ScopedTranslator final {
public:
    explicit ScopedTranslator(const QString& fileName) {
        const QDir appDir(QCoreApplication::applicationDirPath());
        const QStringList candidates{
            QStringLiteral(":/i18n/%1").arg(fileName),
            appDir.filePath(fileName),
            appDir.filePath(QStringLiteral("../%1").arg(fileName)),
            appDir.filePath(QStringLiteral("../../translations/%1").arg(fileName)),
            appDir.filePath(QStringLiteral("../../../translations/%1").arg(fileName)),
        };
        for (const QString& candidate : candidates) {
            if (translator_.load(candidate)) {
                loaded_ = true;
                qApp->installTranslator(&translator_);
                break;
            }
        }
    }

    ~ScopedTranslator() {
        if (loaded_) {
            qApp->removeTranslator(&translator_);
        }
    }

    [[nodiscard]] bool isLoaded() const { return loaded_; }

private:
    QTranslator translator_;
    bool loaded_ = false;
};

void sendLanguageChange(QWidget& widget) {
    QEvent languageChangeEvent(QEvent::LanguageChange);
    QApplication::sendEvent(&widget, &languageChangeEvent);
}

class MapTranslator final : public QTranslator {
public:
    QHash<QString, QString> map;

    bool isEmpty() const override { return map.isEmpty(); }

    QString translate(const char* context,
                      const char* sourceText,
                      const char* disambiguation,
                      int n) const override {
        Q_UNUSED(context);
        Q_UNUSED(disambiguation);
        Q_UNUSED(n);
        return map.value(QString::fromUtf8(sourceText));
    }
};

class ScopedAppTranslator final {
public:
    explicit ScopedAppTranslator(QTranslator* translator)
        : translator_(translator) {
        if (translator_) {
            qApp->installTranslator(translator_);
        }
    }

    ~ScopedAppTranslator() {
        if (translator_) {
            qApp->removeTranslator(translator_);
        }
    }

private:
    QTranslator* translator_;
};

QSpinBox* findRemotePortSpinBox(ui::widgets::UdpConnectionWidget& widget) {
    for (auto* spin : widget.findChildren<QSpinBox*>()) {
        if (spin->minimum() == 0) {
            return spin;
        }
    }
    return nullptr;
}

TEST(ConnectionWidgets, UdpRemotePortZeroDisplaysDashAndAllowsOne) {
    tests::mocks::FakeSettingsService settings;
    ui::widgets::UdpConnectionWidget widget(&settings);
    auto* remotePortSpin = findRemotePortSpinBox(widget);
    ASSERT_NE(remotePortSpin, nullptr);

    remotePortSpin->setValue(0);
    EXPECT_EQ(remotePortSpin->text(), QStringLiteral("-"));

    remotePortSpin->setValue(1);
    EXPECT_EQ(remotePortSpin->text(), QStringLiteral("1"));
}

TEST(ConnectionWidgets, UdpSettingsAreScopedPerGroup) {
    tests::mocks::FakeSettingsService settings;
    ui::widgets::UdpConnectionWidget first(&settings);
    ui::widgets::UdpConnectionWidget second(&settings);

    first.setSettingsGroup(QStringLiteral("udpGroupA"));
    second.setSettingsGroup(QStringLiteral("udpGroupB"));

    auto* firstRemotePort = findRemotePortSpinBox(first);
    auto* secondRemotePort = findRemotePortSpinBox(second);
    ASSERT_NE(firstRemotePort, nullptr);
    ASSERT_NE(secondRemotePort, nullptr);

    firstRemotePort->setValue(9001);
    secondRemotePort->setValue(9002);

    ui::widgets::UdpConnectionWidget reloaded(&settings);
    reloaded.setSettingsGroup(QStringLiteral("udpGroupA"));
    auto* reloadedRemotePort = findRemotePortSpinBox(reloaded);
    ASSERT_NE(reloadedRemotePort, nullptr);
    EXPECT_EQ(reloadedRemotePort->value(), 9001);
}

TEST(ConnectionWidgets, SerialCtorDoesNotPolluteSettingsAndPreservesOfflinePort) {
    tests::mocks::FakeSettingsService settings;
    {
        ui::widgets::SerialConnectionWidget initial(&settings);
        EXPECT_FALSE(settings.contains(QStringLiteral("serial/portName")));
        EXPECT_FALSE(settings.contains(QStringLiteral("modbus/rtu/serial/portName")));
    }

    settings.setValue(QStringLiteral("serialGrp/portName"), QStringLiteral("COM99_OFFLINE_TEST"));
    ui::widgets::SerialConnectionWidget widget(&settings);
    widget.setSettingsGroup(QStringLiteral("serialGrp"));

    auto* portCombo = widget.findChild<QComboBox*>();
    ASSERT_NE(portCombo, nullptr);
    EXPECT_EQ(widget.getConfig().portName, QStringLiteral("COM99_OFFLINE_TEST"));
    EXPECT_TRUE(portCombo->currentText().contains(QStringLiteral("Offline")));

    widget.refreshPorts();
    EXPECT_EQ(widget.getConfig().portName, QStringLiteral("COM99_OFFLINE_TEST"));

    MapTranslator translator;
    translator.map.insert(QStringLiteral("%1 (Offline)"), QStringLiteral("%1 (离线)"));
    ScopedAppTranslator installed(&translator);
    sendLanguageChange(widget);
    EXPECT_EQ(widget.getConfig().portName, QStringLiteral("COM99_OFFLINE_TEST"));
    EXPECT_EQ(portCombo->currentText(), QStringLiteral("COM99_OFFLINE_TEST (离线)"));
}

TEST(ConnectionWidgets, NetworkPreservesDefaultPortWhenPortKeyMissing) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(QStringLiteral("tcpServerGrp/ip"), QStringLiteral("10.0.0.5"));

    ui::widgets::TcpConnectionWidget widget(ui::widgets::TcpRole::Server, &settings);
    widget.setDefaultPort(1502);
    widget.setSettingsGroup(QStringLiteral("tcpServerGrp"));

    EXPECT_EQ(widget.getIpAddress(), QStringLiteral("10.0.0.5"));
    EXPECT_EQ(widget.getPort(), 1502);
}

TEST(ConnectionWidgets, TcpAndUdpRetranslateActiveStateLabels) {
    ScopedTranslator zhCn(QStringLiteral("Modbus-Tools_zh_CN.qm"));
    ASSERT_TRUE(zhCn.isLoaded());

    tests::mocks::FakeSettingsService settings;
    ui::widgets::TcpConnectionWidget clientWidget(ui::widgets::TcpRole::Client, &settings);
    clientWidget.setDisplayState(ui::widgets::BaseConnectionWidget::DisplayState::Connected);
    sendLanguageChange(clientWidget);

    bool foundChineseHostLabel = false;
    for (auto* label : clientWidget.findChildren<QLabel*>()) {
        if (label->text().contains(QStringLiteral("主机"))) {
            foundChineseHostLabel = true;
        }
    }
    EXPECT_TRUE(foundChineseHostLabel);

    MapTranslator custom;
    custom.map.insert(QStringLiteral("Listen:"), QStringLiteral("监听："));
    custom.map.insert(QStringLiteral("Local:"), QStringLiteral("本地："));
    custom.map.insert(QStringLiteral("Remote:"), QStringLiteral("目标："));
    custom.map.insert(QStringLiteral("Remote Port:"), QStringLiteral("目标端口："));
    ScopedAppTranslator installed(&custom);

    ui::widgets::TcpConnectionWidget serverWidget(ui::widgets::TcpRole::Server, &settings);
    serverWidget.setDisplayState(ui::widgets::BaseConnectionWidget::DisplayState::Listening);
    sendLanguageChange(serverWidget);

    bool foundListenLabel = false;
    for (auto* label : serverWidget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("监听：")) {
            foundListenLabel = true;
        }
    }
    EXPECT_TRUE(foundListenLabel);

    ui::widgets::UdpConnectionWidget udpWidget(&settings);
    udpWidget.setDisplayState(ui::widgets::BaseConnectionWidget::DisplayState::Connected);
    sendLanguageChange(udpWidget);

    bool foundLocalLabel = false;
    bool foundRemoteLabel = false;
    bool foundRemotePortLabel = false;
    for (auto* label : udpWidget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("本地：")) foundLocalLabel = true;
        if (label->text() == QStringLiteral("目标：")) foundRemoteLabel = true;
        if (label->text() == QStringLiteral("目标端口：")) foundRemotePortLabel = true;
    }
    EXPECT_TRUE(foundLocalLabel);
    EXPECT_TRUE(foundRemoteLabel);
    EXPECT_TRUE(foundRemotePortLabel);
}

TEST(ConnectionWidgets, ModbusPage_ProtocolSectionAlignsWithConnectionStack) {
    tests::mocks::FakeSettingsService settings;
    ui::views::modbus::ModbusPage page(&settings);
    page.resize(900, 600);
    page.show();
    QCoreApplication::processEvents();

    auto* protoLabel = page.protocolLabel();
    ASSERT_NE(protoLabel, nullptr);
    auto* protoSection = page.findChild<ui::widgets::CollapsibleSection*>(QStringLiteral("modbusProtocolSection"));
    ASSERT_NE(protoSection, nullptr);
    auto* connStack = page.findChild<QStackedWidget*>();
    ASSERT_NE(connStack, nullptr);

    // Initial state (expanded)
    EXPECT_LE(std::abs(protoSection->geometry().y() - connStack->geometry().y()), 2);
    EXPECT_LE(std::abs(protoSection->height() - connStack->height()), 8);

    // Collapse connection settings
    auto* currentConn = qobject_cast<ui::widgets::BaseConnectionWidget*>(connStack->currentWidget());
    ASSERT_NE(currentConn, nullptr);
    currentConn->setSectionExpanded(false);
    QCoreApplication::processEvents();

    EXPECT_FALSE(protoSection->isExpanded());
    EXPECT_LE(std::abs(protoSection->geometry().y() - connStack->geometry().y()), 2);
    EXPECT_LE(std::abs(protoSection->height() - connStack->height()), 8);

    // Re-expand
    currentConn->setSectionExpanded(true);
    QCoreApplication::processEvents();
    EXPECT_TRUE(protoSection->isExpanded());
    EXPECT_LE(std::abs(protoSection->geometry().y() - connStack->geometry().y()), 2);
    EXPECT_LE(std::abs(protoSection->height() - connStack->height()), 8);
}

TEST(ConnectionWidgets, NetworkConnectionWidget_HostWidthIsBounded) {
    tests::mocks::FakeSettingsService settings;
    ui::widgets::TcpConnectionWidget widget(ui::widgets::TcpRole::Client, &settings);
    widget.show();

    auto* ipEdit = widget.findChild<QLineEdit*>();
    ASSERT_NE(ipEdit, nullptr);
    auto* portEdit = widget.findChild<QSpinBox*>();
    ASSERT_NE(portEdit, nullptr);

    EXPECT_EQ(ipEdit->minimumWidth(), 128);
    EXPECT_EQ(ipEdit->maximumWidth(), 196);

    widget.resize(560, 60);
    QCoreApplication::processEvents();
    const int gap560 = portEdit->geometry().x() - ipEdit->geometry().right();
    const int ipWidth560 = ipEdit->width();

    widget.resize(1100, 60);
    QCoreApplication::processEvents();
    const int gap1100 = portEdit->geometry().x() - ipEdit->geometry().right();

    EXPECT_LE(ipEdit->width(), 196);
    // Gap between ipEdit and portEdit remains constant (Host anchor locked)
    EXPECT_EQ(gap560, gap1100);
}

TEST(ConnectionWidgets, SerialConnectionWidget_ResponsiveLayoutNarrowAndWide) {
    tests::mocks::FakeSettingsService settings;
    ui::widgets::SerialConnectionWidget widget(&settings);
    widget.show();

    // Narrow width (< 630px): should be in 2-row layout with low minimum width
    widget.resize(500, 80);
    QCoreApplication::processEvents();
    EXPECT_TRUE(widget.isTwoRowLayout());
    EXPECT_LE(widget.minimumSizeHint().width(), 380);

    // Wide width (>= 650px): should switch to 1-row layout
    widget.resize(750, 60);
    QCoreApplication::processEvents();
    EXPECT_FALSE(widget.isTwoRowLayout());

    // Narrow width again: should fold back to 2-row layout
    widget.resize(520, 80);
    QCoreApplication::processEvents();
    EXPECT_TRUE(widget.isTwoRowLayout());
}

TEST(ConnectionWidgets, ModbusPage_SplitterZeroOverlapInNarrowWindow) {
    tests::mocks::FakeSettingsService settings;
    ui::views::modbus::ModbusPage page(&settings);
    page.resize(720, 520);
    page.show();
    QCoreApplication::processEvents();

    auto* workbench = page.findChild<ui::widgets::modbus::ModbusWorkbenchWidget*>();
    auto* telemetry = page.findChild<ui::widgets::modbus::ModbusTelemetryWidget*>();
    auto* splitter = page.findChild<QSplitter*>();
    ASSERT_NE(workbench, nullptr);
    ASSERT_NE(telemetry, nullptr);
    ASSERT_NE(splitter, nullptr);

    EXPECT_FALSE(splitter->childrenCollapsible());
    EXPECT_LE(workbench->geometry().right(), telemetry->geometry().left());
    EXPECT_EQ(workbench->geometry().bottom(), telemetry->geometry().bottom());
}

} // namespace
