#include "AboutDialog.h"
#include "../../core/AppConstants.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QApplication>

namespace ui::widgets {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setupUi();
}

void AboutDialog::setupUi() {
    setWindowTitle(QCoreApplication::translate("ui::MainWindow", "About"));
    setMinimumWidth(app::constants::Values::Ui::kAboutDialogMinWidth);

    auto* layout = new QVBoxLayout(this);
    auto* aboutLabel = new QLabel(this);
    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutLabel->setText(QCoreApplication::translate("ui::MainWindow", "Welcome to Modbus-Tools<br>"
                            "Version: v%1<br><br>"
                            "An open-source Modbus communication debugging assistant.<br>"
                            "Developer: mingyucheng692<br>"
                            "License: MIT License<br><br>"
                            "This project is developed in spare time, completely free and open-source.<br>"
                            "Feel free to star on GitHub or submit issues.<br>"
                            "Your feedback keeps the project improving!<br><br>"
                            "<a href=\"https://github.com/mingyucheng692/Modbus-Tools\">🌐 Visit GitHub Repository</a>"
                            "&nbsp;&nbsp;&nbsp;"
                            "<a href=\"https://github.com/mingyucheng692/Modbus-Tools/issues\">🐛 Issue Tracker</a>"
                            "<br><br>"
                            "--------------------------<br>"
                            "This software is provided &quot;as is&quot; without warranty of any kind.")
                           .arg(QApplication::applicationVersion()));
    layout->addWidget(aboutLabel);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

} // namespace ui::widgets
