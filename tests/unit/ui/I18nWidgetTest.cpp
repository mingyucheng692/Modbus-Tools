// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "../../../ui/widgets/SerialConnectionWidget.h"
#include "../../../ui/widgets/TcpConnectionWidget.h"
#include "../../../ui/widgets/modbus/ModbusTelemetryWidget.h"
#include "../../../ui/widgets/modbus/ModbusWorkbenchWidget.h"
#include "../../mocks/UiTestDoubles.h"

#include <QApplication>
#include <QSignalSpy>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QSet>
#include <QTranslator>

namespace {

class ScopedTranslator final {
public:
    explicit ScopedTranslator(const QString& fileName)
    {
        const QDir appDir(QCoreApplication::applicationDirPath());
        const QStringList candidates{
            QStringLiteral(":/i18n/%1").arg(fileName),
            appDir.filePath(fileName),
            appDir.filePath(QStringLiteral("../%1").arg(fileName)),
            appDir.filePath(QStringLiteral("../../translations/%1").arg(fileName)),
            appDir.filePath(QStringLiteral("../../../translations/%1").arg(fileName))
        };

        for (const QString& candidate : candidates) {
            if (translator_.load(candidate)) {
                loaded_ = true;
                qApp->installTranslator(&translator_);
                break;
            }
        }
    }

    ~ScopedTranslator()
    {
        if (loaded_) {
            qApp->removeTranslator(&translator_);
        }
    }

    [[nodiscard]] bool isLoaded() const
    {
        return loaded_;
    }

private:
    QTranslator translator_;
    bool loaded_ = false;
};

void sendLanguageChange(QWidget& widget)
{
    QEvent languageChangeEvent(QEvent::LanguageChange);
    QApplication::sendEvent(&widget, &languageChangeEvent);
}

} // namespace

TEST(I18nWidgets, TcpConnectionWidget_RetranslateHostLabel)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::TcpConnectionWidget widget(ui::widgets::TcpRole::Client, &settingsService);

    auto* hostLabel = widget.findChild<QLabel*>(QStringLiteral("hostLabel"));
    // The host label is not an object-named child, verify via text
    QString originalText;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("Host:")) {
            originalText = label->text();
            break;
        }
    }
    EXPECT_EQ(originalText, QStringLiteral("Host:"));

    ScopedTranslator translator(QStringLiteral("Modbus-Tools_zh_CN.qm"));
    ASSERT_TRUE(translator.isLoaded());

    sendLanguageChange(widget);

    bool foundTranslated = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() != QStringLiteral("Host:")) {
            foundTranslated = true;
            break;
        }
    }
    EXPECT_TRUE(foundTranslated);
}

TEST(I18nWidgets, SerialConnectionConfigParsingUsesStableOptionValuesAfterLanguageChange)
{
    tests::mocks::FakeSettingsService settingsService;
    settingsService.values_.insert(QStringLiteral("serial/test/baudRate"), 38400);
    settingsService.values_.insert(QStringLiteral("serial/test/dataBits"), QStringLiteral("8"));
    settingsService.values_.insert(QStringLiteral("serial/test/parity"), QStringLiteral("even"));
    settingsService.values_.insert(QStringLiteral("serial/test/stopBits"), QStringLiteral("2"));
    settingsService.values_.insert(QStringLiteral("serial/test/flowControl"), QStringLiteral("rts_cts"));

    ui::widgets::SerialConnectionWidget widget(&settingsService);
    widget.setSettingsGroup(QStringLiteral("serial/test"));

    ScopedTranslator translator(QStringLiteral("Modbus-Tools_zh_CN.qm"));
    ASSERT_TRUE(translator.isLoaded());

    sendLanguageChange(widget);

    const io::SerialConfig config = widget.getConfig();
    EXPECT_EQ(config.baudRate, 38400);
    EXPECT_EQ(config.dataBits, 8);
    EXPECT_EQ(config.parity, QSerialPort::EvenParity);
    EXPECT_EQ(config.stopBits, QSerialPort::TwoStop);
    EXPECT_EQ(config.flowControl, QSerialPort::HardwareControl);
}

TEST(I18nWidgets, TcpConnectionWidget_SupportsIntermediateStates)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::TcpConnectionWidget widget(ui::widgets::TcpRole::Client, &settingsService);

    widget.setDisplayState(ui::widgets::TcpConnectionWidget::DisplayState::TransportConnected);
    bool foundTransportLabel = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("Transport Connected")) {
            foundTransportLabel = true;
            break;
        }
    }
    EXPECT_TRUE(foundTransportLabel);

    widget.setDisplayState(ui::widgets::TcpConnectionWidget::DisplayState::Disconnecting);
    bool foundDisconnectingLabel = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("Disconnecting")) {
            foundDisconnectingLabel = true;
            break;
        }
    }
    EXPECT_TRUE(foundDisconnectingLabel);
}

