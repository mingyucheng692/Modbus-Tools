#include "MainWindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QLabel>
#include <QStyleFactory>
#include "Widgets/LogWidget.h"
#include "Widgets/ConnectionDock.h"
#include "Widgets/TrafficWidget.h"
#include "Core/Logging/TrafficLog.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    initUI();
    applyDarkTheme();
}

MainWindow::~MainWindow() {}

void MainWindow::initUI() {
    setWindowTitle("Modbus-Tools (Re-Impl)");
    resize(1200, 800);
    
    // Central Widget (MDI Area placeholder)
    analyzer_ = new FrameAnalyzer(this);
    setCentralWidget(analyzer_);

    createActions();
    createDocks();
    
    statusBar()->showMessage("Ready");
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
        analyzer_->analyze(frame);
    });
    
    tabifyDockWidget(logDock_, trafficDock_);
    
    // View Menu to toggle docks
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(connectionDock_->toggleViewAction());
    viewMenu->addAction(logDock_->toggleViewAction());
    viewMenu->addAction(trafficDock_->toggleViewAction());
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
