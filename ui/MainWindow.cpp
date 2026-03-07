#include "MainWindow.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/generic_serial/GenericSerialView.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "widgets/DisclaimerDialog.h"
#include "common/Theme.h"
#include "common/UpdateChecker.h"
#include <QDesktopServices>
#include <QStackedWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QEvent>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <spdlog/spdlog.h>
#include "modbus/base/ModbusConfig.h"

namespace ui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
}

MainWindow::~MainWindow() {
    spdlog::info("MainWindow: Destructor entry");
}

void MainWindow::setupUi() {
    setWindowTitle(tr("Modbus Tools"));
    resize(1200, 800);

    // Central widget with HBox layout
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Navigation
    createNavigation();
    mainLayout->addWidget(navigationList_, 1);

    // Content Area
    stackedWidget_ = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget_, 5); // Ratio 1:5

    // Add Views
    modbusTcpView_ = new views::modbus_tcp::ModbusTcpView(this);
    stackedWidget_->addWidget(modbusTcpView_);
    modbusRtuView_ = new views::modbus_rtu::ModbusRtuView(this);
    stackedWidget_->addWidget(modbusRtuView_);
    stackedWidget_->addWidget(new views::generic_tcp::GenericTcpView(this));
    stackedWidget_->addWidget(new views::generic_serial::GenericSerialView(this));
    stackedWidget_->addWidget(new widgets::FrameAnalyzerWidget(this));

    // Connect Navigation
    connect(navigationList_, &QListWidget::currentRowChanged,
            stackedWidget_, &QStackedWidget::setCurrentIndex);
            
    // Set default view
    navigationList_->setCurrentRow(0);

    updateChecker_ = new common::UpdateChecker(this);
    connect(updateChecker_, &common::UpdateChecker::updateAvailable, this, &MainWindow::handleUpdateAvailable);
    connect(updateChecker_, &common::UpdateChecker::noUpdateAvailable, this, &MainWindow::handleNoUpdateAvailable);
    connect(updateChecker_, &common::UpdateChecker::checkFailed, this, &MainWindow::handleUpdateCheckFailed);

    setupLanguageMenu();
    setupSettingsMenu();
    setupAboutMenu();
    {
        QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
        currentLocale_ = settings.value("app/language", currentLocale_).toString();
        if (!settings.contains("app/language")) {
            settings.setValue("app/language", currentLocale_);
        }
    }
    loadModbusSettings();
    applyModbusSettingsToViews();
    applyLanguage(currentLocale_);
    showDisclaimerIfNeeded();
}

void MainWindow::createNavigation() {
    navigationList_ = new QListWidget(this);
    navigationList_->addItem(tr("Modbus TCP"));
    navigationList_->addItem(tr("Modbus RTU"));
    navigationList_->addItem(tr("TCP Client"));
    navigationList_->addItem(tr("Serial Port"));
    navigationList_->addItem(tr("Frame Analyzer"));
    
    // Style the navigation list a bit
    // Use light theme by default for now
    navigationList_->setStyleSheet(common::Theme::getNavigationStyle(false));
}

void MainWindow::setupSettingsMenu() {
    settingsMenu_ = menuBar()->addMenu(tr("Settings"));
    modbusSettingsAction_ = settingsMenu_->addAction(tr("Modbus Settings"));
    connect(modbusSettingsAction_, &QAction::triggered, this, &MainWindow::openSettingsDialog);
}

void MainWindow::setupLanguageMenu() {
    languageMenu_ = menuBar()->addMenu(tr("Language"));
    languageActionGroup_ = new QActionGroup(this);
    languageActionGroup_->setExclusive(true);

    langEnAction_ = languageMenu_->addAction(tr("English (US)"));
    langEnAction_->setCheckable(true);
    langEnAction_->setData("en_US");
    languageActionGroup_->addAction(langEnAction_);

    langZhCnAction_ = languageMenu_->addAction(tr("简体中文"));
    langZhCnAction_->setCheckable(true);
    langZhCnAction_->setData("zh_CN");
    languageActionGroup_->addAction(langZhCnAction_);

    langZhTwAction_ = languageMenu_->addAction(tr("繁體中文"));
    langZhTwAction_->setCheckable(true);
    langZhTwAction_->setData("zh_TW");
    languageActionGroup_->addAction(langZhTwAction_);

    connect(languageActionGroup_, &QActionGroup::triggered, this, [this](QAction* action) {
        applyLanguage(action->data().toString());
    });
}

