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
#include "application/MainWindowPresenter.h"
#include "application/UpdateCoordinator.h"
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
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QPointer>
#include <QStandardPaths>
#include <QDir>
#include <QStyle>
#include <QIcon>
#include <QColor>
#include <QPainter>
#include <QListWidgetItem>
#include <spdlog/spdlog.h>

namespace {
// Helper functions moved from MainWindow or kept for UI utility

QIcon buildNavigationIcon(const QString& resourcePath, const QColor& accentColor) {
    QPixmap sourcePixmap;
    const QIcon sourceIcon(resourcePath);
    const QList<QSize> candidateSizes = sourceIcon.availableSizes();
    if (!candidateSizes.isEmpty()) {
        QSize best = candidateSizes.first();
        for (const QSize& size : candidateSizes) {
            if (size.width() * size.height() > best.width() * best.height()) {
                best = size;
            }
        }
        sourcePixmap = sourceIcon.pixmap(best);
    } else {
        sourcePixmap.load(resourcePath);
    }
    if (sourcePixmap.isNull()) {
        sourcePixmap = sourceIcon.pixmap(256, 256);
    }
    if (sourcePixmap.isNull()) return QIcon();

    QImage image = sourcePixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixel = QColor::fromRgba(image.pixel(x, y));
            if (pixel.alpha() == 0) continue;
            if (pixel.red() > 245 && pixel.green() > 245 && pixel.blue() > 245) {
                pixel.setAlpha(0);
                image.setPixelColor(x, y, pixel);
                continue;
            }
            const int lightness = qBound(35, qGray(pixel.red(), pixel.green(), pixel.blue()), 215);
            QColor recolored = accentColor;
            recolored.setHsl(accentColor.hslHue(), qMax(accentColor.hslSaturation(), 120), lightness, pixel.alpha());
            image.setPixelColor(x, y, recolored);
        }
    }

    QIcon resultIcon;
    const QList<int> sizes = {20, 24, 32, 48};
    for (int size : sizes) {
        resultIcon.addPixmap(QPixmap::fromImage(image).scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    return resultIcon;
}

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

QString normalizedAppLocale(QString locale) {
    locale = locale.trimmed();
    if (locale == QLatin1String(app::constants::Values::App::kLocaleEn) ||
        locale == QLatin1String(app::constants::Values::App::kLocaleZhCn) ||
        locale == QLatin1String(app::constants::Values::App::kLocaleZhTw) ||
        locale == QStringLiteral("system")) {
        return locale;
    }
    return QStringLiteral("system");
}

QString effectiveAppLocale(const QString& locale) {
    if (locale != QStringLiteral("system")) return locale;
    QLocale systemLocale = QLocale::system();
    if (systemLocale.language() == QLocale::Chinese) {
        if (systemLocale.script() == QLocale::TraditionalChineseScript || 
            systemLocale.country() == QLocale::Taiwan || 
            systemLocale.country() == QLocale::HongKong || 
            systemLocale.country() == QLocale::Macau) {
            return QLatin1String(app::constants::Values::App::kLocaleZhTw);
        }
        return QLatin1String(app::constants::Values::App::kLocaleZhCn);
    }
    return QLatin1String(app::constants::Values::App::kLocaleEn);
}

QString translationFileNameForLocale(const QString& locale) {
    if (locale == QLatin1String(app::constants::Values::App::kLocaleZhCn)) {
        return QStringLiteral("Modbus-Tools_zh_CN.qm");
    }
    if (locale == QLatin1String(app::constants::Values::App::kLocaleZhTw)) {
        return QStringLiteral("Modbus-Tools_zh_TW.qm");
    }
    return {};
}

bool loadAppTranslation(QTranslator& translator, const QString& qmFileName) {
    if (qmFileName.isEmpty()) {
        return false;
    }

    const QString resourcePath = QStringLiteral(":/i18n/%1").arg(qmFileName);
    if (translator.load(resourcePath)) {
        spdlog::info("Loaded app translation from embedded resource: {}", resourcePath.toStdString());
        return true;
    }

    spdlog::warn("Failed to load app translation '{}' from embedded resource '{}'",
                 qmFileName.toStdString(),
                 resourcePath.toStdString());
    return false;
}

int calculateExpandedNavigationWidth(const QListWidget* navigationList) {
    if (!navigationList) return app::constants::Values::Ui::kNavigationExpandedWidth;
    const QFontMetrics metrics(navigationList->fontMetrics());
    int maxTextWidth = 0;
    for (int i = 0; i < navigationList->count(); ++i) {
        maxTextWidth = qMax(maxTextWidth, metrics.horizontalAdvance(navigationList->item(i)->toolTip()));
    }
    return qMax(app::constants::Values::Ui::kNavigationExpandedWidth, maxTextWidth + 60);
}
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
    updateCoordinator_ = new application::UpdateCoordinator(this, updateChecker_, updateManager_, settingsController_, this);
    presenter_ = new application::MainWindowPresenter(this, settingsController_, updateCoordinator_);
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
    
    const QByteArray geometry = settingsController_->mainWindowGeometry();
    if (!geometry.isEmpty()) restoreGeometry(geometry);

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

    const auto pageIndexByNavigationRow = pages.pageIndexByNavigationRow;
    connect(navigationList_, &QListWidget::currentRowChanged, this, [this, pageIndexByNavigationRow](int row) {
        if (!stackedWidget_ || row < 0 || row >= static_cast<int>(pageIndexByNavigationRow.size())) {
            return;
        }
        stackedWidget_->setCurrentIndex(pageIndexByNavigationRow[static_cast<std::size_t>(row)]);
    });
    navigationList_->setCurrentRow(0);

    setupLanguageMenu();
    setupSettingsMenu();
    setupAboutMenu();
    setupThemeToggle();

    const QByteArray windowState = settingsController_->mainWindowState();
    if (!windowState.isEmpty()) restoreState(windowState);

    parameterWheelBlocker_ = new ParameterWheelBlocker(this);
    qApp->installEventFilter(parameterWheelBlocker_);

    currentLocale_ = normalizedAppLocale(settingsController_->language());
    navigationCollapsed_ = settingsController_->navigationCollapsed();
    setNavigationCollapsed(navigationCollapsed_);
    
    updateThemeUi();
    core::update::UpdateManager::cleanupUpdateArtifacts();
    applyModbusSettingsToViews();
    applyLanguage(currentLocale_);
    if (updateCoordinator_) {
        updateCoordinator_->refreshIndicators();
    }
    
    if (updateCoordinator_) {
        updateCoordinator_->triggerAutoCheckIfNeeded();
    }
    showDisclaimerIfNeeded();
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
    
    // Add items with tooltips for retranslateUi to use
    auto addNavItem = [this](const QString& iconPath, const QColor& color, const QString& text) {
        const QIcon icon = buildNavigationIcon(iconPath, color);
        auto* item = new QListWidgetItem(icon.isNull() ? style()->standardIcon(QStyle::SP_FileIcon) : icon, text);
        item->setToolTip(text);
        navigationList_->addItem(item);
    };

    addNavItem(":/assets/Modbus-TCP.ico", QColor("#3B82F6"), tr("Modbus TCP"));
    addNavItem(":/assets/Modbus-RTU.ico", QColor("#8B5CF6"), tr("Modbus RTU"));
    addNavItem(":/assets/TCP-Client.ico", QColor("#06B6D4"), tr("TCP Client"));
    addNavItem(":/assets/Serial-Port.ico", QColor("#F59E0B"), tr("Serial Port"));
    addNavItem(":/assets/Frame-Analyzer.ico", QColor("#10B981"), tr("Frame Analyzer"));

    navigationExpandedWidth_ = calculateExpandedNavigationWidth(navigationList_);
    paneLayout->addWidget(navigationList_);

    common::ThemeUiHelpers::applyNavigationTheme(navigationList_->palette(), navigationPane_, navigationToggleButton_, navigationList_);
    connect(navigationToggleButton_, &QToolButton::clicked, this, [this]() {
        if (presenter_) {
            presenter_->onNavigationToggleRequested();
        }
    });
}

