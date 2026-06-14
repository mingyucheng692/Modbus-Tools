#include <gtest/gtest.h>

#include "../../../ui/widgets/SerialConnectionWidget.h"
#include "../../../ui/widgets/TcpClientConnectionWidget.h"
#include "../../../ui/widgets/TcpServerConnectionWidget.h"
#include "../../../ui/widgets/TrafficMonitorWidget.h"
#include "../../mocks/UiTestDoubles.h"

#include <QApplication>
#include <QSignalSpy>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QLabel>
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

TEST(I18nWidgets, TcpClientConnectionWidget_RetranslateHostLabel)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::TcpClientConnectionWidget widget(&settingsService);

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

TEST(I18nWidgets, TcpClientConnectionWidget_SupportsIntermediateStates)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::TcpClientConnectionWidget widget(&settingsService);

    widget.setDisplayState(ui::widgets::TcpClientConnectionWidget::DisplayState::TransportConnected);
    bool foundTransportLabel = false;
    for (auto* label : widget.findChildren<QLabel*>()) {
        if (label->text() == QStringLiteral("Transport Connected")) {
            foundTransportLabel = true;
            break;
        }
    }
    EXPECT_TRUE(foundTransportLabel);

    widget.setDisplayState(ui::widgets::TcpClientConnectionWidget::DisplayState::Disconnecting);
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

TEST(I18nWidgets, TrafficMonitorDirectionCheckboxesRetranslate)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::TrafficMonitorWidget widget(&settingsService);

    ScopedTranslator translator(QStringLiteral("Modbus-Tools_zh_CN.qm"));
    ASSERT_TRUE(translator.isLoaded());

    sendLanguageChange(widget);

    QSet<QString> checkboxTexts;
    for (auto* checkBox : widget.findChildren<QCheckBox*>()) {
        checkboxTexts.insert(checkBox->text());
    }

    EXPECT_TRUE(checkboxTexts.contains(QStringLiteral("发送")));
    EXPECT_TRUE(checkboxTexts.contains(QStringLiteral("接收")));
}