void MainWindow::setupAboutMenu() {
    aboutMenu_ = menuBar()->addMenu(tr("About"));
    checkUpdatesAction_ = aboutMenu_->addAction(tr("Check for Updates"));
    connect(checkUpdatesAction_, &QAction::triggered, this, &MainWindow::checkForUpdates);
    aboutMenu_->addSeparator();
    aboutAction_ = aboutMenu_->addAction(tr("About"));
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::openAboutDialog);
}

void MainWindow::loadModbusSettings() {
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    modbus::base::ModbusConfig defaultConfig;
    modbusTimeoutMs_ = settings.value("modbus/timeoutMs", defaultConfig.timeoutMs).toInt();
    modbusRetries_ = settings.value("modbus/retryCount", defaultConfig.retries).toInt();
    modbusRetryIntervalMs_ = settings.value("modbus/retryIntervalMs", defaultConfig.retryIntervalMs).toInt();
    modbusRetryEnabled_ = settings.value("modbus/retryEnabled", false).toBool();
    if (!settings.contains("modbus/timeoutMs")) {
        settings.setValue("modbus/timeoutMs", modbusTimeoutMs_);
    }
    if (!settings.contains("modbus/retryCount")) {
        settings.setValue("modbus/retryCount", modbusRetries_);
    }
    if (!settings.contains("modbus/retryIntervalMs")) {
        settings.setValue("modbus/retryIntervalMs", modbusRetryIntervalMs_);
    }
    if (!settings.contains("modbus/retryEnabled")) {
        settings.setValue("modbus/retryEnabled", modbusRetryEnabled_);
    }
}

void MainWindow::applyModbusSettingsToViews() {
    int effectiveRetries = modbusRetryEnabled_ ? modbusRetries_ : 0;
    if (modbusTcpView_) {
        modbusTcpView_->updateModbusSettings(modbusTimeoutMs_, effectiveRetries, modbusRetryIntervalMs_);
    }
    if (modbusRtuView_) {
        modbusRtuView_->updateModbusSettings(modbusTimeoutMs_, effectiveRetries, modbusRetryIntervalMs_);
    }
}

void MainWindow::openSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Modbus Settings"));
    auto layout = new QVBoxLayout(&dialog);
    auto formLayout = new QFormLayout();
    auto timeoutSpin = new QSpinBox(&dialog);
    timeoutSpin->setRange(100, 60000);
    timeoutSpin->setSingleStep(100);
    timeoutSpin->setValue(modbusTimeoutMs_);
    formLayout->addRow(tr("Request Timeout (ms):"), timeoutSpin);

    auto retryEnableCheck = new QCheckBox(&dialog);
    retryEnableCheck->setChecked(modbusRetryEnabled_);
    formLayout->addRow(tr("Enable Retry:"), retryEnableCheck);

    auto retrySpin = new QSpinBox(&dialog);
    retrySpin->setRange(0, 10);
    retrySpin->setValue(modbusRetries_);
    formLayout->addRow(tr("Retry Count:"), retrySpin);

    auto retryIntervalSpin = new QSpinBox(&dialog);
    retryIntervalSpin->setRange(10, 10000);
    retryIntervalSpin->setSingleStep(10);
    retryIntervalSpin->setValue(modbusRetryIntervalMs_);
    formLayout->addRow(tr("Retry Interval (ms):"), retryIntervalSpin);

    auto updateRetryControls = [retrySpin, retryIntervalSpin](bool enabled) {
        retrySpin->setEnabled(enabled);
        retryIntervalSpin->setEnabled(enabled);
    };
    updateRetryControls(modbusRetryEnabled_);
    connect(retryEnableCheck, &QCheckBox::toggled, &dialog, updateRetryControls);

    layout->addLayout(formLayout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        modbusTimeoutMs_ = timeoutSpin->value();
        modbusRetryEnabled_ = retryEnableCheck->isChecked();
        modbusRetries_ = retrySpin->value();
        modbusRetryIntervalMs_ = retryIntervalSpin->value();
        QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
        settings.setValue("modbus/timeoutMs", modbusTimeoutMs_);
        settings.setValue("modbus/retryCount", modbusRetries_);
        settings.setValue("modbus/retryIntervalMs", modbusRetryIntervalMs_);
        settings.setValue("modbus/retryEnabled", modbusRetryEnabled_);
        applyModbusSettingsToViews();
    }
}