void MainWindow::setNavigationCollapsed(bool collapsed) {
    navigationCollapsed_ = collapsed;
    navigationPane_->setFixedWidth(navigationCollapsed_ ? navigationCollapsedWidth_ : navigationExpandedWidth_);
    for (int i = 0; i < navigationList_->count(); ++i) {
        auto* item = navigationList_->item(i);
        item->setText(navigationCollapsed_ ? QString() : item->toolTip());
        item->setTextAlignment(navigationCollapsed_ ? Qt::AlignCenter : (Qt::AlignLeft | Qt::AlignVCenter));
    }
    updateNavigationToggleUi();
}

bool MainWindow::isNavigationCollapsed() const {
    return navigationCollapsed_;
}

void MainWindow::updateNavigationToggleUi() {
    if (!navigationToggleButton_) return;
    navigationToggleButton_->setText(navigationCollapsed_ ? QStringLiteral("≫") : QStringLiteral("≪"));
    navigationToggleButton_->setToolTip(navigationCollapsed_ ? tr("Expand Navigation") : tr("Collapse Navigation"));
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

void MainWindow::applyModbusSettingsToViews() {
    int t, r, i; bool e;
    settingsController_->loadModbusSettings(t, r, i, e);
    int effectiveRetries = e ? r : 0;
    if (modbusTcpView_) modbusTcpView_->updateModbusSettings(t, effectiveRetries, i);
    if (modbusRtuView_) modbusRtuView_->updateModbusSettings(t, effectiveRetries, i);
}

void MainWindow::openModbusSettingsDialog() {
    int t, r, i; bool e;
    settingsController_->loadModbusSettings(t, r, i, e);
    widgets::ModbusSettingsDialog::Settings current{t, r, i, e};
    
    widgets::ModbusSettingsDialog dialog(current, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto s = dialog.settings();
        settingsController_->setModbusSettings(s.timeoutMs, s.retries, s.retryIntervalMs, s.retryEnabled);
        applyModbusSettingsToViews();
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

void MainWindow::persistWindowState() {
    settingsController_->setMainWindowGeometry(saveGeometry());
    settingsController_->setMainWindowState(saveState());
    settingsController_->sync();
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

void MainWindow::showDisclaimerIfNeeded() {
    if (settingsController_->disclaimerAccepted()) return;
    widgets::DisclaimerDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) settingsController_->setDisclaimerAccepted(true);
    else qApp->quit();
}

void MainWindow::applyLanguage(const QString& locale) {
    qApp->removeTranslator(&qtTranslator_);
    qApp->removeTranslator(&appTranslator_);
    currentLocale_ = locale;
    settingsController_->setLanguage(locale);
    if (updateCoordinator_) {
        updateCoordinator_->setCurrentLocale(locale);
    }
    
    const QString eff = effectiveAppLocale(locale);
    if (eff == app::constants::Values::App::kLocaleZhCn) {
        if (qtTranslator_.load("qtbase_zh_CN", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (loadAppTranslation(appTranslator_, translationFileNameForLocale(eff))) {
            qApp->installTranslator(&appTranslator_);
        }
    } else if (eff == app::constants::Values::App::kLocaleZhTw) {
        if (qtTranslator_.load("qtbase_zh_TW", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (loadAppTranslation(appTranslator_, translationFileNameForLocale(eff))) {
            qApp->installTranslator(&appTranslator_);
        }
    }
    retranslateUi();
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("Modbus Tools"));
    QStringList titles = {tr("Modbus TCP"), tr("Modbus RTU"), tr("TCP Client"), tr("Serial Port"), tr("Frame Analyzer")};
    for (int i = 0; i < titles.size() && i < navigationList_->count(); ++i) {
        navigationList_->item(i)->setToolTip(titles[i]);
        navigationList_->item(i)->setText(navigationCollapsed_ ? QString() : titles[i]);
    }
    updateNavigationToggleUi();
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
    
    const QString eff = effectiveAppLocale(currentLocale_);
    if (langEnAction_) langEnAction_->setChecked(eff == app::constants::Values::App::kLocaleEn);
    if (langZhCnAction_) langZhCnAction_->setChecked(eff == app::constants::Values::App::kLocaleZhCn);
    if (langZhTwAction_) langZhTwAction_->setChecked(eff == app::constants::Values::App::kLocaleZhTw);
    
    if (updateCoordinator_) {
        updateCoordinator_->refreshIndicators();
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
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
