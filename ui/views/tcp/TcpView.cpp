#include "TcpView.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "modbus/dispatch/ModbusWorker.h"
#include "modbus/session/ModbusClient.h"
#include "modbus/transport/TcpTransport.h"
#include "io/TcpChannel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <spdlog/spdlog.h>
#include <thread>
#include <QMetaObject>

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

    // 4. Control Options
    controlWidget_ = new widgets::ControlWidget(this);
    mainLayout_->addWidget(controlWidget_);
    
    // Connect Signals (Logging for now)
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked, 
        [this](const QString& ip, int port) {
            spdlog::info("TcpView: Connect requested to {}:{}", ip.toStdString(), port);
            trafficMonitor_->appendInfo(QString("Connecting to %1:%2...").arg(ip).arg(port));
            
            // Initialize backend
            if (!channel_) {
                channel_ = std::make_shared<io::TcpChannel>();
                channel_->setMonitor([this](bool isTx, const QByteArray& data) {
                    // Monitor callback might be called from any thread
                    QMetaObject::invokeMethod(this, [this, isTx, data]() {
                        if (isTx) trafficMonitor_->appendTx(data);
                        else trafficMonitor_->appendRx(data);
                    });
                });
            }
            
            // Re-create stack to ensure clean state
            auto transport = std::make_shared<modbus::transport::TcpTransport>();
            client_ = std::make_shared<modbus::session::ModbusClient>(channel_, transport);
            worker_ = std::make_shared<modbus::dispatch::ModbusWorker>(client_);
            
            channel_->setEndpoint(ip, port);
            worker_->start();

            // Connect asynchronously to avoid blocking UI during handshake (though TcpChannel::open might block)
            std::thread([this]() {
                if (client_->connect()) {
                    QMetaObject::invokeMethod(this, [this]() {
                        connectionWidget_->setConnected(true);
                        trafficMonitor_->appendInfo("Connected");
                    });
                } else {
                    QMetaObject::invokeMethod(this, [this]() {
                        connectionWidget_->setConnected(false);
                        trafficMonitor_->appendInfo("Connection failed");
                    });
                }
            }).detach();
    });

    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("TcpView: Disconnect requested");
            trafficMonitor_->appendInfo("Disconnected");
            
            if (client_) client_->disconnect();
            if (worker_) worker_->stop();
            
            connectionWidget_->setConnected(false);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::readRequested,
        [this](uint8_t fc, int addr, int qty, int slaveId) {
            if (!worker_) return;
            
            using namespace modbus::base;
            QByteArray data;
            data.resize(4);
            data[0] = (addr >> 8) & 0xFF;
            data[1] = addr & 0xFF;
            data[2] = (qty >> 8) & 0xFF;
            data[3] = qty & 0xFF;
            
            Pdu request(static_cast<FunctionCode>(fc), data);

            trafficMonitor_->appendInfo(QString("Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4")
                .arg(fc).arg(addr).arg(qty).arg(slaveId));

            auto start = std::chrono::steady_clock::now();

            auto future = worker_->submit(request, slaveId);
            std::thread([this, future = std::move(future), start]() mutable {
                auto response = future.get();
                auto end = std::chrono::steady_clock::now();
                auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                QMetaObject::invokeMethod(this, [this, response, rtt]() {
                    controlWidget_->updateStats(true, -1); // TX count
                    controlWidget_->updateStats(false, rtt); // RX count + RTT

                    if (!response.isSuccess) {
                        trafficMonitor_->appendInfo(QString("Error: %1").arg(response.error));
                    } else {
                        trafficMonitor_->appendInfo("Success: Response received");
                        // TODO: Parse data and update UI (Waveform or Table) if needed
                    }
                });
            }).detach();
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this](uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId) {
            if (!worker_) return;

            using namespace modbus::base;
            
            // Simple parsing for now (assuming Hex string or single value)
            QByteArray data;
            
            data.append((char)((addr >> 8) & 0xFF));
            data.append((char)(addr & 0xFF));

            // Hack: Parse input as simple integer for now
            bool ok;
            int value = dataStr.toInt(&ok);
            if (!ok) value = 0; // Fallback

            if (fc == 0x06) {
                data.append((char)((value >> 8) & 0xFF));
                data.append((char)(value & 0xFF));
            } else {
                 // Implement other FCs later
            }
            
            Pdu request(static_cast<FunctionCode>(fc), data);

            trafficMonitor_->appendInfo(QString("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
                .arg(fc).arg(addr).arg(dataStr).arg(slaveId));

            auto start = std::chrono::steady_clock::now();

            auto future = worker_->submit(request, slaveId);
            std::thread([this, future = std::move(future), start]() mutable {
                auto response = future.get();
                auto end = std::chrono::steady_clock::now();
                auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                QMetaObject::invokeMethod(this, [this, response, rtt]() {
                    controlWidget_->updateStats(true, -1);
                    controlWidget_->updateStats(false, rtt);

                    if (!response.isSuccess) {
                        trafficMonitor_->appendInfo(QString("Error: %1").arg(response.error));
                    } else {
                        trafficMonitor_->appendInfo("Success: Write confirmed");
                    }
                });
            }).detach();
    });
    
    connect(controlWidget_, &widgets::ControlWidget::pollRequested,
        [this](uint8_t fc, int addr, int qty) {
            if (!worker_) return;
            
            // Use Slave ID from Function Widget
            int slaveId = functionWidget_->getSlaveId();
            
            using namespace modbus::base;
            QByteArray data;
            data.resize(4);
            data[0] = (addr >> 8) & 0xFF;
            data[1] = addr & 0xFF;
            data[2] = (qty >> 8) & 0xFF;
            data[3] = qty & 0xFF;
            
            Pdu request(static_cast<FunctionCode>(fc), data);
            
            auto start = std::chrono::steady_clock::now();

            auto future = worker_->submit(request, slaveId);
            std::thread([this, future = std::move(future), start]() mutable {
                auto response = future.get();
                auto end = std::chrono::steady_clock::now();
                auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                QMetaObject::invokeMethod(this, [this, response, rtt]() {
                    controlWidget_->updateStats(true, -1);
                    controlWidget_->updateStats(false, rtt);

                    if (!response.isSuccess) {
                        trafficMonitor_->appendInfo(QString("Poll Error: %1").arg(response.error));
                    }
                });
            }).detach();
    });
    
    mainLayout_->addStretch();
}

} // namespace ui::views::tcp
