#include "MainWindow.h"
#include "AppConstants.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/generic_serial/GenericSerialView.h"
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
#include <QDesktopServices>
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
#include <QScrollArea>
#include <QStandardPaths>
#include <QDateTime>
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

QWidget* createScrollablePage(QWidget* page, QWidget* parent) {
    auto* scrollArea = new QScrollArea(parent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    scrollArea->setWidget(page);
    return scrollArea;
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
    setupUi();
}

MainWindow::~MainWindow() {
    spdlog::info("MainWindow: Destructor entry");
}

void MainWindow::setupUi() {
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

    modbusTcpView_ = new views::modbus_tcp::ModbusTcpView(settingsController_->settingsService(), this);
    stackedWidget_->addWidget(createScrollablePage(modbusTcpView_, stackedWidget_));
    modbusRtuView_ = new views::modbus_rtu::ModbusRtuView(settingsController_->settingsService(), this);
    stackedWidget_->addWidget(createScrollablePage(modbusRtuView_, stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new views::generic_tcp::GenericTcpView(settingsController_->settingsService(), this), stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new views::generic_serial::GenericSerialView(settingsController_->settingsService(), this), stackedWidget_));
    
    frameAnalyzer_ = new widgets::FrameAnalyzerWidget(settingsController_->settingsService(), this);
    stackedWidget_->addWidget(createScrollablePage(frameAnalyzer_, stackedWidget_));
    
    // Connect Linkage signals
    connect(modbusTcpView_, &views::modbus_tcp::ModbusTcpView::linkageToggled, this, &MainWindow::handleTcpLinkageToggled);
    connect(modbusRtuView_, &views::modbus_rtu::ModbusRtuView::linkageToggled, this, &MainWindow::handleRtuLinkageToggled);
    connect(modbusTcpView_, &views::modbus_tcp::ModbusTcpView::linkageDataReceived, this, &MainWindow::handleLinkageData);
    connect(modbusRtuView_, &views::modbus_rtu::ModbusRtuView::linkageDataReceived, this, &MainWindow::handleLinkageData);
    
    connect(frameAnalyzer_, &widgets::FrameAnalyzerWidget::linkageStopRequested, [this]() {
        modbusTcpView_->setLinked(false);
        modbusRtuView_->setLinked(false);
    });

    connect(navigationList_, &QListWidget::currentRowChanged, stackedWidget_, &QStackedWidget::setCurrentIndex);
    navigationList_->setCurrentRow(0);

    updateChecker_ = new common::UpdateChecker(this);
    connect(updateChecker_, &common::UpdateChecker::updateAvailable, this, &MainWindow::handleUpdateAvailable);
    connect(updateChecker_, &common::UpdateChecker::noUpdateAvailable, [this](const QString& v){ 
        if(checkUpdatesAction_) checkUpdatesAction_->setEnabled(true);
        if(checkingUpdateManually_) QMessageBox::information(this, tr("No Updates"), tr("You are using the latest version: v%1").arg(v));
    });
    connect(updateChecker_, &common::UpdateChecker::checkFailed, [this](const QString& r){
        if(checkUpdatesAction_) checkUpdatesAction_->setEnabled(true);
        if(checkingUpdateManually_) QMessageBox::warning(this, tr("Update Check Failed"), r);
    });

    // UpdateManager connections
    connect(updateManager_, &core::update::UpdateManager::updateReadyToInstall, this, [this](const QString& taskFile) {
        QString error;
        if (core::update::UpdateManager::launchUpdater(taskFile, currentLocale_, error)) {
            spdlog::info("MainWindow: Updater launched successfully, terminating application to apply update.");
            qApp->quit();
        } else {
            spdlog::error("MainWindow: Failed to launch updater: {}", error.toStdString());
            QMessageBox::critical(this, tr("Update Failed"), error);
        }
    });
    connect(updateManager_, &core::update::UpdateManager::updateFailed, this, [this](const QString& err) {
        if (checkingUpdateManually_) QMessageBox::warning(this, tr("Update Failed"), err);
    });

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
    refreshUpdateIndicators();
    
    if (shouldAutoCheckUpdates()) performUpdateCheck(false);
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
        setNavigationCollapsed(!navigationCollapsed_);
        settingsController_->setNavigationCollapsed(navigationCollapsed_);
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

void MainWindow::updateNavigationToggleUi() {
    if (!navigationToggleButton_) return;
    navigationToggleButton_->setText(navigationCollapsed_ ? QStringLiteral("≫") : QStringLiteral("≪"));
    navigationToggleButton_->setToolTip(navigationCollapsed_ ? tr("Expand Navigation") : tr("Collapse Navigation"));
}

void MainWindow::setupSettingsMenu() {
    settingsMenu_ = menuBar()->addMenu(tr("Settings"));
    modbusSettingsAction_ = settingsMenu_->addAction(tr("Modbus Settings"), this, &MainWindow::openModbusSettingsDialog);
    updateSettingsAction_ = settingsMenu_->addAction(tr("Update Settings"), this, &MainWindow::openUpdateSettingsDialog);
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
        applyLanguage(action->data().toString());
    });
}

