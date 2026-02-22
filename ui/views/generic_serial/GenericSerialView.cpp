#include "GenericSerialView.h"
#include "../../widgets/SerialConnectionWidget.h"
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
#include <QCheckBox>
#include <spdlog/spdlog.h>

namespace ui::views::generic_serial {

GenericSerialView::GenericSerialView(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    startWorker();
}

GenericSerialView::~GenericSerialView() {
    stopWorker();
}

void GenericSerialView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 1. Connection Section (Top)
    auto topLayout = new QHBoxLayout();
    connectionWidget_ = new widgets::SerialConnectionWidget(this);
    topLayout->addWidget(connectionWidget_);
    
    // Serial Controls (DTR/RTS)
    auto controlGroup = new QGroupBox("Control", this);
    auto controlLayout = new QHBoxLayout(controlGroup);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    
    dtrCheck_ = new QCheckBox("DTR", this);
    rtsCheck_ = new QCheckBox("RTS", this);
    controlLayout->addWidget(dtrCheck_);
    controlLayout->addWidget(rtsCheck_);
    
    topLayout->addWidget(controlGroup);
    topLayout->addStretch();
    
    mainLayout->addLayout(topLayout);

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
    
    mainLayout->addWidget(splitter, 1);

    // 3. Input Section (Bottom)
    auto inputGroup = new QGroupBox("Send Data", this);
    auto inputLayout = new QVBoxLayout(inputGroup);
    inputWidget_ = new widgets::GenericInputWidget(inputGroup);
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputGroup);

    // Connections
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked, 
            this, &GenericSerialView::onConnectClicked);
    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked, 
            this, &GenericSerialView::onDisconnectClicked);
    
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested, 
            this, &GenericSerialView::onSendRequested);
            
    connect(quickCommandWidget_, &widgets::QuickCommandWidget::sendRequested,
            this, &GenericSerialView::onSendRequested);
            
    connect(dtrCheck_, &QCheckBox::toggled, this, &GenericSerialView::onDtrChanged);
    connect(rtsCheck_, &QCheckBox::toggled, this, &GenericSerialView::onRtsChanged);
    
    // Disable controls initially
    dtrCheck_->setEnabled(false);
    rtsCheck_->setEnabled(false);
}

void GenericSerialView::startWorker() {
    workerThread_ = new QThread(this);
    auto* worker = new io::GenericIoWorker();
    worker->moveToThread(workerThread_);

    connect(worker, &io::GenericIoWorker::stateChanged, this, &GenericSerialView::onWorkerStateChanged);
    connect(worker, &io::GenericIoWorker::errorOccurred, this, &GenericSerialView::onWorkerError);
    connect(worker, &io::GenericIoWorker::dataReceived, this, &GenericSerialView::onWorkerDataReceived);
    connect(worker, &io::GenericIoWorker::monitor, this, &GenericSerialView::onWorkerMonitor);
    connect(worker, &io::GenericIoWorker::bytesWritten, [](qint64){}); // Ignored
    
    workerThread_->start();
    worker_.reset(worker);
}

void GenericSerialView::stopWorker() {
    if (workerThread_) {
        if (worker_) {
            QMetaObject::invokeMethod(worker_.get(), "close", Qt::BlockingQueuedConnection);
        }
        workerThread_->quit();
        workerThread_->wait();
        delete workerThread_;
        workerThread_ = nullptr;
    }
}

void GenericSerialView::onConnectClicked(const io::SerialConfig& config) {
    if (!worker_) return;
    
    spdlog::info("GenericSerial: Connecting to {}", config.portName.toStdString());
    trafficMonitor_->appendInfo(QString("Opening %1...").arg(config.portName));
    
    QMetaObject::invokeMethod(worker_.get(), "openSerial", 
                              Qt::QueuedConnection, 
                              Q_ARG(io::SerialConfig, config));
}

void GenericSerialView::onDisconnectClicked() {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_.get(), "close", Qt::QueuedConnection);
}

void GenericSerialView::onSendRequested(const QByteArray& data) {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_.get(), "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericSerialView::onWorkerStateChanged(int state) {
    bool connected = (state == 2); // Open
    connectionWidget_->setConnected(connected);
    dtrCheck_->setEnabled(connected);
    rtsCheck_->setEnabled(connected);
    
    QString stateStr;
    switch (state) {
        case 0: stateStr = "Closed"; break;
        case 1: stateStr = "Opening"; break;
        case 2: stateStr = "Open"; break;
        case 3: stateStr = "Closing"; break;
        case 4: stateStr = "Error"; break;
        default: stateStr = "Unknown"; break;
    }
    
    trafficMonitor_->appendInfo(QString("State changed: %1").arg(stateStr));
}

void GenericSerialView::onWorkerError(const QString& error) {
    trafficMonitor_->appendInfo(QString("Error: %1").arg(error));
    spdlog::error("GenericSerial Error: {}", error.toStdString());
}

void GenericSerialView::onWorkerDataReceived(const QByteArray& data) {
    // Handled by monitor for display
}

void GenericSerialView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (isTx) {
        trafficMonitor_->appendTx(data);
    } else {
        trafficMonitor_->appendRx(data);
    }
}

void GenericSerialView::onDtrChanged(bool checked) {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_.get(), "setDtr", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::onRtsChanged(bool checked) {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_.get(), "setRts", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

} // namespace ui::views::generic_serial
