#include "MainWindow.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/generic_serial/GenericSerialView.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "widgets/DisclaimerDialog.h"
#include "widgets/UpdateSettingsDialog.h"
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
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSpinBox>
#include <QComboBox>
#include <QAbstractSpinBox>
#include <QLabel>
#include <QRegularExpression>
#include <QToolButton>
#include <QListWidgetItem>
#include <QStyle>
#include <QImage>
#include <QPainter>
#include <QIcon>
#include <QColor>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <spdlog/spdlog.h>
#include "modbus/base/ModbusConfig.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
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
    if (sourcePixmap.isNull()) {
        return QIcon();
    }

    QImage image = sourcePixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    int minX = image.width();
    int minY = image.height();
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixel = QColor::fromRgba(image.pixel(x, y));
            if (pixel.alpha() == 0) {
                continue;
            }
            if (pixel.red() > 245 && pixel.green() > 245 && pixel.blue() > 245) {
                pixel.setAlpha(0);
                image.setPixelColor(x, y, pixel);
                continue;
            }

            const int lightness = qBound(35, qGray(pixel.red(), pixel.green(), pixel.blue()), 215);
            QColor recolored = accentColor;
            recolored.setHsl(accentColor.hslHue(), qMax(accentColor.hslSaturation(), 120), lightness, pixel.alpha());
            image.setPixelColor(x, y, recolored);

            minX = qMin(minX, x);
            minY = qMin(minY, y);
            maxX = qMax(maxX, x);
            maxY = qMax(maxY, y);
        }
    }

    if (maxX < minX || maxY < minY) {
        return QIcon(QPixmap::fromImage(image));
    }

    const QRect contentRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
    QImage cropped = image.copy(contentRect);
    QIcon resultIcon;
    const QList<int> sizes = {20, 22, 24, 28, 32, 40, 48};
    for (int size : sizes) {
        QImage canvas(size, size, QImage::Format_ARGB32_Premultiplied);
        canvas.fill(Qt::transparent);
        QPainter painter(&canvas);
        QImage scaled = cropped.scaled(size - 1, size - 1, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPoint topLeft((canvas.width() - scaled.width()) / 2, (canvas.height() - scaled.height()) / 2);
        painter.drawImage(topLeft, scaled);
        painter.end();
        resultIcon.addPixmap(QPixmap::fromImage(canvas));

        const int highSize = size * 2;
        QImage canvas2x(highSize, highSize, QImage::Format_ARGB32_Premultiplied);
        canvas2x.fill(Qt::transparent);
        QPainter painter2x(&canvas2x);
        QImage scaled2x = cropped.scaled(highSize - 2, highSize - 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPoint topLeft2x((canvas2x.width() - scaled2x.width()) / 2, (canvas2x.height() - scaled2x.height()) / 2);
        painter2x.drawImage(topLeft2x, scaled2x);
        painter2x.end();
        QPixmap hiPixmap = QPixmap::fromImage(canvas2x);
        hiPixmap.setDevicePixelRatio(2.0);
        resultIcon.addPixmap(hiPixmap);

        const int ultraSize = size * 3;
        QImage canvas3x(ultraSize, ultraSize, QImage::Format_ARGB32_Premultiplied);
        canvas3x.fill(Qt::transparent);
        QPainter painter3x(&canvas3x);
        QImage scaled3x = cropped.scaled(ultraSize - 3, ultraSize - 3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPoint topLeft3x((canvas3x.width() - scaled3x.width()) / 2, (canvas3x.height() - scaled3x.height()) / 2);
        painter3x.drawImage(topLeft3x, scaled3x);
        painter3x.end();
        QPixmap ultraPixmap = QPixmap::fromImage(canvas3x);
        ultraPixmap.setDevicePixelRatio(3.0);
        resultIcon.addPixmap(ultraPixmap);
    }

    return resultIcon;
}

QWidget* createScrollablePage(QWidget* page, QWidget* parent) {
    auto* scrollArea = new QScrollArea(parent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    scrollArea->setWidget(page);
    return scrollArea;
}

class ParameterWheelBlocker : public QObject {
public:
    explicit ParameterWheelBlocker(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() != QEvent::Wheel) {
            return QObject::eventFilter(watched, event);
        }
        if (qobject_cast<QAbstractSpinBox*>(watched) || qobject_cast<QComboBox*>(watched)) {
            event->ignore();
            return true;
        }
        return QObject::eventFilter(watched, event);
    }
};
}

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
    setMinimumSize(720, 480);
    resize(1280, 800);

    // Central widget with HBox layout
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Navigation
    createNavigation();
    mainLayout->addWidget(navigationPane_);

    // Content Area
    stackedWidget_ = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget_, 5); // Ratio 1:5

    // Add Views
    modbusTcpView_ = new views::modbus_tcp::ModbusTcpView(this);
    stackedWidget_->addWidget(createScrollablePage(modbusTcpView_, stackedWidget_));
    modbusRtuView_ = new views::modbus_rtu::ModbusRtuView(this);
    stackedWidget_->addWidget(createScrollablePage(modbusRtuView_, stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new views::generic_tcp::GenericTcpView(this), stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new views::generic_serial::GenericSerialView(this), stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new widgets::FrameAnalyzerWidget(this), stackedWidget_));

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
    parameterWheelBlocker_ = new ParameterWheelBlocker(this);
    qApp->installEventFilter(parameterWheelBlocker_);
    {
        QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
        currentLocale_ = settings.value("app/language", currentLocale_).toString();
        navigationCollapsed_ = settings.value("app/navigationCollapsed", false).toBool();
        if (!settings.contains("app/language")) {
            settings.setValue("app/language", currentLocale_);
        }
        if (!settings.contains("app/navigationCollapsed")) {
            settings.setValue("app/navigationCollapsed", navigationCollapsed_);
        }
    }
    setNavigationCollapsed(navigationCollapsed_);
    cleanupUpdateArtifacts();
    loadModbusSettings();
    loadUpdateSettings();
    applyModbusSettingsToViews();
    applyLanguage(currentLocale_);
    refreshUpdateIndicators();
    if (shouldAutoCheckUpdates()) {
        performUpdateCheck(false);
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
    const QIcon modbusTcpIcon = buildNavigationIcon(":/assets/Modbus-TCP.ico", QColor("#3B82F6"));
    const QIcon modbusRtuIcon = buildNavigationIcon(":/assets/Modbus-RTU.ico", QColor("#8B5CF6"));
    const QIcon tcpClientIcon = buildNavigationIcon(":/assets/TCP-Client.ico", QColor("#06B6D4"));
    const QIcon serialPortIcon = buildNavigationIcon(":/assets/Serial-Port.ico", QColor("#F59E0B"));
    const QIcon frameAnalyzerIcon = buildNavigationIcon(":/assets/Frame-Analyzer.ico", QColor("#10B981"));

    navigationList_->addItem(new QListWidgetItem(modbusTcpIcon.isNull() ? style()->standardIcon(QStyle::SP_ComputerIcon) : modbusTcpIcon, tr("Modbus TCP")));
    navigationList_->addItem(new QListWidgetItem(modbusRtuIcon.isNull() ? style()->standardIcon(QStyle::SP_DriveNetIcon) : modbusRtuIcon, tr("Modbus RTU")));
    navigationList_->addItem(new QListWidgetItem(tcpClientIcon.isNull() ? style()->standardIcon(QStyle::SP_DialogOpenButton) : tcpClientIcon, tr("TCP Client")));
    navigationList_->addItem(new QListWidgetItem(serialPortIcon.isNull() ? style()->standardIcon(QStyle::SP_DriveHDIcon) : serialPortIcon, tr("Serial Port")));
    navigationList_->addItem(new QListWidgetItem(frameAnalyzerIcon.isNull() ? style()->standardIcon(QStyle::SP_FileDialogDetailedView) : frameAnalyzerIcon, tr("Frame Analyzer")));
    navigationList_->setIconSize(QSize(24, 24));
    navigationList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    paneLayout->addWidget(navigationList_);

    navigationList_->setStyleSheet(common::Theme::getNavigationStyle(false));
    connect(navigationToggleButton_, &QToolButton::clicked, this, [this]() {
        setNavigationCollapsed(!navigationCollapsed_);
        QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
        settings.setValue("app/navigationCollapsed", navigationCollapsed_);
    });
}

