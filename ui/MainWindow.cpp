#include "MainWindow.h"
#include "AppConstants.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/generic_serial/GenericSerialView.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "widgets/DisclaimerDialog.h"
#include "widgets/UpdateSettingsDialog.h"
#include "common/CompactMenuBarStyle.h"
#include "common/SettingsKeys.h"
#include "common/ThemeController.h"
#include "common/ThemeUiHelpers.h"
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
#include <QCloseEvent>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
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
#include <QLocale>
#include <QImage>
#include <QPainter>
#include <QIcon>
#include <QColor>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QPointer>
#include <QScrollArea>
#include <QStandardPaths>
#include <QFutureWatcher>
#include <QThread>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>
#include <spdlog/spdlog.h>
#include <functional>
#include "modbus/base/ModbusConfig.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
struct UpdatePreparationResult {
    bool success = false;
    QString errorMessage;
    QString taskFilePath;
    QString expectedSha;
    QString actualSha;
};

QString calculateFileSha256Sync(const QString& filePath)
{
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

QString resolveSha256FromChecksumsSync(const QString& checksumsPath, const QString& targetFileName)
{
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

bool writeUpdateTaskFileSync(const QString& taskFilePath,
                             const QString& newExePath,
                             const QString& expectedVersion,
                             const QString& expectedSha256,
                             QString& errorMessage)
{
    const QString targetExePath = QCoreApplication::applicationFilePath();
    const QString backupExePath = targetExePath + ".bak";
    QJsonObject root;
    root.insert("schemaVersion", 1);
    root.insert("launcherPid", static_cast<qint64>(QCoreApplication::applicationPid()));
    root.insert("targetExePath", QDir::toNativeSeparators(targetExePath));
    root.insert("newExePath", QDir::toNativeSeparators(newExePath));
    root.insert("backupExePath", QDir::toNativeSeparators(backupExePath));
    root.insert("expectedVersion", expectedVersion);
    root.insert("expectedSha256", expectedSha256);
    root.insert("restartAfterUpdate", true);

    QFile taskFile(taskFilePath);
    if (!taskFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorMessage = ui::MainWindow::tr("Failed to create update task file");
        return false;
    }

    const QByteArray jsonBytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (taskFile.write(jsonBytes) != jsonBytes.size()) {
        errorMessage = ui::MainWindow::tr("Failed to write update task file");
        return false;
    }
    return true;
}

class UpdateDownloadTask final : public QObject {
public:
    UpdateDownloadTask(ui::MainWindow* owner,
                       QPointer<QProgressDialog> progressDialog,
                       QString filePath,
                       std::function<void(bool, const QString&)> onFinished)
        : owner_(owner),
          progressDialog_(std::move(progressDialog)),
          filePath_(std::move(filePath)),
          onFinished_(std::move(onFinished))
    {
    }

    void start(const QUrl& url)
    {
        if (!url.isValid()) {
            finish(false, ui::MainWindow::tr("Invalid update URL"));
            return;
        }

        QFileInfo fileInfo(filePath_);
        QDir dir = fileInfo.dir();
        if (!dir.exists() && !dir.mkpath(".")) {
            finish(false, ui::MainWindow::tr("Failed to create update directory"));
            return;
        }

        outputFile_.setFileName(filePath_);
        if (!outputFile_.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            finish(false, ui::MainWindow::tr("Failed to write update file"));
            return;
        }

        manager_ = new QNetworkAccessManager(this);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QStringLiteral("Modbus-Tools/%1").arg(ui::common::UpdateChecker::currentVersion()));
        reply_ = manager_->get(request);

        connect(reply_, &QNetworkReply::readyRead, this, [this]() {
            if (reply_) {
                outputFile_.write(reply_->readAll());
            }
        });
        connect(reply_, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
            if (!owner_ || !progressDialog_) {
                return;
            }
            const int value = total > 0 ? qBound(0, static_cast<int>((received * 100) / total), 100) : 0;
            QMetaObject::invokeMethod(owner_.data(), [dialog = progressDialog_, value]() {
                if (dialog) {
                    dialog->setValue(value);
                }
            }, Qt::QueuedConnection);
        });
        connect(reply_, &QNetworkReply::finished, this, [this]() {
            if (reply_) {
                outputFile_.write(reply_->readAll());
            }
            outputFile_.close();

            bool success = true;
            QString errorMessage;
            if (!reply_ || reply_->error() != QNetworkReply::NoError) {
                success = false;
                errorMessage = (reply_ && reply_->error() == QNetworkReply::OperationCanceledError)
                    ? ui::MainWindow::tr("Download canceled")
                    : (reply_ ? reply_->errorString() : ui::MainWindow::tr("Download canceled"));
                QFile::remove(filePath_);
            } else if (QFileInfo(filePath_).size() <= 0) {
                success = false;
                errorMessage = ui::MainWindow::tr("Downloaded file is empty");
                QFile::remove(filePath_);
            }

            finish(success, errorMessage);
        });
    }

    void cancel()
    {
        if (reply_) {
            reply_->abort();
        }
    }

private:
    void finish(bool success, const QString& errorMessage)
    {
        if (owner_ && progressDialog_) {
            QMetaObject::invokeMethod(owner_.data(), [dialog = progressDialog_]() {
                if (dialog) {
                    dialog->close();
                }
            }, Qt::QueuedConnection);
        }

        if (owner_) {
            QMetaObject::invokeMethod(owner_.data(), [callback = onFinished_, success, errorMessage]() {
                callback(success, errorMessage);
            }, Qt::QueuedConnection);
        }
        deleteLater();
    }

    QPointer<ui::MainWindow> owner_;
    QPointer<QProgressDialog> progressDialog_;
    QString filePath_;
    std::function<void(bool, const QString&)> onFinished_;
    QFile outputFile_;
    QPointer<QNetworkReply> reply_;
    QNetworkAccessManager* manager_ = nullptr;
};

