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
#include <spdlog/spdlog.h>
#include "MainWindow.h"
#include "common/SettingsService.h"
#include "common/ThemeController.h"
#include "infra/platform/PathResolver.h"
#include "infra/logging/Logger.h"

#ifndef MODBUS_TOOLS_APP_VERSION
#error "MODBUS_TOOLS_APP_VERSION must be defined by CMake"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(ui_i18n);
    Q_INIT_RESOURCE(assets);
    app.setApplicationName("Modbus-Tools");
    app.setApplicationVersion(QStringLiteral(MODBUS_TOOLS_APP_VERSION));

    infra::platform::PathResolver pathResolver;
    ui::common::SettingsService settingsService(pathResolver);
    ui::common::ThemeController themeController(settingsService);

    QString loggingError;
    if (!logging::Init(pathResolver.resolveLogDir(), &loggingError)) {
        // Degraded mode, NOT fatal: Init failed before spdlog::set_default_logger
        // ran, so SPDLOG_* macros fall through to spdlog's built-in stdout
        // default logger — the app keeps running without file logs. The tool is
        // portable-only (all data next to the exe); an unwritable exe dir also
        // means settings won't persist, so suggest relocation.
        QMessageBox::warning(
            nullptr,
            QCoreApplication::translate("main", "Logging Unavailable"),
            QCoreApplication::translate("main",
                "Cannot write logs next to the program (%1).\n\n"
                "This run will not create log files, and settings "
                "changes will not be saved.\n"
                "Move the program folder to a writable location "
                "(e.g. Desktop) and restart.").arg(loggingError));
    }

    app.setWindowIcon(QIcon(":/assets/logo.svg"));

    // Stack destruction order (reverse of construction) is load-bearing:
    //   window -> themeController -> settingsService -> pathResolver
    // `window` must die before `settingsService` so widgets stop touching QSettings
    // during teardown; `settingsService` must die before `pathResolver` because
    // SettingsService holds a reference to PathResolver's directory strings.
    // If a future change moves settings persistence to an async worker, the
    // worker must be joined before any of these stack objects go out of scope
    // (otherwise the worker may flush into a destroyed SettingsService -> UAF).
    //
    // `window` lives in an explicit scope so spdlog::shutdown() runs AFTER the
    // window's destructor chain (which still emits log lines during teardown).
    // Shutting down the logger before window destruction would drop those lines.
    int exitCode = 0;
    {
        ui::MainWindow window(&settingsService, &themeController, pathResolver);
        window.setWindowIcon(QIcon(":/assets/logo.svg"));
        window.show();

        SPDLOG_INFO("Modbus-Tools initialized");

        exitCode = app.exec();
    }
    // All QObjects are dead here; drain the async queue and stop the backend
    // thread so the final log lines reach disk before process exit.
    spdlog::shutdown();
    return exitCode;
}