TEST(I18nWidgets, SerialConnectionWidget_SupportsIntermediateStates)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::SerialConnectionWidget widget(&settingsService);

    widget.setDisplayState(ui::widgets::SerialConnectionWidget::DisplayState::TransportConnected);
    bool foundTransportLabel = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("Transport Connected")) {
            foundTransportLabel = true;
            break;
        }
    }
    EXPECT_TRUE(foundTransportLabel);

    widget.setDisplayState(ui::widgets::SerialConnectionWidget::DisplayState::Disconnecting);
    bool foundDisconnectingLabel = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("Disconnecting")) {
            foundDisconnectingLabel = true;
            break;
        }
    }
    EXPECT_TRUE(foundDisconnectingLabel);
}

TEST(I18nWidgets, ModbusTelemetryWidgetRetranslatesWithoutCrash)
{
    ui::widgets::modbus::ModbusTelemetryWidget widget;

    ScopedTranslator translator(QStringLiteral("Modbus-Tools_zh_CN.qm"));
    ASSERT_TRUE(translator.isLoaded());

    EXPECT_NO_THROW(sendLanguageChange(widget));
}

TEST(I18nWidgets, ModbusWorkbenchWidgetRetranslate)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::modbus::ModbusWorkbenchWidget widget(&settingsService);

    // 1. In default English locale, verify no QGroupBox title contains a single unescaped '&'.
    const auto initialGroupBoxes = widget.findChildren<QGroupBox*>();
    ASSERT_FALSE(initialGroupBoxes.isEmpty());
    for (auto* gb : initialGroupBoxes) {
        QString stripped = gb->title();
        stripped.replace(QStringLiteral("&&"), QString());
        EXPECT_FALSE(stripped.contains(QLatin1Char('&')))
            << "QGroupBox title contains single unescaped '&': " << gb->title().toStdString();
    }

    auto* fcCombo = widget.findChild<QComboBox*>(QStringLiteral("workbenchFcCombo"));
    ASSERT_NE(fcCombo, nullptr);
    EXPECT_EQ(fcCombo->currentIndex(), 2);
    EXPECT_EQ(fcCombo->itemText(0), QStringLiteral("0x01 Read Coils"));
    EXPECT_EQ(fcCombo->itemText(2), QStringLiteral("0x03 Read Holding Registers"));

    auto* addrBaseCombo = widget.findChild<QComboBox*>(QStringLiteral("workbenchAddrBaseCombo"));
    ASSERT_NE(addrBaseCombo, nullptr);
    EXPECT_EQ(addrBaseCombo->itemText(0), QStringLiteral("0-Based"));
    EXPECT_EQ(addrBaseCombo->itemText(1), QStringLiteral("1-Based"));

    // 2. In Chinese locale, verify titles, labels, buttons, and combo items are translated.
    ScopedTranslator translator(QStringLiteral("Modbus-Tools_zh_CN.qm"));
    ASSERT_TRUE(translator.isLoaded());

    sendLanguageChange(widget);

    QSet<QString> groupTitles;
    for (auto* gb : widget.findChildren<QGroupBox*>()) {
        groupTitles.insert(gb->title());
    }
    EXPECT_TRUE(groupTitles.contains(QStringLiteral("寻址与参数")));
    EXPECT_TRUE(groupTitles.contains(QStringLiteral("执行与轮询")));

    QSet<QString> labelTexts;
    for (auto* lbl : widget.findChildren<QLabel*>()) {
        labelTexts.insert(lbl->text());
    }
    EXPECT_TRUE(labelTexts.contains(QStringLiteral("功能码:")));
    EXPECT_TRUE(labelTexts.contains(QStringLiteral("起始地址:")));
    EXPECT_TRUE(labelTexts.contains(QStringLiteral("从站号 (0-247):")));
    EXPECT_TRUE(labelTexts.contains(QStringLiteral("数量:")));

    QSet<QString> buttonTexts;
    for (auto* btn : widget.findChildren<QPushButton*>()) {
        buttonTexts.insert(btn->text());
    }
    EXPECT_TRUE(buttonTexts.contains(QStringLiteral("执行 (F5)")));
    EXPECT_TRUE(buttonTexts.contains(QStringLiteral("标准模式")));
    EXPECT_TRUE(buttonTexts.contains(QStringLiteral("原始报文")));

    EXPECT_EQ(fcCombo->currentIndex(), 2);
    EXPECT_EQ(fcCombo->itemText(0), QStringLiteral("0x01 读线圈 (Read Coils)"));
    EXPECT_EQ(fcCombo->itemText(2), QStringLiteral("0x03 读保持寄存器 (Read Holding Registers)"));

    EXPECT_EQ(addrBaseCombo->itemText(0), QStringLiteral("0-Based (偏移寻址)"));
    EXPECT_EQ(addrBaseCombo->itemText(1), QStringLiteral("1-Based (常规寻址)"));
}

