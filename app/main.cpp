#include <QApplication>
#include <QIcon>
#include <QResource>
#include "MainWindow.h"
#include "common/Theme.h"
#include "logging/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(i18n);
    Q_INIT_RESOURCE(assets);
    
    // Apply Light Theme
    ui::common::Theme::applyLight(app);
    
    logging::Init(app.applicationDirPath() + "/logs");

    app.setWindowIcon(QIcon(":/assets/logo.svg"));

    ui::MainWindow window;
    window.setWindowIcon(QIcon(":/assets/logo.svg"));
    window.show();

    spdlog::info("Modbus-Tools initialized");

    return app.exec();
}
