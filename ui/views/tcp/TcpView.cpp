#include "TcpView.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <spdlog/spdlog.h>

namespace ui::views::tcp {

TcpView::TcpView(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TcpView::~TcpView() = default;

void TcpView::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    // 1. Connection Section
    connectionWidget_ = new widgets::TcpConnectionWidget(this);
    mainLayout_->addWidget(connectionWidget_);
    
    // 2. Modbus Functions Section
    functionWidget_ = new widgets::FunctionWidget(this);
    mainLayout_->addWidget(functionWidget_);
    
    // 3. Data Traffic (Placeholder for Phase 3)
    auto trafficLabel = new QLabel("Data Traffic Widget will be here (Phase 3)", this);
    trafficLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    trafficLabel->setAlignment(Qt::AlignCenter);
    trafficLabel->setStyleSheet("background-color: #f0f0f0; min-height: 200px;");
    mainLayout_->addWidget(trafficLabel);
    
    // Connect Signals (Logging for now)
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
        [](const QString& ip, int port) {
            spdlog::info("TcpView: Connect requested to {}:{}", ip.toStdString(), port);
    });

    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
        []() {
            spdlog::info("TcpView: Disconnect requested");
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::readRequested,
        [](uint8_t fc, int addr, int qty) {
            spdlog::info("TcpView: Read FC:0x{:02X} Addr:{} Qty:{}", fc, addr, qty);
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [](uint8_t fc, int addr, const QString& data, const QString& fmt) {
            spdlog::info("TcpView: Write FC:0x{:02X} Addr:{} Data:{} Fmt:{}", 
                         fc, addr, data.toStdString(), fmt.toStdString());
    });
    
    mainLayout_->addStretch();
}

} // namespace ui::views::tcp
