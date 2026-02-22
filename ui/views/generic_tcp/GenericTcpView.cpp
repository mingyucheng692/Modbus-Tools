#include "GenericTcpView.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/QuickCommandWidget.h"
#include "../../../core/io/GenericIoWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QDebug>
#include <QThread>
#include <QMetaObject>
#include <spdlog/spdlog.h>

namespace ui::views::generic_tcp {

GenericTcpView::GenericTcpView(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    startWorker();
}

GenericTcpView::~GenericTcpView() {
    stopWorker();
}

void GenericTcpView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 1. Connection Section (Top)
    connectionWidget_ = new widgets::TcpConnectionWidget(this);
    connectionWidget_->setDefaultPort(8080); // Set default port for Generic TCP Client
    mainLayout->addWidget(connectionWidget_);

    // 2. Central Area (Splitter: Traffic | QuickCommands)
    auto splitter = new QSplitter(Qt::Horizontal, this);
    
    // Left: Traffic Monitor
    trafficMonitor_ = new widgets::TrafficMonitorWidget(splitter);
    splitter->addWidget(trafficMonitor_);
    
    // Right: Quick Commands (Collapsible via splitter)
    auto quickCmdGroup = new QGroupBox("Quick Commands", splitter);
    auto quickCmdLayout = new QVBoxLayout(quickCmdGroup);
    quickCommandWidget_ = new widgets::QuickCommandWidget(quickCmdGroup);
    quickCmdLayout->addWidget(quickCommandWidget_);
    splitter->addWidget(quickCmdGroup);
    
    // Set initial sizes (Traffic 70%, QuickCmd 30%)
    splitter->setStretchFactor(0, 7);
    splitter->setStretchFactor(1, 3);
    
    mainLayout->addWidget(splitter, 1); // Stretch factor 1 to take available space

    // 3. Input Section (Bottom)
    auto inputGroup = new QGroupBox("Send Data", this);
    auto inputLayout = new QVBoxLayout(inputGroup);
    inputWidget_ = new widgets::GenericInputWidget(inputGroup);
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputGroup);

    // Connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
            this, &GenericTcpView::onConnectClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked, 
            this, &GenericTcpView::onDisconnectClicked);
    
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested, 
            this, &GenericTcpView::onSendRequested);
            
    connect(quickCommandWidget_, &widgets::QuickCommandWidget::sendRequested,
            this, &GenericTcpView::onSendRequested);
}

void GenericTcpView::startWorker() {
    workerThread_ = new QThread(this);
    auto* worker = new io::GenericIoWorker(); // Created, no parent yet (will move to thread)
    worker->moveToThread(workerThread_);

    // Connect signals/slots using queued connection
    // We don't delete worker on thread finish, because unique_ptr owns it.
    // connect(workerThread_, &QThread::finished, worker, &QObject::deleteLater); 
    
    connect(worker, &io::GenericIoWorker::stateChanged, this, &GenericTcpView::onWorkerStateChanged);
    connect(worker, &io::GenericIoWorker::errorOccurred, this, &GenericTcpView::onWorkerError);
    connect(worker, &io::GenericIoWorker::dataReceived, this, &GenericTcpView::onWorkerDataReceived);
    connect(worker, &io::GenericIoWorker::monitor, this, &GenericTcpView::onWorkerMonitor);
    connect(worker, &io::GenericIoWorker::bytesWritten, this, &GenericTcpView::onWorkerBytesWritten);

    workerThread_->start();
    
    worker_.reset(worker);
}

void GenericTcpView::stopWorker() {
    if (workerThread_) {
        // Close connection first
        if (worker_) {
            // Invoke close on worker thread
            QMetaObject::invokeMethod(worker_.get(), "close", Qt::BlockingQueuedConnection);
        }
        
        workerThread_->quit();
        workerThread_->wait();
        delete workerThread_;
        workerThread_ = nullptr;
    }
    // worker_ (unique_ptr) will be destroyed here.
}

void GenericTcpView::onConnectClicked(const QString& ip, int port) {
    if (!worker_) return;
    
    spdlog::info("GenericTcp: Connecting to {}:{}", ip.toStdString(), port);
    trafficMonitor_->appendInfo(QString("Connecting to %1:%2...").arg(ip).arg(port));
    
    // Invoke openTcp on worker thread
    QMetaObject::invokeMethod(worker_.get(), "openTcp", 
                              Qt::QueuedConnection, 
                              Q_ARG(QString, ip), 
                              Q_ARG(int, port));
}

void GenericTcpView::onDisconnectClicked() {
    if (!worker_) return;
    
    QMetaObject::invokeMethod(worker_.get(), "close", Qt::QueuedConnection);
}

void GenericTcpView::onSendRequested(const QByteArray& data) {
    if (!worker_) return;
    
    // Invoke write on worker thread
    QMetaObject::invokeMethod(worker_.get(), "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericTcpView::onWorkerStateChanged(int state) {
    // Map int back to ChannelState? Or just use int.
    // 0=Closed, 1=Opening, 2=Open, 3=Closing, 4=Error
    
    bool connected = (state == 2); // Open
    connectionWidget_->setConnected(connected);
    
    QString stateStr;
    switch (state) {
        case 0: stateStr = "Closed"; break;
        case 1: stateStr = "Opening"; break;
        case 2: stateStr = "Connected"; break;
        case 3: stateStr = "Closing"; break;
        case 4: stateStr = "Error"; break;
        default: stateStr = "Unknown"; break;
    }
    
    trafficMonitor_->appendInfo(QString("State changed: %1").arg(stateStr));
    
    if (state == 4) { // Error
        // Maybe show error in status bar?
    }
}

void GenericTcpView::onWorkerError(const QString& error) {
    trafficMonitor_->appendInfo(QString("Error: %1").arg(error));
    spdlog::error("GenericTcp Error: {}", error.toStdString());
}

void GenericTcpView::onWorkerDataReceived(const QByteArray& data) {
    // TrafficMonitor handles display (handled via onWorkerMonitor usually?)
    // Wait, onWorkerMonitor handles both TX and RX for logging.
    // dataReceived is for processing.
    // Since this is a raw view, we might just rely on monitor?
    // But monitor gives raw bytes.
    // Let's use monitor for display to be consistent.
    
    // But wait, TrafficMonitor has appendRx/appendTx.
    // monitor signal gives isTx bool.
}

void GenericTcpView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (isTx) {
        trafficMonitor_->appendTx(data);
    } else {
        trafficMonitor_->appendRx(data);
    }
}

void GenericTcpView::onWorkerBytesWritten(qint64 bytes) {
    // Update stats if needed
}

} // namespace ui::views::generic_tcp