void MainWindow::setNavigationCollapsed(bool collapsed) {
    navigationCollapsed_ = collapsed;
    if (navigationPane_) {
        navigationPane_->setFixedWidth(navigationCollapsed_ ? navigationCollapsedWidth_ : navigationExpandedWidth_);
    }
    if (navigationList_) {
        for (int i = 0; i < navigationList_->count(); ++i) {
            auto* item = navigationList_->item(i);
            if (!item) {
                continue;
            }
            const QString title = item->toolTip().isEmpty() ? item->text() : item->toolTip();
            item->setToolTip(title);
            item->setText(navigationCollapsed_ ? QString() : title);
            item->setTextAlignment(navigationCollapsed_ ? Qt::AlignCenter : (Qt::AlignLeft | Qt::AlignVCenter));
            item->setSizeHint(navigationCollapsed_ ? QSize(48, 44) : QSize());
        }
    }
    updateNavigationToggleUi();
}

void MainWindow::updateNavigationToggleUi() {
    if (!navigationToggleButton_) {
        return;
    }
    navigationToggleButton_->setText(navigationCollapsed_ ? QStringLiteral("≫") : QStringLiteral("≪"));
    navigationToggleButton_->setToolTip(navigationCollapsed_ ? tr("Expand Navigation") : tr("Collapse Navigation"));
}

