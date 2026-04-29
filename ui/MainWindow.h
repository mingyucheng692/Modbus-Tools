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
#include "application/IMainWindowPresenter.h"
#include "application/UpdateCoordinator.h"
#include <QMainWindow>
#include <memory>

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
namespace common { class ISettingsService; }
namespace shell { class NavigationController; }
struct MainWindowBusinessContext;

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
    void restoreWindowGeometry(const QByteArray& geometry) override;
    void restoreWindowState(const QByteArray& state) override;
    [[nodiscard]] QByteArray saveWindowGeometry() const override;
    [[nodiscard]] QByteArray saveWindowState() const override;
    void applyModbusSettings(int timeoutMs, int retries, int retryIntervalMs) override;
    void openModbusSettingsDialog() override;
    void openUpdateSettingsDialog() override;
    void openAboutDialog() override;
    [[nodiscard]] bool showDisclaimerDialog() override;
    void retranslateUi(const QString& effectiveLocale) override;

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
    void setupSettingsMenu();
    void setupLanguageMenu();
    void setupAboutMenu();
    void setupThemeToggle();
    void updateThemeUi();
    void updateThemeToggleUi();

    // Logic Bridge / Delegation
    void applyModbusSettingsToViews(int timeoutMs, int retries, int retryIntervalMs);
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

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
    common::ThemeController* themeController_ = nullptr;
    std::unique_ptr<shell::NavigationController> navigationController_;
    std::unique_ptr<MainWindowBusinessContext> businessContext_;
    std::unique_ptr<application::IMainWindowPresenter> presenter_;
    
    // Local State
    QString effectiveLocale_ = "en_US";

    QObject* parameterWheelBlocker_ = nullptr;
};

} // namespace ui