void downloadUpdateAssetAsync(ui::MainWindow* owner,
                              const QUrl& url,
                              const QString& filePath,
                              const std::function<void(bool, const QString&)>& onFinished)
{
    if (!owner) {
        return;
    }

    const QString assetName = QFileInfo(filePath).fileName();
    auto* progressDialog = new QProgressDialog(ui::MainWindow::tr("Downloading %1").arg(assetName),
                                               ui::MainWindow::tr("Cancel"),
                                               0,
                                               100,
                                               owner);
    progressDialog->setWindowTitle(ui::MainWindow::tr("Downloading Update"));
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);

    auto* workerThread = new QThread();
    auto* task = new UpdateDownloadTask(owner, progressDialog, filePath, onFinished);
    const QPointer<UpdateDownloadTask> safeTask(task);
    task->moveToThread(workerThread);
    QObject::connect(task, &QObject::destroyed, workerThread, &QThread::quit, Qt::UniqueConnection);
    QObject::connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    QObject::connect(progressDialog, &QProgressDialog::canceled, owner, [safeTask]() {
        if (!safeTask) {
            return;
        }
        QMetaObject::invokeMethod(safeTask.data(), [safeTask]() {
            if (safeTask) {
                safeTask->cancel();
            }
        }, Qt::QueuedConnection);
    });

    workerThread->start();
    QMetaObject::invokeMethod(task, [task, url]() {
        task->start(url);
    }, Qt::QueuedConnection);
    progressDialog->show();
}

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

QString normalizedAppLocale(QString locale) {
    locale = locale.trimmed();
    if (locale == QStringLiteral("en_US") ||
        locale == QStringLiteral("zh_CN") ||
        locale == QStringLiteral("zh_TW") ||
        locale == QStringLiteral("system")) {
        return locale;
    }
    return QStringLiteral("system");
}

QString effectiveAppLocale(const QString& locale) {
    if (locale == QStringLiteral("zh_CN") || locale == QStringLiteral("zh_TW")) {
        return locale;
    }
    if (locale == QStringLiteral("en_US")) {
        return QStringLiteral("en_US");
    }

    const QString systemName = QLocale::system().name();
    return systemName == QStringLiteral("zh_CN")
        ? QStringLiteral("zh_CN")
        : (systemName == QStringLiteral("zh_TW") ? QStringLiteral("zh_TW") : QStringLiteral("en_US"));
}

