#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "Core/Logging/LogManager.h"
#include "Core/CoreWorker.h"
#include "UI/MainWindow.h"

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

    MainWindow w;
    
    // Wire UI -> Core
    QObject::connect(&w, &MainWindow::requestConnectTcp, worker, &CoreWorker::connectTcp);
    QObject::connect(&w, &MainWindow::requestDisconnect, worker, &CoreWorker::disconnect);
    QObject::connect(&w, &MainWindow::requestSend, worker, &CoreWorker::sendRequest);
    
    w.show();

    int ret = a.exec();
    
    workerThread.quit();
    workerThread.wait();
    
    return ret;
}
