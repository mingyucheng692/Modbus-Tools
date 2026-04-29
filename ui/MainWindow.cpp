/**
 * @file MainWindow.cpp
 * @brief Implementation of MainWindow.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "MainWindow.h"
#include "AppConstants.h"
#include "application/AppLifecycleCoordinator.h"
#include "application/LanguageCoordinator.h"
#include "application/MainWindowPresenter.h"
#include "application/UpdateCoordinator.h"
#include "shell/NavigationController.h"
#include "shell/MainWindowPageBuilder.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "widgets/DisclaimerDialog.h"
#include "widgets/UpdateSettingsDialog.h"
#include "widgets/ModbusSettingsDialog.h"
#include "widgets/AboutDialog.h"
#include "common/CompactMenuBarStyle.h"
#include "common/SettingsKeys.h"
#include "common/ThemeController.h"
#include "common/ThemeUiHelpers.h"
#include "common/UpdateChecker.h"
#include "common/ISettingsService.h"
#include "../core/update/UpdateManager.h"
#include "../core/common/SettingsController.h"
#include <QStackedWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QToolButton>
#include <QApplication>
#include <QEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QPointer>
#include <QStandardPaths>
#include <QDir>
#include <QStyle>
#include <vector>
#include <spdlog/spdlog.h>

namespace {
// Helper functions moved from MainWindow or kept for UI utility

class ParameterWheelBlocker : public QObject {
public:
    explicit ParameterWheelBlocker(QObject* parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::Wheel) {
            if (watched->inherits("QAbstractSpinBox") || watched->inherits("QComboBox")) {
                event->ignore();
                return true;
            }
        }
        return QObject::eventFilter(watched, event);
    }
};
}

namespace ui {

MainWindow::MainWindow(common::ISettingsService* settingsService,
                       common::ThemeController* themeController,
                       QWidget *parent)
    : QMainWindow(parent),
      themeController_(themeController),
      settingsController_(new core::common::SettingsController(settingsService, this)),
      updateManager_(new core::update::UpdateManager(this)) {
    updateChecker_ = new common::UpdateChecker(this);
    languageCoordinator_ = std::make_unique<application::LanguageCoordinator>(settingsController_);
    updateCoordinator_ = new application::UpdateCoordinator(this, updateChecker_, updateManager_, settingsController_, this);
    appLifecycleCoordinator_ = std::make_unique<application::AppLifecycleCoordinator>(this, settingsController_, updateCoordinator_);
    presenter_ = std::make_unique<application::MainWindowPresenter>(
        this,
        settingsController_,
        appLifecycleCoordinator_.get(),
        languageCoordinator_.get(),
        updateCoordinator_);
    presenter_->initialize();
}

MainWindow::~MainWindow() {
    spdlog::info("MainWindow: Destructor entry");
}

void MainWindow::initializeUi() {
    setWindowTitle(tr("Modbus Tools"));
    setMinimumSize(app::constants::Values::Ui::kMainWindowMinWidth, 
                   app::constants::Values::Ui::kMainWindowMinHeight);
    menuBar()->setStyle(new common::CompactMenuBarStyle(menuBar()->style()->name()));
    resize(app::constants::Values::Ui::kMainWindowDefaultWidth,
           app::constants::Values::Ui::kMainWindowDefaultHeight);

    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    createNavigation();
    mainLayout->addWidget(navigationPane_);

    stackedWidget_ = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget_, 5);

    MainWindowPageBuilder pageBuilder(settingsController_->settingsService());
    const MainWindowPages pages = pageBuilder.build(stackedWidget_, this);
    modbusTcpView_ = pages.modbusTcpView;
    modbusRtuView_ = pages.modbusRtuView;
    frameAnalyzer_ = pages.frameAnalyzer;
    analyzerLinkageController_ = new AnalyzerLinkageController(this);
    analyzerLinkageController_->bind(modbusTcpView_, modbusRtuView_, frameAnalyzer_);
    
    // Connect Linkage signals
    connect(modbusTcpView_, &views::modbus_tcp::ModbusTcpView::linkageToggled, this, &MainWindow::handleTcpLinkageToggled);
    connect(modbusRtuView_, &views::modbus_rtu::ModbusRtuView::linkageToggled, this, &MainWindow::handleRtuLinkageToggled);
    connect(modbusTcpView_, &views::modbus_tcp::ModbusTcpView::linkageDataReceived, this, &MainWindow::handleLinkageData);
    connect(modbusRtuView_, &views::modbus_rtu::ModbusRtuView::linkageDataReceived, this, &MainWindow::handleLinkageData);
    connect(modbusTcpView_, &views::modbus_tcp::ModbusTcpView::linkageSourceDisconnected, this, [this]() {
        if (analyzerLinkageController_) {
            analyzerLinkageController_->handleSourceDisconnected(AnalyzerLinkageController::LinkSource::Tcp);
        }
    });
    connect(modbusRtuView_, &views::modbus_rtu::ModbusRtuView::linkageSourceDisconnected, this, [this]() {
        if (analyzerLinkageController_) {
            analyzerLinkageController_->handleSourceDisconnected(AnalyzerLinkageController::LinkSource::Rtu);
        }
    });
    connect(frameAnalyzer_, &widgets::FrameAnalyzerWidget::linkagePauseToggled, this, &MainWindow::handleLinkagePauseToggled);
    connect(frameAnalyzer_, &widgets::FrameAnalyzerWidget::linkageStopRequested, this, &MainWindow::handleLinkageStopRequested);

    if (navigationController_) {
        navigationController_->bindToStack(stackedWidget_,
                                           std::vector<int>(pages.pageIndexByNavigationRow.begin(),
                                                            pages.pageIndexByNavigationRow.end()));
    }

    setupLanguageMenu();
    setupSettingsMenu();
    setupAboutMenu();
    setupThemeToggle();

    parameterWheelBlocker_ = new ParameterWheelBlocker(this);
    qApp->installEventFilter(parameterWheelBlocker_);
    
    updateThemeUi();
}

void MainWindow::createNavigation() {
    navigationPane_ = new QWidget(this);
    auto paneLayout = new QVBoxLayout(navigationPane_);
    paneLayout->setContentsMargins(0, 0, 0, 0);
    paneLayout->setSpacing(0);

    navigationToggleButton_ = new QToolButton(navigationPane_);
    navigationToggleButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    navigationToggleButton_->setFixedHeight(28);
    paneLayout->addWidget(navigationToggleButton_);

    navigationList_ = new QListWidget(navigationPane_);
    navigationList_->setIconSize(QSize(24, 24));
    paneLayout->addWidget(navigationList_);

    navigationController_ = std::make_unique<shell::NavigationController>(navigationList_, navigationPane_, navigationToggleButton_);
    navigationController_->initialize(
        {tr("Modbus TCP"), tr("Modbus RTU"), tr("TCP Client"), tr("Serial Port"), tr("Frame Analyzer")});

    common::ThemeUiHelpers::applyNavigationTheme(navigationList_->palette(), navigationPane_, navigationToggleButton_, navigationList_);
    connect(navigationToggleButton_, &QToolButton::clicked, this, [this]() {
        if (presenter_) {
            presenter_->onNavigationToggleRequested();
        }
    });
}

void MainWindow::setNavigationCollapsed(bool collapsed) {
    if (navigationController_) {
        navigationController_->setCollapsed(collapsed);
    }
}

bool MainWindow::isNavigationCollapsed() const {
    return navigationController_ && navigationController_->isCollapsed();
}

void MainWindow::restoreWindowGeometry(const QByteArray& geometry) {
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void MainWindow::restoreWindowState(const QByteArray& state) {
    if (!state.isEmpty()) {
        restoreState(state);
    }
}

QByteArray MainWindow::saveWindowGeometry() const {
    return saveGeometry();
}

QByteArray MainWindow::saveWindowState() const {
    return saveState();
}

void MainWindow::applyModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    applyModbusSettingsToViews(timeoutMs, retries, retryIntervalMs);
}

void MainWindow::setupSettingsMenu() {
    settingsMenu_ = menuBar()->addMenu(tr("Settings"));
    modbusSettingsAction_ = settingsMenu_->addAction(tr("Modbus Settings"), this, [this]() {
        if (presenter_) {
            presenter_->onModbusSettingsRequested();
        }
    });
    updateSettingsAction_ = settingsMenu_->addAction(tr("Update Settings"), this, [this]() {
        if (presenter_) {
            presenter_->onUpdateSettingsRequested();
        }
    });
}

void MainWindow::setupLanguageMenu() {
    languageMenu_ = menuBar()->addMenu(tr("Language"));
    languageActionGroup_ = new QActionGroup(this);
    languageActionGroup_->setExclusive(true);

    auto addLang = [this](const QString& text, const QString& code) {
        auto* action = languageMenu_->addAction(text);
        action->setCheckable(true);
        action->setData(code);
        languageActionGroup_->addAction(action);
        if (code == QLatin1String(app::constants::Values::App::kLocaleEn)) langEnAction_ = action;
        else if (code == QLatin1String(app::constants::Values::App::kLocaleZhCn)) langZhCnAction_ = action;
        else if (code == QLatin1String(app::constants::Values::App::kLocaleZhTw)) langZhTwAction_ = action;
    };

    addLang(tr("English (US)"), app::constants::Values::App::kLocaleEn);
    addLang(tr("简体中文"), app::constants::Values::App::kLocaleZhCn);
    addLang(tr("繁體中文"), app::constants::Values::App::kLocaleZhTw);

    connect(languageActionGroup_, &QActionGroup::triggered, this, [this](QAction* action) {
        if (presenter_) {
            presenter_->onLanguageSelected(action->data().toString());
        }
    });
}

void MainWindow::setupAboutMenu() {
    aboutMenu_ = menuBar()->addMenu(tr("About"));
    checkUpdatesAction_ = aboutMenu_->addAction(tr("Check for Updates"), this, [this]() {
        if (presenter_) {
            presenter_->onCheckForUpdatesRequested();
        }
    });
    aboutMenu_->addSeparator();
    aboutAction_ = aboutMenu_->addAction(tr("About"), this, [this]() {
        if (presenter_) {
            presenter_->onAboutRequested();
        }
    });
}

void MainWindow::setupThemeToggle() {
    themeToggleButton_ = new QToolButton(menuBar());
    themeToggleButton_->setAutoRaise(true);
    themeToggleButton_->setFixedSize(28, 28);
    menuBar()->setCornerWidget(themeToggleButton_, Qt::TopRightCorner);
    connect(themeToggleButton_, &QToolButton::clicked, themeController_, &common::ThemeController::cycleMode);
    connect(themeController_, &common::ThemeController::themeChanged, this, &MainWindow::updateThemeUi);
}

void MainWindow::updateThemeUi() {
    common::ThemeUiHelpers::applyNavigationTheme(palette(), navigationPane_, navigationToggleButton_, navigationList_);
    updateThemeToggleUi();
}

void MainWindow::updateThemeToggleUi() {
    if (!themeToggleButton_) return;
    common::ThemeUiHelpers::applyThemeToggleAppearance(themeToggleButton_->palette(), menuBar(), style(), themeToggleButton_, themeController_->currentMode());
    QString tooltip = (themeController_->currentMode() == common::Theme::Mode::Auto) ? tr("Theme: Auto") : 
                      (themeController_->currentMode() == common::Theme::Mode::Light) ? tr("Theme: Light") : tr("Theme: Dark");
    themeToggleButton_->setToolTip(tooltip);
}

void MainWindow::applyModbusSettingsToViews(int timeoutMs, int retries, int retryIntervalMs) {
    if (modbusTcpView_) modbusTcpView_->updateModbusSettings(timeoutMs, retries, retryIntervalMs);
    if (modbusRtuView_) modbusRtuView_->updateModbusSettings(timeoutMs, retries, retryIntervalMs);
}

void MainWindow::openModbusSettingsDialog() {
    int t, r, i; bool e;
    settingsController_->loadModbusSettings(t, r, i, e);
    widgets::ModbusSettingsDialog::Settings current{t, r, i, e};
    
    widgets::ModbusSettingsDialog dialog(current, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto s = dialog.settings();
        settingsController_->setModbusSettings(s.timeoutMs, s.retries, s.retryIntervalMs, s.retryEnabled);
        applyModbusSettingsToViews(s.timeoutMs, s.retryEnabled ? s.retries : 0, s.retryIntervalMs);
    }
}

void MainWindow::openUpdateSettingsDialog() {
    widgets::UpdateSettingsDialog dialog(settingsController_->updateCheckFrequency(), this);
    if (dialog.exec() == QDialog::Accepted) {
        settingsController_->setUpdateCheckFrequency(dialog.selectedFrequency());
    }
}

void MainWindow::openAboutDialog() {
    widgets::AboutDialog dialog(this);
    dialog.exec();
}

bool MainWindow::showDisclaimerDialog() {
    widgets::DisclaimerDialog dialog(this);
    return dialog.exec() == QDialog::Accepted;
}

void MainWindow::checkForUpdates() {
    if (presenter_) {
        presenter_->onCheckForUpdatesRequested();
    }
}

void MainWindow::setUpdateCheckActionEnabled(bool enabled) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(enabled);
    }
}

void MainWindow::setUpdateIndicatorVisible(bool visible) {
    if (aboutMenu_) {
        aboutMenu_->setTitle(visible ? tr("About") + " ●" : tr("About"));
    }
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setText(visible ? tr("Check for Updates") + " ●" : tr("Check for Updates"));
    }
}

void MainWindow::showUpdateInfoMessage(const QString& title, const QString& message) {
    QMessageBox::information(this, title, message);
}

void MainWindow::showUpdateWarningMessage(const QString& title, const QString& message) {
    QMessageBox::warning(this, title, message);
}

void MainWindow::showUpdateCriticalMessage(const QString& title, const QString& message) {
    QMessageBox::critical(this, title, message);
}

bool MainWindow::confirmOpenDownloadPage(const QString& latestVersion) {
    return QMessageBox::question(
               this,
               tr("Update Available"),
               tr("New version v%1 available. Open download page?").arg(latestVersion))
           == QMessageBox::Yes;
}

application::UpdatePromptChoice MainWindow::promptUpdateAction(const QString& currentVersion, const QString& latestVersion) {
    QMessageBox mb(this);
    mb.setWindowTitle(tr("Update Available"));
    mb.setText(tr("Current: v%1, Latest: v%2\nChoose update method:").arg(currentVersion, latestVersion));
    auto* btnNow = mb.addButton(tr("Update Main Program"), QMessageBox::AcceptRole);
    auto* btnFull = mb.addButton(tr("Download Full Package"), QMessageBox::ActionRole);
    auto* btnCancel = mb.addButton(QMessageBox::Cancel);
    mb.exec();

    if (mb.clickedButton() == btnNow) {
        return application::UpdatePromptChoice::StartSilentUpdate;
    }
    if (mb.clickedButton() == btnFull) {
        return application::UpdatePromptChoice::DownloadFullPackage;
    }
    Q_UNUSED(btnCancel);
    return application::UpdatePromptChoice::Cancel;
}

void MainWindow::showUpdateProgress(core::update::UpdateManager* updateManager) {
    auto* progress = new QProgressDialog(tr("Downloading Update..."), tr("Cancel"), 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setAutoClose(true);
    connect(updateManager, &core::update::UpdateManager::downloadProgress, progress, &QProgressDialog::setValue);
    connect(progress, &QProgressDialog::canceled, updateManager, &core::update::UpdateManager::cancelUpdate);
    connect(updateManager, &core::update::UpdateManager::updateReadyToInstall, progress, &QProgressDialog::close);
    connect(updateManager, &core::update::UpdateManager::updateFailed, progress, &QProgressDialog::close);
    connect(updateManager, &core::update::UpdateManager::updateCanceled, progress, &QProgressDialog::close);
    progress->show();
}

void MainWindow::retranslateUi(const QString& effectiveLocale) {
    effectiveLocale_ = effectiveLocale;
    setWindowTitle(tr("Modbus Tools"));
    const QStringList titles = {tr("Modbus TCP"), tr("Modbus RTU"), tr("TCP Client"), tr("Serial Port"), tr("Frame Analyzer")};
    if (navigationController_) {
        navigationController_->retranslate(titles, tr("Expand Navigation"), tr("Collapse Navigation"));
    }
    if (languageMenu_) languageMenu_->setTitle(tr("Language"));
    if (settingsMenu_) settingsMenu_->setTitle(tr("Settings"));
    if (aboutMenu_) aboutMenu_->setTitle(tr("About"));
    if (modbusSettingsAction_) modbusSettingsAction_->setText(tr("Modbus Settings"));
    if (updateSettingsAction_) updateSettingsAction_->setText(tr("Update Settings"));
    if (checkUpdatesAction_) checkUpdatesAction_->setText(tr("Check for Updates"));
    if (aboutAction_) aboutAction_->setText(tr("About"));
    
    if (langEnAction_) langEnAction_->setText(tr("English (US)"));
    if (langZhCnAction_) langZhCnAction_->setText(tr("简体中文"));
    if (langZhTwAction_) langZhTwAction_->setText(tr("繁體中文"));
    
    if (langEnAction_) langEnAction_->setChecked(effectiveLocale_ == app::constants::Values::App::kLocaleEn);
    if (langZhCnAction_) langZhCnAction_->setChecked(effectiveLocale_ == app::constants::Values::App::kLocaleZhCn);
    if (langZhTwAction_) langZhTwAction_->setChecked(effectiveLocale_ == app::constants::Values::App::kLocaleZhTw);
    
    if (updateCoordinator_) {
        updateCoordinator_->refreshIndicators();
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi(effectiveLocale_);
    else if (event->type() == QEvent::PaletteChange) updateThemeUi();
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (presenter_) {
        presenter_->onCloseRequested();
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::handleTcpLinkageToggled(bool active) {
    if (analyzerLinkageController_) {
        analyzerLinkageController_->requestLinkToggle(AnalyzerLinkageController::LinkSource::Tcp, active);
    }
}

void MainWindow::handleRtuLinkageToggled(bool active) {
    if (analyzerLinkageController_) {
        analyzerLinkageController_->requestLinkToggle(AnalyzerLinkageController::LinkSource::Rtu, active);
    }
}

void MainWindow::handleLinkageStopRequested() {
    if (analyzerLinkageController_) {
        analyzerLinkageController_->requestStop();
    }
}

void MainWindow::handleLinkagePauseToggled(bool paused) {
    if (analyzerLinkageController_) {
        analyzerLinkageController_->requestPause(paused);
    }
}

void MainWindow::handleLinkageData(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr) {
    if (analyzerLinkageController_) {
        analyzerLinkageController_->handleLiveData(pdu, protocol, addr);
    }
}

} // namespace ui
