#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "Core/Logging/LogManager.h"
#include "Core/CoreWorker.h"

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

    spdlog::info("Modbus-Tools Started. Main Thread: {}", (uint64_t)QThread::currentThreadId());

    // Setup Core Worker
    QThread workerThread;
    CoreWorker* worker = new CoreWorker();
    worker->moveToThread(&workerThread);

    QObject::connect(&workerThread, &QThread::started, worker, &CoreWorker::init);
    QObject::connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    
    // Test connection
    QObject::connect(worker, &CoreWorker::testResponse, [](const QString& msg){
        spdlog::info("Main Thread received: {}", msg.toStdString());
    });

    workerThread.start();
    
    // Send test command after 1s
    QTimer::singleShot(1000, worker, &CoreWorker::testWorker);

    QMainWindow w;
    w.setWindowTitle("Modbus-Tools (Dev)");
    w.resize(800, 600);
    w.show();

    int ret = a.exec();
    
    workerThread.quit();
    workerThread.wait();
    
    return ret;
}
