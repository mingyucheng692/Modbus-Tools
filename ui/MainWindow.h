#pragma once

#include <QMainWindow>
#include <QTranslator>

class QStackedWidget;
class QListWidget;
class QAction;
class QActionGroup;
class QMenu;
class QEvent;
class QObject;

namespace ui {
namespace views::modbus_tcp { class ModbusTcpView; }
namespace views::modbus_rtu { class ModbusRtuView; }
namespace common { class UpdateChecker; }
namespace widgets { class UpdateSettingsDialog; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    void createNavigation();
    void setupSettingsMenu();
    void setupLanguageMenu();
    void setupAboutMenu();
    void retranslateUi();
    void applyLanguage(const QString& locale);
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
    void handleUpdateAvailable(const QString& currentVersion,
                               const QString& latestVersion,
                               const QString& downloadUrl,
                               const QString& releaseUrl);
    void handleNoUpdateAvailable(const QString& currentVersion);
    void handleUpdateCheckFailed(const QString& reason);
    void showDisclaimerIfNeeded();
    void changeEvent(QEvent* event) override;

    QListWidget* navigationList_ = nullptr;
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
    QTranslator qtTranslator_;
    QTranslator appTranslator_;
    common::UpdateChecker* updateChecker_ = nullptr;
    QString currentLocale_ = "en_US";
    int modbusTimeoutMs_ = 1000;
    int modbusRetries_ = 0;
    int modbusRetryIntervalMs_ = 100;
    bool modbusRetryEnabled_ = false;
    QString updateCheckFrequency_ = "startup";
    bool updateAvailable_ = false;
    bool checkingUpdateManually_ = false;
    QString pendingLatestVersion_;
    QString pendingDownloadUrl_;
    QString pendingReleaseUrl_;
    QObject* parameterWheelBlocker_ = nullptr;
};

} // namespace ui
