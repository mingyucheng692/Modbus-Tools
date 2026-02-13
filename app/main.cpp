#include <QApplication>
#include "MainWindow.h"
#include "common/Theme.h"
#include "logging/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Apply Light Theme
    ui::common::Theme::applyLight(app);
    
    logging::Init(app.applicationDirPath() + "/logs");

    ui::MainWindow window;
    window.show();

    spdlog::info("Modbus-Tools initialized");

    return app.exec();
}
