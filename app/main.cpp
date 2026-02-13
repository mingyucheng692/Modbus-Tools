#include <QApplication>
#include "MainWindow.h"
#include "logging/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    logging::Init(app.applicationDirPath() + "/logs");

    ui::MainWindow window;
    window.show();

    spdlog::info("Modbus-Tools initialized");

    return app.exec();
}
