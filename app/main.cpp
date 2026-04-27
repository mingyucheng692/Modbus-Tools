/**
 * @file main.cpp
 * @brief Implementation of main.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QResource>
#include <exception>
#include "MainWindow.h"
#include "common/QtThemeRuntime.h"
#include "common/SettingsService.h"
#include "common/ThemeController.h"
#include "logging/Logger.h"

#ifndef MODBUS_TOOLS_APP_VERSION
#error "MODBUS_TOOLS_APP_VERSION must be defined by CMake"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(i18n);
    Q_INIT_RESOURCE(assets);
    app.setApplicationName("Modbus-Tools");
    app.setApplicationVersion(QStringLiteral(MODBUS_TOOLS_APP_VERSION));

    ui::common::SettingsService settingsService;
    ui::common::QtThemeRuntime themeRuntime(app);
    ui::common::ThemeController themeController(themeRuntime, settingsService);

    try {
        logging::Init(app.applicationDirPath() + "/logs");
    } catch (const std::exception& ex) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Startup Error"),
            QObject::tr("Failed to initialize application logging.\n%1").arg(QString::fromUtf8(ex.what())));
        return 1;
    } catch (...) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Startup Error"),
            QObject::tr("Failed to initialize application logging."));
        return 1;
    }

    app.setWindowIcon(QIcon(":/assets/logo.svg"));

    ui::MainWindow window(&settingsService, &themeController);
    window.setWindowIcon(QIcon(":/assets/logo.svg"));
    window.show();

    spdlog::info("Modbus-Tools initialized");

    return app.exec();
}
