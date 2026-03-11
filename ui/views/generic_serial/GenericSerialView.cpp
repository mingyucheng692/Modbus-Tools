#include "GenericSerialView.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "../../../core/io/GenericIoWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QThread>
#include <QMetaObject>
#include <QCheckBox>
#include <QEvent>
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
    mainLayout->setSpacing(4);

    // 1. Connection Section (Top)
    auto topLayout = new QHBoxLayout();
    connectionWidget_ = new widgets::SerialConnectionWidget(this);
    topLayout->addWidget(connectionWidget_);
    
    // Serial Controls (DTR/RTS)
    controlGroup_ = new QGroupBox(this);
    auto controlLayout = new QHBoxLayout(controlGroup_);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    
    dtrCheck_ = new QCheckBox(this);
    rtsCheck_ = new QCheckBox(this);
    controlLayout->addWidget(dtrCheck_);
    controlLayout->addWidget(rtsCheck_);
    
    topLayout->addWidget(controlGroup_);
    topLayout->addStretch();
    
    mainLayout->addLayout(topLayout);

    // 2. Central Area (Traffic Monitor)
    trafficMonitor_ = new widgets::TrafficMonitorWidget(this);
    mainLayout->addWidget(trafficMonitor_);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(this);
    inputSection_->setSettingsKey("serial_port/ui/inputCollapsed");
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(inputSection_->contentWidget());
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_);
    mainLayout->addStretch();

    // Connections
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked, 
            this, &GenericSerialView::onConnectClicked);
    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked, 
            this, &GenericSerialView::onDisconnectClicked);
    
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested, 
            this, &GenericSerialView::onSendRequested);
            
    connect(dtrCheck_, &QCheckBox::toggled, this, &GenericSerialView::onDtrChanged);
    connect(rtsCheck_, &QCheckBox::toggled, this, &GenericSerialView::onRtsChanged);
    
    // Disable controls initially
    dtrCheck_->setEnabled(false);
    rtsCheck_->setEnabled(false);

    retranslateUi();
}

void GenericSerialView::startWorker() {
    workerThread_ = new QThread(this);
    auto* worker = new io::GenericIoWorker();
    worker->moveToThread(workerThread_);

    connect(worker, &io::GenericIoWorker::stateChanged, this, &GenericSerialView::onWorkerStateChanged);
    connect(worker, &io::GenericIoWorker::errorOccurred, this, &GenericSerialView::onWorkerError);
    connect(worker, &io::GenericIoWorker::monitor, this, &GenericSerialView::onWorkerMonitor);
    
    workerThread_->start();
    worker_ = worker;
}

void GenericSerialView::stopWorker() {
    if (workerThread_) {
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

void GenericSerialView::onConnectClicked(const io::SerialConfig& config) {
    if (!worker_) return;
    
    spdlog::info("GenericSerial: Connecting to {}", config.portName.toStdString());
    trafficMonitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    
    QMetaObject::invokeMethod(worker_, "openSerial", 
                              Qt::QueuedConnection, 
                              Q_ARG(io::SerialConfig, config));
}

void GenericSerialView::onDisconnectClicked() {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericSerialView::onSendRequested(const QByteArray& data) {
    if (!worker_) return;
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }
    QMetaObject::invokeMethod(worker_, "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericSerialView::onWorkerStateChanged(io::ChannelState state) {
    isConnected_ = (state == io::ChannelState::Open);
    connectionWidget_->setConnected(isConnected_);
    dtrCheck_->setEnabled(isConnected_);
    rtsCheck_->setEnabled(isConnected_);
    
    QString stateStr;
    switch (state) {
        case io::ChannelState::Closed: stateStr = tr("Closed"); break;
        case io::ChannelState::Opening: stateStr = tr("Opening"); break;
        case io::ChannelState::Open: stateStr = tr("Open"); break;
        case io::ChannelState::Closing: stateStr = tr("Closing"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }
    
    trafficMonitor_->appendInfo(tr("State changed: %1").arg(stateStr));
}

void GenericSerialView::onWorkerError(const QString& error) {
    trafficMonitor_->appendInfo(tr("Error: %1").arg(error));
    spdlog::error("GenericSerial Error: {}", error.toStdString());
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
    QMetaObject::invokeMethod(worker_, "setDtr", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::onRtsChanged(bool checked) {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "setRts", 
                              Qt::QueuedConnection, 
                              Q_ARG(bool, checked));
}

void GenericSerialView::retranslateUi() {
    if (controlGroup_) controlGroup_->setTitle(tr("Control"));
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
    if (dtrCheck_) dtrCheck_->setText(tr("DTR"));
    if (rtsCheck_) rtsCheck_->setText(tr("RTS"));
}

void GenericSerialView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::generic_serial