void MainWindow::setupAboutMenu() {
    aboutMenu_ = menuBar()->addMenu(tr("About"));
    checkUpdatesAction_ = aboutMenu_->addAction(tr("Check for Updates"), this, &MainWindow::checkForUpdates);
    aboutMenu_->addSeparator();
    aboutAction_ = aboutMenu_->addAction(tr("About"), this, &MainWindow::openAboutDialog);
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

void MainWindow::checkForUpdates() {
    if (updateAvailable_ && !pendingUpdateInfo_.latestVersion.isEmpty()) {
        promptUpdateAction(common::UpdateChecker::currentVersion());
        return;
    }
    if (checkUpdatesAction_) checkUpdatesAction_->setEnabled(false);
    performUpdateCheck(true);
}

void MainWindow::performUpdateCheck(bool manual) {
    checkingUpdateManually_ = manual;
    settingsController_->setLastUpdateCheckUtc(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    updateChecker_->checkForUpdates();
}

bool MainWindow::shouldAutoCheckUpdates() const {
    QString freq = settingsController_->updateCheckFrequency();
    if (freq == app::constants::Values::App::kUpdateCheckNever) return false;
    if (freq == app::constants::Values::App::kUpdateCheckStartup) return true;
    
    QDateTime lastCheck = QDateTime::fromString(settingsController_->lastUpdateCheckUtc(), Qt::ISODate);
    if (!lastCheck.isValid()) return true;
    
    qint64 days = lastCheck.daysTo(QDateTime::currentDateTimeUtc());
    if (freq == app::constants::Values::App::kUpdateCheckWeekly) return days >= 7;
    if (freq == app::constants::Values::App::kUpdateCheckMonthly) return days >= 30;
    return true;
}

void MainWindow::refreshUpdateIndicators() {
    if (aboutMenu_) aboutMenu_->setTitle(updateAvailable_ ? tr("About") + " ●" : tr("About"));
    if (checkUpdatesAction_) checkUpdatesAction_->setText(updateAvailable_ ? tr("Check for Updates") + " ●" : tr("Check for Updates"));
}

void MainWindow::handleUpdateAvailable(const QString& cur, const QString& lat, const QString& uUrl, const QString& uSha, const QString& cUrl, const QString& fUrl, const QString& rUrl) {
    if (checkUpdatesAction_) checkUpdatesAction_->setEnabled(true);
    updateAvailable_ = true;
    pendingUpdateInfo_ = {lat, uUrl, uSha, cUrl, fUrl, rUrl};
    refreshUpdateIndicators();
    if (checkingUpdateManually_) promptUpdateAction(cur);
}

void MainWindow::promptUpdateAction(const QString& currentVersion) {
    const QString downloadUrl = pendingUpdateInfo_.fullPackageUrl.isEmpty() ? pendingUpdateInfo_.releaseUrl : pendingUpdateInfo_.fullPackageUrl;
    if (pendingUpdateInfo_.updateOnlyUrl.isEmpty()) {
        if (QMessageBox::question(this, tr("Update Available"), tr("New version v%1 available. Open download page?").arg(pendingUpdateInfo_.latestVersion)) == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl(downloadUrl));
        }
        return;
    }

    QMessageBox mb(this);
    mb.setWindowTitle(tr("Update Available"));
    mb.setText(tr("Current: v%1, Latest: v%2\nChoose update method:").arg(currentVersion, pendingUpdateInfo_.latestVersion));
    auto* btnNow = mb.addButton(tr("Update Main Program"), QMessageBox::AcceptRole);
    auto* btnFull = mb.addButton(tr("Download Full Package"), QMessageBox::ActionRole);
    mb.addButton(QMessageBox::Cancel);
    mb.exec();

    if (mb.clickedButton() == btnNow) startSilentUpdate();
    else if (mb.clickedButton() == btnFull) QDesktopServices::openUrl(QUrl(downloadUrl));
}

void MainWindow::startSilentUpdate() {
    updateManager_->startUpdate(QUrl(pendingUpdateInfo_.updateOnlyUrl), 
                                pendingUpdateInfo_.updateOnlySha256, 
                                pendingUpdateInfo_.checksumsUrl, 
                                pendingUpdateInfo_.latestVersion);
    
    auto* progress = new QProgressDialog(tr("Downloading Update..."), tr("Cancel"), 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setAutoClose(true);
    connect(updateManager_, &core::update::UpdateManager::downloadProgress, progress, &QProgressDialog::setValue);
    connect(progress, &QProgressDialog::canceled, updateManager_, &core::update::UpdateManager::cancelUpdate);
    connect(updateManager_, &core::update::UpdateManager::updateReadyToInstall, progress, &QProgressDialog::close);
    connect(updateManager_, &core::update::UpdateManager::updateFailed, progress, &QProgressDialog::close);
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
    
    const QString eff = effectiveAppLocale(locale);
    if (eff == app::constants::Values::App::kLocaleZhCn) {
        if (qtTranslator_.load("qtbase_zh_CN", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appTranslator_.load(":/i18n/Modbus-Tools_zh_CN.qm")) {
            qApp->installTranslator(&appTranslator_);
        }
    } else if (eff == app::constants::Values::App::kLocaleZhTw) {
        if (qtTranslator_.load("qtbase_zh_TW", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appTranslator_.load(":/i18n/Modbus-Tools_zh_TW.qm")) {
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
    
    refreshUpdateIndicators();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    else if (event->type() == QEvent::PaletteChange) updateThemeUi();
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    settingsController_->setMainWindowGeometry(saveGeometry());
    settingsController_->setMainWindowState(saveState());
    settingsController_->sync();
    QMainWindow::closeEvent(event);
}

void MainWindow::handleTcpLinkageToggled(bool active) {
    if (active && this->modbusRtuView_) {
        this->modbusRtuView_->setLinked(false);
    }
}

void MainWindow::handleRtuLinkageToggled(bool active) {
    if (active && this->modbusTcpView_) {
        this->modbusTcpView_->setLinked(false);
    }
}

void MainWindow::handleLinkageData(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr) {
    if (this->frameAnalyzer_) {
        this->frameAnalyzer_->processLivePdu(pdu, protocol, addr);
    }
}

} // namespace ui
