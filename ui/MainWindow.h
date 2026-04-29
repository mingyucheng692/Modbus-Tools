/**
 * @file MainWindow.h
 * @brief Header file for MainWindow.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "application/IMainWindowView.h"
#include "application/MainWindowPresenter.h"
#include "application/UpdateCoordinator.h"
#include "AppConstants.h"
#include "AnalyzerLinkageController.h"
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
namespace application { class MainWindowPresenter; }
namespace application { class UpdateCoordinator; }

class MainWindow : public QMainWindow,
                   public application::IMainWindowView,
                   public application::IUpdateInteractionView {
    Q_OBJECT

public:
    explicit MainWindow(common::ISettingsService* settingsService,
                        common::ThemeController* themeController,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    // Main Window View
    void initializeUi() override;
    void setNavigationCollapsed(bool collapsed) override;
    [[nodiscard]] bool isNavigationCollapsed() const override;
    void openModbusSettingsDialog() override;
    void openUpdateSettingsDialog() override;
    void openAboutDialog() override;
    void applyLanguage(const QString& locale) override;
    void persistWindowState() override;

    // Update View Interaction
    void setUpdateCheckActionEnabled(bool enabled) override;
    void setUpdateIndicatorVisible(bool visible) override;
    void showUpdateInfoMessage(const QString& title, const QString& message) override;
    void showUpdateWarningMessage(const QString& title, const QString& message) override;
    void showUpdateCriticalMessage(const QString& title, const QString& message) override;
    bool confirmOpenDownloadPage(const QString& latestVersion) override;
    application::UpdatePromptChoice promptUpdateAction(const QString& currentVersion, const QString& latestVersion) override;
    void showUpdateProgress(core::update::UpdateManager* updateManager) override;

    // UI Setup
    void createNavigation();
    void updateNavigationToggleUi();
    void setupSettingsMenu();
    void setupLanguageMenu();
    void setupAboutMenu();
    void setupThemeToggle();
    void retranslateUi();
    void updateThemeUi();
    void updateThemeToggleUi();

    // Logic Bridge / Delegation
    void applyModbusSettingsToViews();
    void checkForUpdates();
    void showDisclaimerIfNeeded();
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    
    // Linkage handling
    void handleTcpLinkageToggled(bool active);
    void handleRtuLinkageToggled(bool active);
    void handleLinkageStopRequested();
    void handleLinkagePauseToggled(bool paused);
    void handleLinkageData(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);

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
    application::MainWindowPresenter* presenter_ = nullptr;
    application::UpdateCoordinator* updateCoordinator_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    
    // Local State
    QString currentLocale_ = "en_US";

    QObject* parameterWheelBlocker_ = nullptr;
    bool navigationCollapsed_ = false;
    int navigationExpandedWidth_ = app::constants::Values::Ui::kNavigationExpandedWidth;
    int navigationCollapsedWidth_ = app::constants::Values::Ui::kNavigationCollapsedWidth;
    AnalyzerLinkageController* analyzerLinkageController_ = nullptr;
};

} // namespace ui
