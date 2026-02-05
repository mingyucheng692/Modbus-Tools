#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <vector>
#include <memory>
#include "Core/Logging/LogManager.h"
#include "Core/CoreWorker.h"
#include "UI/MainWindow.h"
#include "UI/Utils/WheelEventFilter.h"

// Factory function to create a new window with its own worker
MainWindow* createNewWindow() {
    // Each window needs its own worker and thread
    QThread* workerThread = new QThread();
    CoreWorker* worker = new CoreWorker();
    worker->moveToThread(workerThread);

    QObject::connect(workerThread, &QThread::started, worker, &CoreWorker::init);
    QObject::connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    MainWindow* w = new MainWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->connectWorker(worker);
    
    // When window closes, quit the thread
    QObject::connect(w, &MainWindow::destroyed, workerThread, &QThread::quit);
    
    workerThread->start();
    w->show();
    
    return w;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Install Global Event Filter
    a.installEventFilter(new WheelEventFilter(&a));

    // Init Logging (Global singleton)
    LogManager::instance().init();

    spdlog::info("Modbus-Tools Started. Main Thread: {}", (uint64_t)QThread::currentThreadId());

    // Create the first window
    createNewWindow();

    return a.exec();
}
