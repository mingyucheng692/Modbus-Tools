#pragma once

#include "common/ISettingsService.h"
#include <QMainWindow>
#include <QTranslator>

class QStackedWidget;
class QListWidget;
class QAction;
class QActionGroup;
class QMenu;
class QEvent;
class QCloseEvent;
class QObject;
class QWidget;
class QToolButton;
class QUrl;

namespace ui {
namespace views::modbus_tcp { class ModbusTcpView; }
namespace views::modbus_rtu { class ModbusRtuView; }
namespace common { class ThemeController; }
namespace common { class UpdateChecker; }
namespace widgets { class UpdateSettingsDialog; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(common::ISettingsService* settingsService,
                        common::ThemeController* themeController,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    void createNavigation();
    void setNavigationCollapsed(bool collapsed);
    void updateNavigationToggleUi();
    void setupSettingsMenu();
    void setupLanguageMenu();
    void setupAboutMenu();
    void setupThemeToggle();
    void retranslateUi();
    void applyLanguage(const QString& locale);
    void updateThemeUi();
    void updateThemeToggleUi();
    void loadModbusSettings();
    void loadUpdateSettings();
    void applyModbusSettingsToViews();
    void openModbusSettingsDialog();
    void openUpdateSettingsDialog();
    void openAboutDialog();
    void checkForUpdates();
    void performUpdateCheck(bool manual);
    bool shouldAutoCheckUpdates() const;
    void refreshUpdateIndicators();
    void cleanupUpdateArtifacts();
    void promptUpdateAction(const QString& currentVersion);
    bool tryStartSilentUpdate();
    void saveWindowSettings();
    bool downloadUpdateAsset(const QUrl& url, const QString& filePath, QString& errorMessage) const;
    QString calculateFileSha256(const QString& filePath) const;
    QString resolveSha256FromChecksums(const QString& checksumsPath, const QString& targetFileName) const;
    bool writeUpdateTaskFile(const QString& taskFilePath,
                             const QString& newExePath,
                             const QString& expectedSha256,
                             QString& errorMessage) const;
    bool launchUpdater(const QString& taskFilePath, QString& errorMessage) const;
    void handleUpdateAvailable(const QString& currentVersion,
                               const QString& latestVersion,
                               const QString& updateOnlyUrl,
                               const QString& updateOnlySha256,
                               const QString& checksumsUrl,
                               const QString& fullPackageUrl,
                               const QString& releaseUrl);
    void handleNoUpdateAvailable(const QString& currentVersion);
    void handleUpdateCheckFailed(const QString& reason);
    void showDisclaimerIfNeeded();
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

    QListWidget* navigationList_ = nullptr;
    QWidget* navigationPane_ = nullptr;
    QToolButton* navigationToggleButton_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
    views::modbus_tcp::ModbusTcpView* modbusTcpView_ = nullptr;
    views::modbus_rtu::ModbusRtuView* modbusRtuView_ = nullptr;
    QMenu* settingsMenu_ = nullptr;
    QAction* modbusSettingsAction_ = nullptr;
    QAction* updateSettingsAction_ = nullptr;
    QMenu* aboutMenu_ = nullptr;
    QAction* checkUpdatesAction_ = nullptr;
    QAction* aboutAction_ = nullptr;
    QMenu* languageMenu_ = nullptr;
    QActionGroup* languageActionGroup_ = nullptr;
    QAction* langEnAction_ = nullptr;
    QAction* langZhCnAction_ = nullptr;
    QAction* langZhTwAction_ = nullptr;
    QToolButton* themeToggleButton_ = nullptr;
    QTranslator qtTranslator_;
    QTranslator appTranslator_;
    common::ThemeController* themeController_ = nullptr;
    common::UpdateChecker* updateChecker_ = nullptr;
    common::ISettingsService* settingsService_ = nullptr;
    QString currentLocale_ = "en_US";
    int modbusTimeoutMs_ = 1000;
    int modbusRetries_ = 0;
    int modbusRetryIntervalMs_ = 100;
    bool modbusRetryEnabled_ = false;
    QString updateCheckFrequency_ = "startup";
    bool updateAvailable_ = false;
    bool checkingUpdateManually_ = false;
    QString pendingLatestVersion_;
    QString pendingUpdateOnlyUrl_;
    QString pendingUpdateOnlySha256_;
    QString pendingChecksumsUrl_;
    QString pendingFullPackageUrl_;
    QString pendingDownloadUrl_;
    QString pendingReleaseUrl_;
    QObject* parameterWheelBlocker_ = nullptr;
    bool navigationCollapsed_ = false;
    int navigationExpandedWidth_ = 188;
    int navigationCollapsedWidth_ = 52;
};

} // namespace ui
