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

namespace core::common { class ISettingsService; }

namespace ui {
namespace views::modbus_tcp { class ModbusTcpPage; }
namespace views::modbus_rtu { class ModbusRtuPage; }
namespace views::modbus_ascii { class ModbusAsciiPage; }
namespace widgets { class FrameAnalyzerWidget; }
namespace common { class ThemeController; }
namespace common { class CompactMenuBarStyle; }
namespace shell { class NavigationController; }
class UpdateInteractionView;
struct MainWindowBusinessContext;

class MainWindow : public QMainWindow,
                   public application::IMainWindowView {
    Q_OBJECT

public:
    explicit MainWindow(core::common::ISettingsService* settingsService,
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
    views::modbus_tcp::ModbusTcpPage* modbusTcpView_ = nullptr;
    views::modbus_rtu::ModbusRtuPage* modbusRtuView_ = nullptr;
    views::modbus_ascii::ModbusAsciiPage* modbusAsciiView_ = nullptr;
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
    std::unique_ptr<UpdateInteractionView> updateInteractionView_;
    common::ThemeController* themeController_ = nullptr;
    std::unique_ptr<shell::NavigationController> navigationController_;
    std::unique_ptr<MainWindowBusinessContext> businessContext_;
    std::unique_ptr<application::IMainWindowPresenter> presenter_;
    
    // Local State
    QString effectiveLocale_ = "en_US";

    QObject* parameterWheelBlocker_ = nullptr;

    // Owns the QStyle applied to the menu bar. QWidget::setStyle does not take
    // ownership and QStyle is not a QObject (no parent), so RAII ownership is
    // required to avoid leaking the object (see plan.md L3).
    std::unique_ptr<common::CompactMenuBarStyle> menuBarStyle_;
};

} // namespace ui
