#include <QApplication>
#include <QMainWindow>

#include "logging/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    logging::Init(app.applicationDirPath() + "/logs");

    QMainWindow window;
    window.setWindowTitle("Modbus Tools");
    window.resize(800, 600);
    window.show();

    spdlog::info("Modbus-Tools initialized");

    return app.exec();
}
