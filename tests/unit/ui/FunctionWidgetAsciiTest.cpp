#include <gtest/gtest.h>

#include "../../../ui/widgets/FunctionWidget.h"
#include "../../mocks/UiTestDoubles.h"

#include <QPushButton>
#include <QTextEdit>

namespace {

QPushButton* findButtonByText(QWidget& widget, const QString& text)
{
    for (auto* button : widget.findChildren<QPushButton*>()) {
        if (button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

} // namespace

TEST(FunctionWidgetAsciiTest, AppendHelperEncodesAsciiFrameWithLrc)
{
    tests::mocks::FakeSettingsService settingsService;
    ui::widgets::FunctionWidget widget(&settingsService);
    widget.setTransportMode(ui::application::modbus::TransportUiMode::Ascii);

    auto* rawDataEdit = widget.findChild<QTextEdit*>();
    ASSERT_NE(rawDataEdit, nullptr);
    rawDataEdit->setPlainText(QStringLiteral("01 03 00 00 00 01"));

    auto* helperButton = findButtonByText(widget, QStringLiteral("Add LRC + Encode ASCII"));
    ASSERT_NE(helperButton, nullptr);
    helperButton->click();

    EXPECT_EQ(rawDataEdit->toPlainText(),
              QStringLiteral("3A 30 31 30 33 30 30 30 30 30 30 30 31 46 42 0D 0A"));
}