void MainWindow::setupSettingsMenu() {
    settingsMenu_ = menuBar()->addMenu(tr("Settings"));
    modbusSettingsAction_ = settingsMenu_->addAction(tr("Modbus Settings"));
    connect(modbusSettingsAction_, &QAction::triggered, this, &MainWindow::openModbusSettingsDialog);
    updateSettingsAction_ = settingsMenu_->addAction(tr("Update Settings"));
    connect(updateSettingsAction_, &QAction::triggered, this, &MainWindow::openUpdateSettingsDialog);
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
    refreshUpdateIndicators();
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

void MainWindow::loadUpdateSettings() {
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    updateCheckFrequency_ = settings.value("app/updateCheckFrequency", "startup").toString();
    if (updateCheckFrequency_.isEmpty()) {
        updateCheckFrequency_ = "startup";
    }
    if (!settings.contains("app/updateCheckFrequency")) {
        settings.setValue("app/updateCheckFrequency", updateCheckFrequency_);
    }
}

void MainWindow::openModbusSettingsDialog() {
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

void MainWindow::openUpdateSettingsDialog() {
    widgets::UpdateSettingsDialog dialog(updateCheckFrequency_, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    updateCheckFrequency_ = dialog.selectedFrequency();
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue("app/updateCheckFrequency", updateCheckFrequency_);
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
    if (updateAvailable_ && !pendingLatestVersion_.isEmpty()) {
        promptUpdateAction(common::UpdateChecker::currentVersion());
        return;
    }
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(false);
    }
    performUpdateCheck(true);
}

void MainWindow::performUpdateCheck(bool manual) {
    checkingUpdateManually_ = manual;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue("app/updateLastCheckUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    updateChecker_->checkForUpdates();
}

bool MainWindow::shouldAutoCheckUpdates() const {
    if (updateCheckFrequency_ == "off") {
        return false;
    }
    if (updateCheckFrequency_ == "startup") {
        return true;
    }
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    const QString lastCheckUtc = settings.value("app/updateLastCheckUtc").toString();
    if (lastCheckUtc.isEmpty()) {
        return true;
    }
    const QDateTime lastCheck = QDateTime::fromString(lastCheckUtc, Qt::ISODate);
    if (!lastCheck.isValid()) {
        return true;
    }
    const qint64 elapsedDays = lastCheck.daysTo(QDateTime::currentDateTimeUtc());
    if (updateCheckFrequency_ == "weekly") {
        return elapsedDays >= 7;
    }
    if (updateCheckFrequency_ == "monthly") {
        return elapsedDays >= 30;
    }
    return true;
}

void MainWindow::refreshUpdateIndicators() {
    const QString aboutText = tr("About");
    const QString checkText = tr("Check for Updates");
    if (aboutMenu_) {
        aboutMenu_->setTitle(updateAvailable_ ? aboutText + QStringLiteral(" ●") : aboutText);
    }
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setText(updateAvailable_ ? checkText + QStringLiteral(" ●") : checkText);
    }
}

void MainWindow::cleanupUpdateArtifacts() {
    const QDir appDir(QApplication::applicationDirPath());
    QFile::remove(appDir.filePath("modbus-tools.exe.bak"));
    QFile::remove(appDir.filePath("updater.old.exe"));
    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempRoot.isEmpty()) {
        return;
    }
    QDir(QStringLiteral("%1/ModbusToolsUpdate").arg(tempRoot)).removeRecursively();
}

bool MainWindow::downloadUpdateAsset(const QUrl& url, const QString& filePath, QString& errorMessage) const {
    if (!url.isValid()) {
        errorMessage = tr("Invalid update URL");
        return false;
    }

    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(".")) {
        errorMessage = tr("Failed to create update directory");
        return false;
    }

    QFile outputFile(filePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorMessage = tr("Failed to write update file");
        return false;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Modbus-Tools/%1").arg(common::UpdateChecker::currentVersion()));
    QNetworkReply* reply = manager.get(request);

    const QString assetName = QFileInfo(filePath).fileName();
    QProgressDialog progressDialog(tr("Downloading %1").arg(assetName),
                                   tr("Cancel"),
                                   0,
                                   100,
                                   const_cast<MainWindow*>(this));
    progressDialog.setWindowTitle(tr("Downloading Update"));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.setValue(0);

    connect(&progressDialog, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::readyRead, [&outputFile, reply]() {
        outputFile.write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, &progressDialog, [&progressDialog](qint64 received, qint64 total) {
        if (total > 0) {
            const int progress = static_cast<int>((received * 100) / total);
            progressDialog.setValue(qBound(0, progress, 100));
        } else {
            progressDialog.setValue(0);
        }
    });

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    progressDialog.setValue(100);
    outputFile.write(reply->readAll());
    outputFile.close();

    if (reply->error() != QNetworkReply::NoError) {
        errorMessage = reply->error() == QNetworkReply::OperationCanceledError
                           ? tr("Download canceled")
                           : reply->errorString();
        reply->deleteLater();
        QFile::remove(filePath);
        return false;
    }

    reply->deleteLater();
    if (QFileInfo(filePath).size() <= 0) {
        errorMessage = tr("Downloaded file is empty");
        QFile::remove(filePath);
        return false;
    }
    return true;
}

QString MainWindow::calculateFileSha256(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return {};
    }
    return hash.result().toHex();
}

QString MainWindow::resolveSha256FromChecksums(const QString& checksumsPath, const QString& targetFileName) const {
    QFile checksumsFile(checksumsPath);
    if (!checksumsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    const QStringList lines = QString::fromUtf8(checksumsFile.readAll()).split('\n');
    const QRegularExpression pattern(QStringLiteral("^\\s*([a-fA-F0-9]{64})\\s+\\*?(.+?)\\s*$"));
    for (const QString& line : lines) {
        const QRegularExpressionMatch match = pattern.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        const QString fileName = QFileInfo(match.captured(2).trimmed()).fileName();
        if (fileName.compare(targetFileName, Qt::CaseInsensitive) == 0) {
            return match.captured(1).toLower();
        }
    }
    return {};
}

bool MainWindow::writeUpdateTaskFile(const QString& taskFilePath,
                                     const QString& newExePath,
                                     const QString& expectedSha256,
                                     QString& errorMessage) const {
    const QString targetExePath = QCoreApplication::applicationFilePath();
    const QString backupExePath = targetExePath + ".bak";
    QJsonObject root;
    root.insert("schemaVersion", 1);
    root.insert("launcherPid", static_cast<qint64>(QCoreApplication::applicationPid()));
    root.insert("targetExePath", QDir::toNativeSeparators(targetExePath));
    root.insert("newExePath", QDir::toNativeSeparators(newExePath));
    root.insert("backupExePath", QDir::toNativeSeparators(backupExePath));
    root.insert("expectedVersion", pendingLatestVersion_);
    root.insert("expectedSha256", expectedSha256);
    root.insert("restartAfterUpdate", true);

    QFile taskFile(taskFilePath);
    if (!taskFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorMessage = tr("Failed to create update task file");
        return false;
    }

    const QByteArray jsonBytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (taskFile.write(jsonBytes) != jsonBytes.size()) {
        errorMessage = tr("Failed to write update task file");
        return false;
    }
    return true;
}

bool MainWindow::launchUpdater(const QString& taskFilePath, QString& errorMessage) const {
    const QString updaterPath = QDir(QApplication::applicationDirPath()).filePath("updater.exe");
    if (!QFileInfo::exists(updaterPath)) {
        errorMessage = tr("Updater not found");
        return false;
    }
#ifdef Q_OS_WIN
    const QString parameters = QStringLiteral("--task \"%1\"").arg(QDir::toNativeSeparators(taskFilePath));
    SHELLEXECUTEINFOW shellExecInfo{};
    shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellExecInfo.hwnd = nullptr;
    shellExecInfo.lpVerb = L"runas";
    const std::wstring updaterPathW = QDir::toNativeSeparators(updaterPath).toStdWString();
    const std::wstring parametersW = parameters.toStdWString();
    const std::wstring workingDirW = QDir::toNativeSeparators(QApplication::applicationDirPath()).toStdWString();
    shellExecInfo.lpFile = updaterPathW.c_str();
    shellExecInfo.lpParameters = parametersW.c_str();
    shellExecInfo.lpDirectory = workingDirW.c_str();
    shellExecInfo.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&shellExecInfo)) {
        errorMessage = tr("Failed to launch updater");
        return false;
    }
    if (shellExecInfo.hProcess) {
        CloseHandle(shellExecInfo.hProcess);
    }
    return true;
#else
    Q_UNUSED(taskFilePath);
    Q_UNUSED(updaterPath);
    errorMessage = tr("Failed to launch updater");
    return false;
#endif
}

