// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "ui/widgets/NetworkConnectionWidget.h"
#include "ui/widgets/SerialConnectionWidget.h"
#include "ui/widgets/TcpConnectionWidget.h"
#include "ui/widgets/UdpConnectionWidget.h"
#include "mocks/UiTestDoubles.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
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

} // namespace
