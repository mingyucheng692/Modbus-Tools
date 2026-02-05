#include "MainWindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QLabel>
#include <QStyleFactory>
#include "Widgets/LogWidget.h"
#include "Widgets/ConnectionDock.h"
#include "Widgets/TrafficWidget.h"
#include "Widgets/WaveformWidget.h"
#include "Core/Logging/TrafficLog.h"

#include "Core/CoreWorker.h"
#include "Core/Modbus/ModbusTcpClient.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    applyDarkTheme();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    setWindowTitle("Modbus-Tools (Re-Impl)");
    resize(1200, 800);
    
    // Central Widget (MDI Area placeholder)
    frameAnalyzer_ = new FrameAnalyzer(this);
    setCentralWidget(frameAnalyzer_);

    createActions();
    createDocks();
    
    statusLabel_ = new QLabel("Ready");
    rttLabel_ = new QLabel("RTT: -");
    rttLabel_->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    
    statusBar()->addWidget(statusLabel_, 1);
    statusBar()->addPermanentWidget(rttLabel_);
}

void MainWindow::connectWorker(CoreWorker* worker) {
    connect(this, &MainWindow::requestConnectTcp, worker, &CoreWorker::connectTcp);
    connect(this, &MainWindow::requestDisconnect, worker, &CoreWorker::disconnect);
    connect(this, &MainWindow::requestSend, worker, &CoreWorker::sendRequest);
    connect(this, &MainWindow::pollToggled, worker, &CoreWorker::setPolling);
    connect(this, &MainWindow::requestSimulation, worker, &CoreWorker::setSimulation);
    
    connect(worker, &CoreWorker::workerReady, this, &MainWindow::onWorkerReady);
    if (worker->modbusClient()) {
        connect(worker->modbusClient(), &Modbus::ModbusTcpClient::responseReceived, this, &MainWindow::onResponseReceived);
    }
}

void MainWindow::onWorkerReady() {
    statusLabel_->setText("Core Worker Ready");
}

void MainWindow::createDocks() {
    // Left Dock: Connection
    connectionDock_ = new QDockWidget("Connection", this);
    connectionDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    ConnectionDock* connWidget = new ConnectionDock(connectionDock_);
    connect(connWidget, &ConnectionDock::connectTcp, this, &MainWindow::requestConnectTcp);
    connect(connWidget, &ConnectionDock::disconnectRequested, this, &MainWindow::requestDisconnect);
    connect(connWidget, &ConnectionDock::sendRequest, this, &MainWindow::requestSend);
    connect(connWidget, &ConnectionDock::pollToggled, this, &MainWindow::pollToggled);
    connect(connWidget, &ConnectionDock::simulationChanged, this, &MainWindow::requestSimulation);
    
    connectionDock_->setWidget(connWidget);
    addDockWidget(Qt::LeftDockWidgetArea, connectionDock_);

    // Bottom Dock: Logs
    logDock_ = new QDockWidget("Logs / Console", this);
    logDock_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    LogWidget* logWidget = new LogWidget(logDock_);
    logDock_->setWidget(logWidget);
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);
    
    // Traffic Dock
    trafficDock_ = new QDockWidget("Traffic Monitor", this);
    trafficDock_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    TrafficWidget* trafficWidget = new TrafficWidget(trafficDock_);
    trafficDock_->setWidget(trafficWidget);
    addDockWidget(Qt::BottomDockWidgetArea, trafficDock_);
    
    connect(trafficWidget, &TrafficWidget::frameSelected, [this](size_t index){
        const auto& frame = TrafficLog::instance().getFrame(index);
        frameAnalyzer_->analyze(frame);
    });
    
    tabifyDockWidget(logDock_, trafficDock_);
    
    // View Menu to toggle docks
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(connectionDock_->toggleViewAction());
    viewMenu->addAction(logDock_->toggleViewAction());
    viewMenu->addAction(trafficDock_->toggleViewAction());

    // Waveform Dock
    waveformDock_ = new QDockWidget("Waveform Monitor", this);
    waveformDock_->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    waveformWidget_ = new WaveformWidget(this);
    waveformDock_->setWidget(waveformWidget_);
    addDockWidget(Qt::RightDockWidgetArea, waveformDock_);
    
    viewMenu->addAction(waveformDock_->toggleViewAction());
}

void MainWindow::onResponseReceived(uint16_t transactionId, uint8_t unitId, Modbus::FunctionCode fc, const std::vector<uint8_t>& data, int responseTimeMs, uint16_t startAddr) {
    // Update RTT
    if (responseTimeMs >= 0) {
        rttLabel_->setText(QString("RTT: %1 ms").arg(responseTimeMs));
    } else {
        rttLabel_->setText("RTT: Timeout");
    }

    if (fc == Modbus::FunctionCode::ReadHoldingRegisters || fc == Modbus::FunctionCode::ReadInputRegisters) {
        if (data.size() >= 2) { 
             uint16_t val = (data[0] << 8) | data[1];
             waveformWidget_->setMonitoredAddress(startAddr); 
             waveformWidget_->addValue(startAddr, static_cast<double>(val));
        }
    }
}

void MainWindow::createActions() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("E&xit", qApp, &QApplication::quit);
}

void MainWindow::applyDarkTheme() {
    // Fusion style with Dark Palette
    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    QPalette p = qApp->palette();
    p.setColor(QPalette::Window, QColor(53, 53, 53));
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, QColor(25, 25, 25));
    p.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    p.setColor(QPalette::ToolTipBase, Qt::white);
    p.setColor(QPalette::ToolTipText, Qt::white);
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Button, QColor(53, 53, 53));
    p.setColor(QPalette::ButtonText, Qt::white);
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Link, QColor(42, 130, 218));
    p.setColor(QPalette::Highlight, QColor(42, 130, 218));
    p.setColor(QPalette::HighlightedText, Qt::black);
    
    qApp->setPalette(p);
    
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}