bool MainWindow::tryStartSilentUpdate() {
    if (pendingUpdateOnlyUrl_.isEmpty()) {
        return false;
    }

    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempRoot.isEmpty()) {
        return false;
    }
    const QDir workingDir(QStringLiteral("%1/ModbusToolsUpdate").arg(tempRoot));
    QDir().mkpath(workingDir.path());

    const QUrl updateUrl(pendingUpdateOnlyUrl_);
    const QString updateFileName = QFileInfo(updateUrl.path()).fileName();
    if (updateFileName.isEmpty()) {
        return false;
    }
    const QString updateFilePath = workingDir.filePath(updateFileName);

    QString errorMessage;
    if (!downloadUpdateAsset(updateUrl, updateFilePath, errorMessage)) {
        if (checkingUpdateManually_) {
            QMessageBox::warning(this, tr("Update Check Failed"), errorMessage);
        }
        return false;
    }

    QString expectedSha = pendingUpdateOnlySha256_.trimmed().toLower();
    if (expectedSha.isEmpty()) {
        if (pendingChecksumsUrl_.isEmpty()) {
            if (checkingUpdateManually_) {
                QMessageBox::warning(this, tr("Update Check Failed"), tr("Missing update checksum"));
            }
            return false;
        }
        const QString checksumsPath = workingDir.filePath("sha256sums.txt");
        if (!downloadUpdateAsset(QUrl(pendingChecksumsUrl_), checksumsPath, errorMessage)) {
            if (checkingUpdateManually_) {
                QMessageBox::warning(this, tr("Update Check Failed"), errorMessage);
            }
            return false;
        }
        expectedSha = resolveSha256FromChecksums(checksumsPath, updateFileName);
        if (expectedSha.isEmpty()) {
            if (checkingUpdateManually_) {
                QMessageBox::warning(this, tr("Update Check Failed"), tr("Unable to resolve update checksum"));
            }
            return false;
        }
    }

    const QString actualSha = calculateFileSha256(updateFilePath);
    if (actualSha.isEmpty() || actualSha.compare(expectedSha, Qt::CaseInsensitive) != 0) {
        if (checkingUpdateManually_) {
            QMessageBox::warning(this, tr("Update Check Failed"), tr("Update file verification failed"));
        }
        return false;
    }

    const QString taskFilePath = workingDir.filePath("update_task.json");
    if (!writeUpdateTaskFile(taskFilePath, updateFilePath, expectedSha, errorMessage)) {
        if (checkingUpdateManually_) {
            QMessageBox::warning(this, tr("Update Check Failed"), errorMessage);
        }
        return false;
    }

    if (!launchUpdater(taskFilePath, errorMessage)) {
        if (checkingUpdateManually_) {
            QMessageBox::warning(this, tr("Update Check Failed"), errorMessage);
        }
        return false;
    }

    qApp->quit();
    return true;
}

