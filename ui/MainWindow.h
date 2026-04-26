/**
 * @file MainWindow.h
 * @brief Header file for MainWindow.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "AppConstants.h"
#include <QMainWindow>
#include <QTranslator>
#include <memory>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

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

namespace core::update { class UpdateManager; }
namespace core::common { class SettingsController; }
namespace ui {
namespace views::modbus_tcp { class ModbusTcpView; }
namespace views::modbus_rtu { class ModbusRtuView; }
namespace widgets { class FrameAnalyzerWidget; }
namespace common { class ThemeController; }
namespace common { class UpdateChecker; }
namespace common { class ISettingsService; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(common::ISettingsService* settingsService,
                        common::ThemeController* themeController,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    enum class AnalyzerLinkSource {
        None,
        Tcp,
        Rtu
    };

    // UI Setup
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

    // Logic Bridge / Delegation
    void applyModbusSettingsToViews();
    void openModbusSettingsDialog();
    void openUpdateSettingsDialog();
    void openAboutDialog();
    void checkForUpdates();
    void performUpdateCheck(bool manual);
    bool shouldAutoCheckUpdates() const;
    void refreshUpdateIndicators();
    void promptUpdateAction(const QString& currentVersion);
    void handleUpdateAvailable(const QString& currentVersion,
                               const QString& latestVersion,
                               const QString& updateOnlyUrl,
                               const QString& updateOnlySha256,
                               const QString& checksumsUrl,
                               const QString& fullPackageUrl,
                               const QString& releaseUrl);
    
    void startSilentUpdate();
    void showDisclaimerIfNeeded();
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    
    // Linkage handling
    void handleTcpLinkageToggled(bool active);
    void handleRtuLinkageToggled(bool active);
    void handleLinkageStopRequested();
    void handleLinkageData(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);
    void handleLinkageToggle(AnalyzerLinkSource source, bool active);
    void stopActiveLinkage();
    static AnalyzerLinkSource linkSourceFromProtocol(modbus::core::parser::ProtocolType protocol);

    // UI Components
    QListWidget* navigationList_ = nullptr;
    QWidget* navigationPane_ = nullptr;
    QToolButton* navigationToggleButton_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
    views::modbus_tcp::ModbusTcpView* modbusTcpView_ = nullptr;
    views::modbus_rtu::ModbusRtuView* modbusRtuView_ = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer_ = nullptr;
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
    
    // Services and Controllers
    QTranslator qtTranslator_;
    QTranslator appTranslator_;
    common::ThemeController* themeController_ = nullptr;
    common::UpdateChecker* updateChecker_ = nullptr;
    core::update::UpdateManager* updateManager_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    
    // Local State
    QString currentLocale_ = "en_US";
    bool updateAvailable_ = false;
    bool checkingUpdateManually_ = false;
    
    // Cached Update Info (for prompt)
    struct {
        QString latestVersion;
        QString updateOnlyUrl;
        QString updateOnlySha256;
        QString checksumsUrl;
        QString fullPackageUrl;
        QString releaseUrl;
    } pendingUpdateInfo_;

    QObject* parameterWheelBlocker_ = nullptr;
    bool navigationCollapsed_ = false;
    int navigationExpandedWidth_ = app::constants::Values::Ui::kNavigationExpandedWidth;
    int navigationCollapsedWidth_ = app::constants::Values::Ui::kNavigationCollapsedWidth;
    AnalyzerLinkSource activeLinkSource_ = AnalyzerLinkSource::None;
};

} // namespace ui
