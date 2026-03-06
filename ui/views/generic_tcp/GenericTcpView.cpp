#include "GenericTcpView.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../../core/io/GenericIoWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QThread>
#include <QMetaObject>
#include <QEvent>
#include <spdlog/spdlog.h>
#include "../../../core/io/IChannel.h"

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

    // 2. Central Area (Traffic Monitor)
    trafficMonitor_ = new widgets::TrafficMonitorWidget(this);
    mainLayout->addWidget(trafficMonitor_, 1);

    // 3. Input Section (Bottom)
    inputGroup_ = new QGroupBox(this);
    auto inputLayout = new QVBoxLayout(inputGroup_);
    inputWidget_ = new widgets::GenericInputWidget(inputGroup_);
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputGroup_);

    // Connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
            this, &GenericTcpView::onConnectClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked, 
            this, &GenericTcpView::onDisconnectClicked);
    
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested, 
            this, &GenericTcpView::onSendRequested);

    retranslateUi();
}

void GenericTcpView::startWorker() {
    workerThread_ = new QThread(this);
    auto* worker = new io::GenericIoWorker();
    worker->moveToThread(workerThread_);

    connect(worker, &io::GenericIoWorker::stateChanged, this, &GenericTcpView::onWorkerStateChanged);
    connect(worker, &io::GenericIoWorker::errorOccurred, this, &GenericTcpView::onWorkerError);
    connect(worker, &io::GenericIoWorker::monitor, this, &GenericTcpView::onWorkerMonitor);

    workerThread_->start();
    worker_ = worker;
}

void GenericTcpView::stopWorker() {
    if (workerThread_) {
        // Close connection first
        if (worker_) {
            QMetaObject::invokeMethod(worker_, "close", Qt::BlockingQueuedConnection);
            QMetaObject::invokeMethod(worker_, "deleteLater", Qt::QueuedConnection);
            worker_ = nullptr;
        }
        
        workerThread_->quit();
        workerThread_->wait();
        delete workerThread_;
        workerThread_ = nullptr;
    }
}

void GenericTcpView::onConnectClicked(const QString& ip, int port) {
    if (!worker_) return;
    
    spdlog::info("GenericTcp: Connecting to {}:{}", ip.toStdString(), port);
    trafficMonitor_->appendInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    
    // Invoke openTcp on worker thread
    QMetaObject::invokeMethod(worker_, "openTcp", 
                              Qt::QueuedConnection, 
                              Q_ARG(QString, ip), 
                              Q_ARG(int, port));
}

void GenericTcpView::onDisconnectClicked() {
    if (!worker_) return;
    
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericTcpView::onSendRequested(const QByteArray& data) {
    if (!worker_) return;
    
    // Invoke write on worker thread
    QMetaObject::invokeMethod(worker_, "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericTcpView::onWorkerStateChanged(io::ChannelState state) {
    bool connected = (state == io::ChannelState::Open);
    connectionWidget_->setConnected(connected);
    
    QString stateStr;
    switch (state) {
        case io::ChannelState::Closed: stateStr = tr("Closed"); break;
        case io::ChannelState::Opening: stateStr = tr("Opening"); break;
        case io::ChannelState::Open: stateStr = tr("Connected"); break;
        case io::ChannelState::Closing: stateStr = tr("Closing"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }
    
    trafficMonitor_->appendInfo(tr("State changed: %1").arg(stateStr));
    
}

void GenericTcpView::onWorkerError(const QString& error) {
    trafficMonitor_->appendInfo(tr("Error: %1").arg(error));
    spdlog::error("GenericTcp Error: {}", error.toStdString());
}

void GenericTcpView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (isTx) {
        trafficMonitor_->appendTx(data);
    } else {
        trafficMonitor_->appendRx(data);
    }
}

void GenericTcpView::retranslateUi() {
    if (inputGroup_) inputGroup_->setTitle(tr("Send Data"));
}

void GenericTcpView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::generic_tcp