void MainWindow::promptUpdateAction(const QString& currentVersion) {
    const QString fullDownloadUrl = pendingFullPackageUrl_.isEmpty() ? pendingReleaseUrl_ : pendingFullPackageUrl_;
    const bool hasUpdateOnly = !pendingUpdateOnlyUrl_.isEmpty();
    const QString baseMessage = tr("Current version: v%1\nLatest version: v%2")
                                    .arg(currentVersion, pendingLatestVersion_);

    if (!hasUpdateOnly) {
        const QString message = baseMessage + tr("\n\nUpdateOnly package is unavailable for this release.\nOpen download page now?");
        const auto result = QMessageBox::question(this, tr("Update Available"), message, QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes && !fullDownloadUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(fullDownloadUrl));
        }
        return;
    }

    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setWindowTitle(tr("Update Available"));
    messageBox.setText(baseMessage + tr("\n\nChoose update method:"));
    auto* updateNowButton = messageBox.addButton(tr("Update Main Program"), QMessageBox::AcceptRole);
    QPushButton* downloadFullButton = nullptr;
    if (!fullDownloadUrl.isEmpty()) {
        downloadFullButton = messageBox.addButton(tr("Download Full Package"), QMessageBox::ActionRole);
    }
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.exec();

    if (messageBox.clickedButton() == updateNowButton) {
        if (!tryStartSilentUpdate()) {
            QMessageBox::warning(this, tr("Update Check Failed"), tr("Main program update failed"));
        }
        return;
    }
    if (downloadFullButton && messageBox.clickedButton() == downloadFullButton) {
        QDesktopServices::openUrl(QUrl(fullDownloadUrl));
    }
}

