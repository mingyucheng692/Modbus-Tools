#include <QApplication>
#include <QIcon>
#include <QResource>
#include "MainWindow.h"
#include "common/Theme.h"
#include "logging/Logger.h"

#ifndef MODBUS_TOOLS_APP_VERSION
#error "MODBUS_TOOLS_APP_VERSION must be defined by CMake"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(i18n);
    Q_INIT_RESOURCE(assets);
    
    // Apply Light Theme
    ui::common::Theme::applyLight(app);
    app.setApplicationName("Modbus-Tools");
    app.setApplicationVersion(QStringLiteral(MODBUS_TOOLS_APP_VERSION));

    logging::Init(app.applicationDirPath() + "/logs");

    app.setWindowIcon(QIcon(":/assets/logo.svg"));

    ui::MainWindow window;
    window.setWindowIcon(QIcon(":/assets/logo.svg"));
    window.show();

    spdlog::info("Modbus-Tools initialized");

    return app.exec();
}