int calculateExpandedNavigationWidth(const QListWidget* navigationList) {
    if (!navigationList) {
        return app::constants::Constants::Ui::kNavigationExpandedWidth;
    }

    const QFontMetrics metrics(navigationList->fontMetrics());
    int maxTextWidth = 0;
    for (int index = 0; index < navigationList->count(); ++index) {
        const QListWidgetItem* item = navigationList->item(index);
        if (!item) {
            continue;
        }
        const QString title = item->toolTip().isEmpty() ? item->text() : item->toolTip();
        maxTextWidth = qMax(maxTextWidth, metrics.horizontalAdvance(title));
    }

    const int iconWidth = qMax(app::constants::Constants::Ui::kNavigationMinIconWidth,
                               navigationList->iconSize().width());
    const int frameWidth = navigationList->frameWidth() * 2;
    const int leftInset = app::constants::Constants::Ui::kNavigationLeftInset;
    const int textGap = app::constants::Constants::Ui::kNavigationTextGap;
    const int rightInset = app::constants::Constants::Ui::kNavigationRightInset;
    return qMax(app::constants::Constants::Ui::kNavigationExpandedWidth,
                frameWidth + leftInset + iconWidth + textGap + maxTextWidth + rightInset);
}
}

namespace ui {

MainWindow::MainWindow(common::ISettingsService* settingsService,
                       common::ThemeController* themeController,
                       QWidget *parent)
    : QMainWindow(parent),
      themeController_(themeController),
      settingsService_(settingsService) {
    Q_ASSERT(themeController_ != nullptr);
    Q_ASSERT(settingsService_ != nullptr);
    setupUi();
}

MainWindow::~MainWindow() {
    spdlog::info("MainWindow: Destructor entry");
}

void MainWindow::setupUi() {
    setWindowTitle(tr("Modbus Tools"));
    setMinimumSize(720, 480);
    menuBar()->setStyle(new common::CompactMenuBarStyle(menuBar()->style()->name()));
    using namespace common::settings_keys;
    resize(1280, 800);
    const QByteArray geometry = settingsService_->value(kAppMainWindowGeometry).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

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
    modbusTcpView_ = new views::modbus_tcp::ModbusTcpView(settingsService_, this);
    stackedWidget_->addWidget(createScrollablePage(modbusTcpView_, stackedWidget_));
    modbusRtuView_ = new views::modbus_rtu::ModbusRtuView(settingsService_, this);
    stackedWidget_->addWidget(createScrollablePage(modbusRtuView_, stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new views::generic_tcp::GenericTcpView(settingsService_, this), stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new views::generic_serial::GenericSerialView(settingsService_, this), stackedWidget_));
    stackedWidget_->addWidget(createScrollablePage(new widgets::FrameAnalyzerWidget(settingsService_, this), stackedWidget_));

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
    setupThemeToggle();
    const QByteArray windowState = settingsService_->value(kAppMainWindowState).toByteArray();
    if (!windowState.isEmpty()) {
        restoreState(windowState);
    }
    parameterWheelBlocker_ = new ParameterWheelBlocker(this);
    qApp->installEventFilter(parameterWheelBlocker_);
    currentLocale_ = normalizedAppLocale(settingsService_->value(kAppLanguage).toString());
    navigationCollapsed_ = settingsService_->value(kAppNavigationCollapsed).toBool();
    setNavigationCollapsed(navigationCollapsed_);
    updateThemeUi();
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
    navigationExpandedWidth_ = calculateExpandedNavigationWidth(navigationList_);
    paneLayout->addWidget(navigationList_);

    common::ThemeUiHelpers::applyNavigationTheme(navigationList_->palette(),
                                                 navigationPane_,
                                                 navigationToggleButton_,
                                                 navigationList_);
    connect(navigationToggleButton_, &QToolButton::clicked, this, [this]() {
        setNavigationCollapsed(!navigationCollapsed_);
        settingsService_->setValue(common::settings_keys::kAppNavigationCollapsed, navigationCollapsed_);
    });
}

void MainWindow::setNavigationCollapsed(bool collapsed) {
    navigationCollapsed_ = collapsed;
    navigationExpandedWidth_ = calculateExpandedNavigationWidth(navigationList_);
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

void MainWindow::setupThemeToggle() {
    themeToggleButton_ = new QToolButton(menuBar());
    themeToggleButton_->setAutoRaise(true);
    themeToggleButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    themeToggleButton_->setCursor(Qt::PointingHandCursor);

    const int textHeight = menuBar()->fontMetrics().height();
    const int buttonExtent = qMax(textHeight + 6, style()->pixelMetric(QStyle::PM_SmallIconSize) + 2);
    themeToggleButton_->setFixedSize(buttonExtent, buttonExtent);
    menuBar()->setCornerWidget(themeToggleButton_, Qt::TopRightCorner);

    connect(themeToggleButton_, &QToolButton::clicked, themeController_, &common::ThemeController::cycleMode);
    connect(themeController_, &common::ThemeController::themeChanged, this, &MainWindow::updateThemeUi);
    updateThemeToggleUi();
}

void MainWindow::updateThemeUi() {
    common::ThemeUiHelpers::applyNavigationTheme(palette(),
                                                 navigationPane_,
                                                 navigationToggleButton_,
                                                 navigationList_);
    updateThemeToggleUi();
}

void MainWindow::updateThemeToggleUi() {
    if (!themeToggleButton_) {
        return;
    }

    common::ThemeUiHelpers::applyThemeToggleAppearance(themeToggleButton_->palette(),
                                                       menuBar(),
                                                       style(),
                                                       themeToggleButton_,
                                                       themeController_->currentMode());

    QString tooltip;
    switch (themeController_->currentMode()) {
    case common::Theme::Mode::Auto:
        tooltip = tr("Theme: Auto");
        break;
    case common::Theme::Mode::Light:
        tooltip = tr("Theme: Light");
        break;
    case common::Theme::Mode::Dark:
        tooltip = tr("Theme: Dark");
        break;
    }
    themeToggleButton_->setToolTip(tooltip);
}

void MainWindow::loadModbusSettings() {
    using namespace common::settings_keys;
    modbusTimeoutMs_ = settingsService_->value(kModbusTimeoutMs).toInt();
    modbusRetries_ = settingsService_->value(kModbusRetryCount).toInt();
    modbusRetryIntervalMs_ = settingsService_->value(kModbusRetryIntervalMs).toInt();
    modbusRetryEnabled_ = settingsService_->value(kModbusRetryEnabled).toBool();
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
    updateCheckFrequency_ = settingsService_->value(common::settings_keys::kAppUpdateCheckFrequency).toString();
    if (updateCheckFrequency_.isEmpty()) {
        updateCheckFrequency_ = QStringLiteral("startup");
    }
}

void MainWindow::openModbusSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Modbus Settings"));
    auto layout = new QVBoxLayout(&dialog);
    auto formLayout = new QFormLayout();
    auto timeoutSpin = new QSpinBox(&dialog);
    timeoutSpin->setRange(app::constants::Constants::Modbus::kMinTimeoutMs,
                          app::constants::Constants::Modbus::kMaxTimeoutMs);
    timeoutSpin->setSingleStep(app::constants::Constants::Modbus::kTimeoutStepMs);
    timeoutSpin->setValue(modbusTimeoutMs_);
    formLayout->addRow(tr("Request Timeout (ms):"), timeoutSpin);

    auto retryEnableCheck = new QCheckBox(&dialog);
    retryEnableCheck->setChecked(modbusRetryEnabled_);
    formLayout->addRow(tr("Enable Retry:"), retryEnableCheck);

    auto retrySpin = new QSpinBox(&dialog);
    retrySpin->setRange(app::constants::Constants::Modbus::kMinRetryCount,
                        app::constants::Constants::Modbus::kMaxRetryCount);
    retrySpin->setValue(modbusRetries_);
    formLayout->addRow(tr("Retry Count:"), retrySpin);

    auto retryIntervalSpin = new QSpinBox(&dialog);
    retryIntervalSpin->setRange(app::constants::Constants::Modbus::kMinRetryIntervalMs,
                                app::constants::Constants::Modbus::kMaxRetryIntervalMs);
    retryIntervalSpin->setSingleStep(app::constants::Constants::Modbus::kRetryIntervalStepMs);
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
        using namespace common::settings_keys;
        settingsService_->setValue(kModbusTimeoutMs, modbusTimeoutMs_);
        settingsService_->setValue(kModbusRetryCount, modbusRetries_);
        settingsService_->setValue(kModbusRetryIntervalMs, modbusRetryIntervalMs_);
        settingsService_->setValue(kModbusRetryEnabled, modbusRetryEnabled_);
        applyModbusSettingsToViews();
    }
}