void MainWindow::handleUpdateAvailable(const QString& currentVersion,
                                       const QString& latestVersion,
                                       const QString& updateOnlyUrl,
                                       const QString& updateOnlySha256,
                                       const QString& checksumsUrl,
                                       const QString& fullPackageUrl,
                                       const QString& releaseUrl) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(true);
    }

    updateAvailable_ = true;
    pendingLatestVersion_ = latestVersion;
    pendingUpdateOnlyUrl_ = updateOnlyUrl;
    pendingUpdateOnlySha256_ = updateOnlySha256;
    pendingChecksumsUrl_ = checksumsUrl;
    pendingFullPackageUrl_ = fullPackageUrl;
    pendingDownloadUrl_ = pendingFullPackageUrl_;
    pendingReleaseUrl_ = releaseUrl;
    refreshUpdateIndicators();

    if (!checkingUpdateManually_) {
        return;
    }
    promptUpdateAction(currentVersion);
}

void MainWindow::handleNoUpdateAvailable(const QString& currentVersion) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(true);
    }
    updateAvailable_ = false;
    pendingLatestVersion_.clear();
    pendingUpdateOnlyUrl_.clear();
    pendingUpdateOnlySha256_.clear();
    pendingChecksumsUrl_.clear();
    pendingFullPackageUrl_.clear();
    pendingDownloadUrl_.clear();
    pendingReleaseUrl_.clear();
    refreshUpdateIndicators();
    if (checkingUpdateManually_) {
        QMessageBox::information(this, tr("No Updates"),
                                 tr("You are already using the latest version: v%1").arg(currentVersion));
    }
}

void MainWindow::handleUpdateCheckFailed(const QString& reason) {
    if (checkUpdatesAction_) {
        checkUpdatesAction_->setEnabled(true);
    }
    if (checkingUpdateManually_) {
        QMessageBox::warning(this, tr("Update Check Failed"), reason);
    }
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
        navigationList_->item(0)->setToolTip(tr("Modbus TCP"));
        navigationList_->item(1)->setToolTip(tr("Modbus RTU"));
        navigationList_->item(2)->setToolTip(tr("TCP Client"));
        navigationList_->item(3)->setToolTip(tr("Serial Port"));
        navigationList_->item(4)->setToolTip(tr("Frame Analyzer"));
        if (!navigationCollapsed_) {
            navigationList_->item(0)->setText(tr("Modbus TCP"));
            navigationList_->item(1)->setText(tr("Modbus RTU"));
            navigationList_->item(2)->setText(tr("TCP Client"));
            navigationList_->item(3)->setText(tr("Serial Port"));
            navigationList_->item(4)->setText(tr("Frame Analyzer"));
        } else {
            navigationList_->item(0)->setText(QString());
            navigationList_->item(1)->setText(QString());
            navigationList_->item(2)->setText(QString());
            navigationList_->item(3)->setText(QString());
            navigationList_->item(4)->setText(QString());
        }
    }
    updateNavigationToggleUi();
    if (navigationPane_) {
        navigationPane_->setFixedWidth(navigationCollapsed_ ? navigationCollapsedWidth_ : navigationExpandedWidth_);
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
    if (updateSettingsAction_) {
        updateSettingsAction_->setText(tr("Update Settings"));
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
    refreshUpdateIndicators();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

} // namespace ui
