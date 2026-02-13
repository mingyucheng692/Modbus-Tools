#include "MainWindow.h"
#include "views/tcp/TcpView.h"
#include "views/rtu/RtuView.h"
#include <QStackedWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QWidget>
#include <spdlog/spdlog.h>

namespace ui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("Modbus Tools");
    resize(1200, 800);

    // Central widget with HBox layout
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Navigation
    createNavigation();
    mainLayout->addWidget(navigationList_, 1);

    // Content Area
    stackedWidget_ = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget_, 5); // Ratio 1:5

    // Add Views
    stackedWidget_->addWidget(new views::tcp::TcpView(this));
    stackedWidget_->addWidget(new views::rtu::RtuView(this));

    // Connect Navigation
    connect(navigationList_, &QListWidget::currentRowChanged,
            stackedWidget_, &QStackedWidget::setCurrentIndex);
            
    // Set default view
    navigationList_->setCurrentRow(0);
}

void MainWindow::createNavigation() {
    navigationList_ = new QListWidget(this);
    navigationList_->addItem("Modbus TCP");
    navigationList_->addItem("Modbus RTU");
    
    // Style the navigation list a bit
    navigationList_->setStyleSheet(R"(
        QListWidget {
            background-color: #2b2b2b;
            color: #dcdcdc;
            border: none;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 15px;
        }
        QListWidget::item:selected {
            background-color: #3d3d3d;
            border-left: 4px solid #3daee9;
        }
    )");
}

} // namespace ui
