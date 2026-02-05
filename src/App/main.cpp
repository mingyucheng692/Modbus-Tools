#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include "Core/Logging/LogManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Init Logging
    LogManager::instance().init();

    // Connect Log Sink to Qt Debug (for verification)
    QObject::connect(LogManager::instance().getQtSinkBase(), &QtLogSinkBase::logReceived, 
        [](const QString& msg, int level){
            qDebug() << "QtLogSink Received:" << msg.trimmed();
        });

    spdlog::info("Modbus-Tools Started (Async Log Check)");
    spdlog::warn("This is a warning from spdlog");

    QMainWindow w;
    w.setWindowTitle("Modbus-Tools (Dev)");
    w.resize(800, 600);
    w.show();

    return a.exec();
}