void MainWindow::openUpdateSettingsDialog() {
    widgets::UpdateSettingsDialog dialog(updateCheckFrequency_, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    updateCheckFrequency_ = dialog.selectedFrequency();
    settingsService_->setValue(common::settings_keys::kAppUpdateCheckFrequency, updateCheckFrequency_);
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
    settingsService_->setValue(common::settings_keys::kAppUpdateLastCheckUtc, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    updateChecker_->checkForUpdates();
}

bool MainWindow::shouldAutoCheckUpdates() const {
    if (updateCheckFrequency_ == "off") {
        return false;
    }
    if (updateCheckFrequency_ == "startup") {
        return true;
    }
    const QString lastCheckUtc = settingsService_->value(common::settings_keys::kAppUpdateLastCheckUtc).toString();
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



QString MainWindow::calculateFileSha256(const QString& filePath) const {
    return calculateFileSha256Sync(filePath);
}

QString MainWindow::resolveSha256FromChecksums(const QString& checksumsPath, const QString& targetFileName) const {
    return resolveSha256FromChecksumsSync(checksumsPath, targetFileName);
}

bool MainWindow::writeUpdateTaskFile(const QString& taskFilePath,
                                     const QString& newExePath,
                                     const QString& expectedSha256,
                                     QString& errorMessage) const {
    return writeUpdateTaskFileSync(taskFilePath, newExePath, pendingLatestVersion_, expectedSha256, errorMessage);
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

void MainWindow::startSilentUpdate() {
    if (pendingUpdateOnlyUrl_.isEmpty()) {
        if (checkingUpdateManually_) {
            QMessageBox::warning(this, tr("Update Check Failed"), tr("Main program update failed"));
        }
        return;
    }

    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempRoot.isEmpty()) {
        if (checkingUpdateManually_) QMessageBox::warning(this, tr("Update Check Failed"), tr("Main program update failed"));
        return;
    }
    const QDir workingDir(QStringLiteral("%1/ModbusToolsUpdate").arg(tempRoot));
    QDir().mkpath(workingDir.path());

    const QUrl updateUrl(pendingUpdateOnlyUrl_);
    const QString updateFileName = QFileInfo(updateUrl.path()).fileName();
    if (updateFileName.isEmpty()) {
        if (checkingUpdateManually_) {
            QMessageBox::warning(this, tr("Update Check Failed"), tr("Main program update failed"));
        }
        return;
    }
    const QString updateFilePath = workingDir.filePath(updateFileName);
    const QString normalizedExpectedSha = pendingUpdateOnlySha256_.trimmed().toLower();
    const QString checksumsUrl = pendingChecksumsUrl_;

    downloadUpdateAssetAsync(this, updateUrl, updateFilePath, [this, updateFilePath, normalizedExpectedSha, checksumsUrl](bool success, const QString& errorMessage) {
        if (!success) {
            if (checkingUpdateManually_) {
                QMessageBox::warning(this, tr("Update Check Failed"), errorMessage);
            }
            return;
        }

        if (normalizedExpectedSha.isEmpty() && !checksumsUrl.isEmpty()) {
            const QString checksumsPath = QFileInfo(updateFilePath).dir().filePath("sha256sums.txt");
            downloadUpdateAssetAsync(this, QUrl(checksumsUrl), checksumsPath, [this, updateFilePath, checksumsPath](bool checksumSuccess, const QString& checksumError) {
                if (!checksumSuccess) {
                    if (checkingUpdateManually_) {
                        QMessageBox::warning(this, tr("Update Check Failed"), checksumError);
                    }
                    return;
                }
                processDownloadedUpdate(updateFilePath, QString(), checksumsPath);
            });
            return;
        }

        processDownloadedUpdate(updateFilePath, normalizedExpectedSha);
    });
}

void MainWindow::processDownloadedUpdate(const QString& updateFilePath,
                                         const QString& expectedSha,
                                         const QString& checksumsPath) {
    const QString normalizedExpectedSha = expectedSha.trimmed().toLower();
    const QString normalizedChecksumsPath = checksumsPath;
    const QString updateFileName = QFileInfo(updateFilePath).fileName();
    const QString taskFilePath = QFileInfo(updateFilePath).dir().filePath("update_task.json");
    const QString expectedVersion = pendingLatestVersion_;

    auto result = std::make_shared<UpdatePreparationResult>();
    updatePreparationCancelToken_ = std::make_shared<std::atomic_bool>(false);
    auto cancelToken = updatePreparationCancelToken_;
    auto* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, result, cancelToken]() {
        if (!cancelToken || cancelToken->load()) {
            return;
        }
        if (!result->success) {
            if (checkingUpdateManually_) {
                QMessageBox::warning(this, tr("Update Check Failed"), result->errorMessage);
            }
            return;
        }

        QString errorMessage;
        if (!launchUpdater(result->taskFilePath, errorMessage)) {
            if (checkingUpdateManually_) {
                QMessageBox::warning(this, tr("Update Check Failed"), errorMessage);
            }
            return;
        }

        qApp->quit();
    });
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QObject::deleteLater);
    watcher->setFuture(QtConcurrent::run([result,
                                          updateFilePath,
                                          updateFileName,
                                          normalizedExpectedSha,
                                          normalizedChecksumsPath,
                                          taskFilePath,
                                          expectedVersion,
                                          cancelToken]() {
        if (!cancelToken || cancelToken->load()) {
            return;
        }
        result->expectedSha = normalizedExpectedSha;
        if (result->expectedSha.isEmpty() && !normalizedChecksumsPath.isEmpty()) {
            result->expectedSha = resolveSha256FromChecksumsSync(normalizedChecksumsPath, updateFileName);
        }
        if (!cancelToken || cancelToken->load()) {
            return;
        }
        if (result->expectedSha.isEmpty()) {
            result->errorMessage = ui::MainWindow::tr("Missing update checksum");
            return;
        }

        result->actualSha = calculateFileSha256Sync(updateFilePath);
        if (result->actualSha.isEmpty()) {
            result->errorMessage = ui::MainWindow::tr("Failed to calculate update checksum");
            return;
        }
        if (result->actualSha.compare(result->expectedSha, Qt::CaseInsensitive) != 0) {
            result->errorMessage = ui::MainWindow::tr("Checksum mismatch. Expected: %1\nActual: %2")
                                       .arg(result->expectedSha, result->actualSha);
            return;
        }
        if (!cancelToken || cancelToken->load()) {
            return;
        }

        QString errorMessage;
        if (!writeUpdateTaskFileSync(taskFilePath, updateFilePath, expectedVersion, result->expectedSha, errorMessage)) {
            result->errorMessage = errorMessage;
            return;
        }

        result->success = true;
        result->taskFilePath = taskFilePath;
    }));
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
        startSilentUpdate();
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
    if (settingsService_->value(common::settings_keys::kAppDisclaimerAccepted).toBool()) {
        return;
    }

    widgets::DisclaimerDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        qApp->quit();
        return;
    }
    settingsService_->setValue(common::settings_keys::kAppDisclaimerAccepted, true);
}

