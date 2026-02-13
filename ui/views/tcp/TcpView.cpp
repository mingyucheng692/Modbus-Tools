#include "TcpView.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
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
    
    // 3. Data Traffic
    trafficMonitor_ = new widgets::TrafficMonitorWidget(this);
    trafficMonitor_->setMinimumHeight(200);
    mainLayout_->addWidget(trafficMonitor_);
    
    // Connect Signals (Logging for now)
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
        [this](const QString& ip, int port) {
            spdlog::info("TcpView: Connect requested to {}:{}", ip.toStdString(), port);
            trafficMonitor_->appendInfo(QString("Connecting to %1:%2...").arg(ip).arg(port));
            // TODO: Implement actual connection logic
            connectionWidget_->setConnected(true); // Simulate success
            trafficMonitor_->appendInfo("Connected (Simulated)");
    });

    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("TcpView: Disconnect requested");
            trafficMonitor_->appendInfo("Disconnected");
            connectionWidget_->setConnected(false);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::readRequested,
        [this](uint8_t fc, int addr, int qty) {
            spdlog::info("TcpView: Read FC:0x{:02X} Addr:{} Qty:{}", fc, addr, qty);
            // Simulate TX frame: TransID(2) + Proto(2) + Len(2) + Unit(1) + FC(1) + Addr(2) + Qty(2)
            QByteArray frame;
            frame.append("\x00\x01\x00\x00\x00\x06\x01", 7);
            frame.append((char)fc);
            frame.append((char)(addr >> 8));
            frame.append((char)(addr & 0xFF));
            frame.append((char)(qty >> 8));
            frame.append((char)(qty & 0xFF));
            trafficMonitor_->appendTx(frame);
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this](uint8_t fc, int addr, const QString& data, const QString& fmt) {
            spdlog::info("TcpView: Write FC:0x{:02X} Addr:{} Data:{} Fmt:{}", 
                         fc, addr, data.toStdString(), fmt.toStdString());
             // Simulate TX frame for write
            QByteArray frame;
            frame.append("\x00\x02\x00\x00\x00\x06\x01", 7); // Dummy header
            frame.append((char)fc);
            frame.append((char)(addr >> 8));
            frame.append((char)(addr & 0xFF));
            // Data is simplified here
            trafficMonitor_->appendTx(frame);
            trafficMonitor_->appendInfo(QString("Write Request: %1").arg(data));
    });
    
    mainLayout_->addStretch();
}

} // namespace ui::views::tcp