void MainWindow::openAboutDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("About"));
    dialog.setMinimumWidth(420);

    auto layout = new QVBoxLayout(&dialog);
    auto aboutLabel = new QLabel(&dialog);
    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutLabel->setText(tr("Welcome to Modbus-Tools<br>"
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

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    dialog.exec();
}

void MainWindow::checkForUpdates() {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(false);
    }
    updateChecker_->checkForUpdates();
}

void MainWindow::handleUpdateAvailable(const QString& currentVersion,
                                       const QString& latestVersion,
                                       const QString& downloadUrl,
                                       const QString& releaseUrl) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(true);
    }

    const QString message = tr("Current version: v%1\nLatest version: v%2\n\nOpen download page now?")
                                .arg(currentVersion, latestVersion);
    const auto result = QMessageBox::question(this, tr("Update Available"), message, QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) {
        return;
    }

    const QString targetUrl = downloadUrl.isEmpty() ? releaseUrl : downloadUrl;
    if (!targetUrl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(targetUrl));
    }
}

void MainWindow::handleNoUpdateAvailable(const QString& currentVersion) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(true);
    }
    QMessageBox::information(this, tr("No Updates"),
                             tr("You are already using the latest version: v%1").arg(currentVersion));
}

void MainWindow::handleUpdateCheckFailed(const QString& reason) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(true);
    }
    QMessageBox::warning(this, tr("Update Check Failed"), reason);
}

void MainWindow::showDisclaimerIfNeeded() {
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    if (settings.value("app/disclaimerAccepted", false).toBool()) {
        return;
    }

    widgets::DisclaimerDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        qApp->quit();
        return;
    }
    settings.setValue("app/disclaimerAccepted", true);
}

void MainWindow::applyLanguage(const QString& locale) {
    qApp->removeTranslator(&qtTranslator_);
    qApp->removeTranslator(&appTranslator_);

    currentLocale_ = locale;
    {
        QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
        settings.setValue("app/language", currentLocale_);
    }
    if (locale == "zh_CN") {
        const bool qtLoaded = qtTranslator_.load("qtbase_zh_CN", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        const bool appLoaded = appTranslator_.load(":/i18n/Modbus-Tools_zh_CN.qm");
        if (qtLoaded) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appLoaded) {
            qApp->installTranslator(&appTranslator_);
        }
    } else if (locale == "zh_TW") {
        const bool qtLoaded = qtTranslator_.load("qtbase_zh_TW", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        const bool appLoaded = appTranslator_.load(":/i18n/Modbus-Tools_zh_TW.qm");
        if (qtLoaded) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appLoaded) {
            qApp->installTranslator(&appTranslator_);
        }
    }

    retranslateUi();
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("Modbus Tools"));
    if (navigationList_ && navigationList_->count() >= 5) {
        navigationList_->item(0)->setText(tr("Modbus TCP"));
        navigationList_->item(1)->setText(tr("Modbus RTU"));
        navigationList_->item(2)->setText(tr("TCP Client"));
        navigationList_->item(3)->setText(tr("Serial Port"));
        navigationList_->item(4)->setText(tr("Frame Analyzer"));
    }

    if (languageMenu_) {
        languageMenu_->setTitle(tr("Language"));
    }
    if (settingsMenu_) {
        settingsMenu_->setTitle(tr("Settings"));
    }
    if (aboutMenu_) {
        aboutMenu_->setTitle(tr("About"));
    }
    if (modbusSettingsAction_) {
        modbusSettingsAction_->setText(tr("Modbus Settings"));
    }
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setText(tr("Check for Updates"));
    }
    if (aboutAction_) {
        aboutAction_->setText(tr("About"));
    }
    if (langEnAction_) {
        langEnAction_->setText(tr("English (US)"));
        langEnAction_->setChecked(currentLocale_ == "en_US");
    }
    if (langZhCnAction_) {
        langZhCnAction_->setText(tr("简体中文"));
        langZhCnAction_->setChecked(currentLocale_ == "zh_CN");
    }
    if (langZhTwAction_) {
        langZhTwAction_->setText(tr("繁體中文"));
        langZhTwAction_->setChecked(currentLocale_ == "zh_TW");
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

} // namespace ui