void MainWindow::saveWindowSettings() {
    settingsService_->setValue(common::settings_keys::kAppMainWindowGeometry, saveGeometry());
    settingsService_->setValue(common::settings_keys::kAppMainWindowState, saveState());
    settingsService_->sync();
}

void MainWindow::applyLanguage(const QString& locale) {
    qApp->removeTranslator(&qtTranslator_);
    qApp->removeTranslator(&appTranslator_);

    currentLocale_ = locale;
    settingsService_->setValue(common::settings_keys::kAppLanguage, currentLocale_);

    const QString effectiveLocale = effectiveAppLocale(locale);

    if (effectiveLocale == "zh_CN") {
        const bool qtLoaded = qtTranslator_.load("qtbase_zh_CN", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        const bool appLoaded = appTranslator_.load(":/i18n/Modbus-Tools_zh_CN.qm");
        if (qtLoaded) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appLoaded) {
            qApp->installTranslator(&appTranslator_);
        }
    } else if (effectiveLocale == "zh_TW") {
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
        langEnAction_->setChecked(effectiveAppLocale(currentLocale_) == "en_US");
    }
    if (langZhCnAction_) {
        langZhCnAction_->setText(tr("简体中文"));
        langZhCnAction_->setChecked(effectiveAppLocale(currentLocale_) == "zh_CN");
    }
    if (langZhTwAction_) {
        langZhTwAction_->setText(tr("繁體中文"));
        langZhTwAction_->setChecked(effectiveAppLocale(currentLocale_) == "zh_TW");
    }
    updateThemeToggleUi();
    refreshUpdateIndicators();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange) {
        updateThemeUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (updatePreparationCancelToken_) {
        updatePreparationCancelToken_->store(true);
    }
    saveWindowSettings();
    QMainWindow::closeEvent(event);
}

} // namespace ui
