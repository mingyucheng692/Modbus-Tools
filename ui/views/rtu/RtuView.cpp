#include "RtuView.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "modbus/dispatch/ModbusWorker.h"
#include "modbus/session/ModbusClient.h"
#include "modbus/transport/RtuTransport.h"
#include "io/SerialChannel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <spdlog/spdlog.h>
#include <thread>
#include <QMetaObject>

namespace ui::views::rtu {

RtuView::RtuView(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

RtuView::~RtuView() {
    if (client_) client_->disconnect();
    if (worker_) worker_->stop();
}

void RtuView::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    // 1. Connection
    connectionWidget_ = new widgets::SerialConnectionWidget(this);
    mainLayout_->addWidget(connectionWidget_);
    
    // 2. Functions
    functionWidget_ = new widgets::FunctionWidget(this);
    mainLayout_->addWidget(functionWidget_);
    
    // 3. Traffic
    trafficMonitor_ = new widgets::TrafficMonitorWidget(this);
    trafficMonitor_->setMinimumHeight(200);
    mainLayout_->addWidget(trafficMonitor_);
    
    // Connect Signals
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked, 
        [this](const io::SerialConfig& config) {
            spdlog::info("RtuView: Connect requested to {}", config.portName.toStdString());
            trafficMonitor_->appendInfo(QString("Opening %1...").arg(config.portName));
            
            if (!channel_) {
                channel_ = std::make_shared<io::SerialChannel>();
                channel_->setMonitor([this](bool isTx, const QByteArray& data) {
                    QMetaObject::invokeMethod(this, [this, isTx, data]() {
                        if (isTx) trafficMonitor_->appendTx(data);
                        else trafficMonitor_->appendRx(data);
                    });
                });
            }
            
            auto transport = std::make_shared<modbus::transport::RtuTransport>();
            client_ = std::make_shared<modbus::session::ModbusClient>(channel_, transport);
            worker_ = std::make_shared<modbus::dispatch::ModbusWorker>(client_);
            
            channel_->setConfig(config);
            worker_->start();

            std::thread([this]() {
                if (client_->connect()) {
                    QMetaObject::invokeMethod(this, [this]() {
                        connectionWidget_->setConnected(true);
                        trafficMonitor_->appendInfo("Port Opened");
                    });
                } else {
                    QMetaObject::invokeMethod(this, [this]() {
                        connectionWidget_->setConnected(false);
                        trafficMonitor_->appendInfo("Failed to open port");
                    });
                }
            }).detach();
    });

    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("RtuView: Disconnect requested");
            trafficMonitor_->appendInfo("Closed");
            
            if (client_) client_->disconnect();
            if (worker_) worker_->stop();
            
            connectionWidget_->setConnected(false);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::readRequested,
        [this](uint8_t fc, int addr, int qty) {
            if (!worker_) return;
            
            using namespace modbus::base;
            QByteArray data;
            data.resize(4);
            data[0] = (addr >> 8) & 0xFF;
            data[1] = addr & 0xFF;
            data[2] = (qty >> 8) & 0xFF;
            data[3] = qty & 0xFF;
            
            Pdu request(static_cast<FunctionCode>(fc), data);

            trafficMonitor_->appendInfo(QString("Sending Read Request FC:%1 Addr:%2 Qty:%3").arg(fc).arg(addr).arg(qty));

            auto future = worker_->submit(request);
            std::thread([this, future = std::move(future)]() mutable {
                auto response = future.get();
                QMetaObject::invokeMethod(this, [this, response]() {
                    if (!response.isSuccess) {
                        trafficMonitor_->appendInfo(QString("Error: %1").arg(response.error));
                    } else {
                        trafficMonitor_->appendInfo("Success: Response received");
                    }
                });
            }).detach();
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this](uint8_t fc, int addr, const QString& dataStr, const QString& fmt) {
            if (!worker_) return;

            using namespace modbus::base;
            
            QByteArray data;
            data.append((char)((addr >> 8) & 0xFF));
            data.append((char)(addr & 0xFF));

            bool ok;
            int value = dataStr.toInt(&ok);
            if (!ok) value = 0;

            if (fc == 0x06) {
                data.append((char)((value >> 8) & 0xFF));
                data.append((char)(value & 0xFF));
            } else {
                 // Implement other FCs later
            }
            
            Pdu request(static_cast<FunctionCode>(fc), data);

            trafficMonitor_->appendInfo(QString("Sending Write Request FC:%1 Addr:%2 Data:%3").arg(fc).arg(addr).arg(dataStr));

            auto future = worker_->submit(request);
            std::thread([this, future = std::move(future)]() mutable {
                auto response = future.get();
                QMetaObject::invokeMethod(this, [this, response]() {
                    if (!response.isSuccess) {
                        trafficMonitor_->appendInfo(QString("Error: %1").arg(response.error));
                    } else {
                        trafficMonitor_->appendInfo("Success: Write confirmed");
                    }
                });
            }).detach();
    });
    
    mainLayout_->addStretch();
}

} // namespace ui::views::rtu
