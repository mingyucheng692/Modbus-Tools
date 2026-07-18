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
#include <QMainWindow>
#include <memory>

namespace infra::platform { class PathResolver; }

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

namespace core::common { class ISettingsService; class SettingsController; }
namespace core::update { class UpdateManager; }

namespace ui {
namespace views::modbus { class ModbusPage; }
namespace widgets { class FrameAnalyzerWidget; }
namespace common { class ThemeController; }
namespace common { class CompactMenuBarStyle; }
namespace common { class UpdateChecker; }
namespace shell { class NavigationController; }
namespace application { class AnalyzerLinkCoordinator; }
namespace application { class LanguageCoordinator; }
namespace application { class UpdateCoordinator; }
namespace application { class AppLifecycleCoordinator; }
class UpdateInteractionView;

class MainWindow : public QMainWindow,
                   public application::IMainWindowView {
    Q_OBJECT

public:
    explicit MainWindow(core::common::ISettingsService* settingsService,
                        common::ThemeController* themeController,
                        infra::platform::PathResolver& pathResolver,
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
    void requestQuit() override;
    void retranslateUi(const QString& effectiveLocale) override;

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
    views::modbus::ModbusPage* modbusView_ = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer_ = nullptr;
    QMenu* settingsMenu_ = nullptr;
    QAction* modbusSettingsAction_ = nullptr;
    QAction* updateSettingsAction_ = nullptr;
    QAction* openLogFolderAction_ = nullptr;
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
    std::unique_ptr<UpdateInteractionView> updateInteractionView_;
    common::ThemeController* themeController_ = nullptr;
    std::unique_ptr<shell::NavigationController> navigationController_;
    std::unique_ptr<core::common::SettingsController> settingsController_;
    std::unique_ptr<common::UpdateChecker> updateChecker_;
    std::unique_ptr<core::update::UpdateManager> updateManager_;
    std::unique_ptr<application::AnalyzerLinkCoordinator> analyzerLinkCoordinator_;
    std::unique_ptr<application::LanguageCoordinator> languageCoordinator_;
    std::unique_ptr<application::UpdateCoordinator> updateCoordinator_;
    std::unique_ptr<application::AppLifecycleCoordinator> appLifecycleCoordinator_;
    // Original UI-layer pointer retained for UI-side builders that expect
    // the settings service interface.
    core::common::ISettingsService* settingsService_ = nullptr;
    infra::platform::PathResolver& pathResolver_;
    
    // Local State
    QString effectiveLocale_ = "en_US";

    QObject* parameterWheelBlocker_ = nullptr;

    // Owns the QStyle applied to the menu bar. QWidget::setStyle does not take
    // ownership and QStyle is not a QObject (no parent), so RAII ownership is
    // required to avoid leaking the object (see plan.md L3).
    std::unique_ptr<common::CompactMenuBarStyle> menuBarStyle_;
};

} // namespace ui
