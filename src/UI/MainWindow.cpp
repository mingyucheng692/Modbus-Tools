#include "MainWindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QLabel>
#include <QStyleFactory>
#include "Widgets/LogWidget.h"
#include "Widgets/ConnectionDock.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    initUI();
    applyDarkTheme();
}

MainWindow::~MainWindow() {}

void MainWindow::initUI() {
    setWindowTitle("Modbus-Tools (Re-Impl)");
    resize(1200, 800);
    
    // Central Widget (MDI Area placeholder)
    QLabel* centralLabel = new QLabel("MDI Area / Monitor", this);
    centralLabel->setAlignment(Qt::AlignCenter);
    setCentralWidget(centralLabel);

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
    
    connectionDock_->setWidget(connWidget);
    addDockWidget(Qt::LeftDockWidgetArea, connectionDock_);

    // Bottom Dock: Logs
    logDock_ = new QDockWidget("Logs / Console", this);
    logDock_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    LogWidget* logWidget = new LogWidget(logDock_);
    logDock_->setWidget(logWidget);
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);
    
    // View Menu to toggle docks
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(connectionDock_->toggleViewAction());
    viewMenu->addAction(logDock_->toggleViewAction());
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
