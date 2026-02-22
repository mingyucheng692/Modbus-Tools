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
#include <QRegularExpression>
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
            
            QByteArray data;
            
            data.append((char)((addr >> 8) & 0xFF));
            data.append((char)(addr & 0xFF));

            QString trimmed = dataStr.trimmed();
            auto parseHexBytes = [](const QString& input) {
                QString cleaned = input;
                cleaned.remove(QRegularExpression("[^0-9A-Fa-f]"));
                if (cleaned.isEmpty()) return QByteArray();
                if (cleaned.size() % 2 == 1) cleaned.prepend("0");
                return QByteArray::fromHex(cleaned.toLatin1());
            };
            auto parseDecimalList = [](const QString& input, bool& okOut) {
                okOut = true;
                QList<quint16> values;
                QStringList parts = input.split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
                if (parts.isEmpty()) {
                    okOut = false;
                    return values;
                }
                for (const auto& part : parts) {
                    bool ok = false;
                    uint value = part.toUInt(&ok, 10);
                    if (!ok || value > 65535) {
                        okOut = false;
                        return QList<quint16>();
                    }
                    values.append(static_cast<quint16>(value));
                }
                return values;
            };

            if (fc == 0x05) {
                bool coilOn = false;
                if (fmt == "Decimal") {
                    bool ok = false;
                    int value = trimmed.toInt(&ok);
                    if (!ok) {
                        trafficMonitor_->appendInfo("Error: Invalid decimal value for 0x05");
                        return;
                    }
                    coilOn = (value != 0);
                } else if (fmt == "Hex") {
                    QByteArray bytes = parseHexBytes(trimmed);
                    if (bytes.isEmpty()) {
                        trafficMonitor_->appendInfo("Error: Invalid hex value for 0x05");
                        return;
                    }
                    if (bytes.size() == 1) {
                        coilOn = static_cast<uint8_t>(bytes[0]) != 0;
                    } else {
                        uint16_t raw = (static_cast<uint8_t>(bytes[0]) << 8) | static_cast<uint8_t>(bytes[1]);
                        coilOn = raw != 0;
                    }
                } else {
                    QString upper = trimmed.toUpper();
                    if (upper == "ON" || upper == "TRUE") {
                        coilOn = true;
                    } else if (upper == "OFF" || upper == "FALSE") {
                        coilOn = false;
                    } else {
                        bool ok = false;
                        int value = trimmed.toInt(&ok);
                        if (!ok) {
                            trafficMonitor_->appendInfo("Error: Invalid ASCII value for 0x05");
                            return;
                        }
                        coilOn = (value != 0);
                    }
                }
                data.append(static_cast<char>(coilOn ? 0xFF : 0x00));
                data.append(static_cast<char>(0x00));
            } else if (fc == 0x06) {
                if (trimmed.isEmpty()) {
                    trafficMonitor_->appendInfo("Error: Empty value for 0x06");
                    return;
                }
                if (fmt == "Decimal") {
                    bool ok = false;
                    uint value = trimmed.toUInt(&ok, 10);
                    if (!ok || value > 65535) {
                        trafficMonitor_->appendInfo("Error: Invalid decimal value for 0x06");
                        return;
                    }
                    data.append(static_cast<char>((value >> 8) & 0xFF));
                    data.append(static_cast<char>(value & 0xFF));
                } else if (fmt == "Hex") {
                    QByteArray bytes = parseHexBytes(trimmed);
                    if (bytes.isEmpty()) {
                        trafficMonitor_->appendInfo("Error: Invalid hex value for 0x06");
                        return;
                    }
                    if (bytes.size() == 1) {
                        data.append(static_cast<char>(0x00));
                        data.append(bytes[0]);
                    } else {
                        data.append(bytes[0]);
                        data.append(bytes[1]);
                    }
                } else {
                    QByteArray bytes = trimmed.toLatin1();
                    if (bytes.isEmpty()) {
                        trafficMonitor_->appendInfo("Error: Invalid ASCII value for 0x06");
                        return;
                    }
                    if (bytes.size() == 1) {
                        bytes.prepend(static_cast<char>(0x00));
                    }
                    data.append(bytes[0]);
                    data.append(bytes[1]);
                }
            } else if (fc == 0x0F) {
                if (fmt != "Hex") {
                    trafficMonitor_->appendInfo("Error: 0x0F requires Hex data");
                    return;
                }
                QByteArray bytes = parseHexBytes(trimmed);
                if (bytes.isEmpty()) {
                    trafficMonitor_->appendInfo("Error: Invalid hex value for 0x0F");
                    return;
                }
                int quantity = functionWidget_->getQuantity();
                if (quantity <= 0) {
                    trafficMonitor_->appendInfo("Error: Invalid quantity for 0x0F");
                    return;
                }
                int byteCount = (quantity + 7) / 8;
                if (bytes.size() < byteCount) {
                    trafficMonitor_->appendInfo("Error: Coil data length mismatch for 0x0F");
                    return;
                }
                if (bytes.size() > byteCount) {
                    bytes = bytes.left(byteCount);
                }
                data.append(static_cast<char>((quantity >> 8) & 0xFF));
                data.append(static_cast<char>(quantity & 0xFF));
                data.append(static_cast<char>(byteCount));
                data.append(bytes);
            } else if (fc == 0x10) {
                if (trimmed.isEmpty()) {
                    trafficMonitor_->appendInfo("Error: Empty value for 0x10");
                    return;
                }
                int quantity = functionWidget_->getQuantity();
                if (quantity <= 0) {
                    trafficMonitor_->appendInfo("Error: Invalid quantity for 0x10");
                    return;
                }
                QByteArray payload;
                int registerCount = 0;
                if (fmt == "Decimal") {
                    bool okList = false;
                    auto values = parseDecimalList(trimmed, okList);
                    if (!okList) {
                        trafficMonitor_->appendInfo("Error: Invalid decimal list for 0x10");
                        return;
                    }
                    registerCount = values.size();
                    for (quint16 value : values) {
                        payload.append(static_cast<char>((value >> 8) & 0xFF));
                        payload.append(static_cast<char>(value & 0xFF));
                    }
                } else if (fmt == "Hex") {
                    payload = parseHexBytes(trimmed);
                    if (payload.isEmpty() || (payload.size() % 2 != 0)) {
                        trafficMonitor_->appendInfo("Error: Invalid hex value for 0x10");
                        return;
                    }
                    registerCount = payload.size() / 2;
                } else {
                    payload = trimmed.toLatin1();
                    if (payload.isEmpty()) {
                        trafficMonitor_->appendInfo("Error: Invalid ASCII value for 0x10");
                        return;
                    }
                    if (payload.size() % 2 == 1) {
                        payload.prepend(static_cast<char>(0x00));
                    }
                    registerCount = payload.size() / 2;
                }
                if (registerCount != quantity) {
                    trafficMonitor_->appendInfo("Error: Quantity does not match data length for 0x10");
                    return;
                }
                data.append(static_cast<char>((quantity >> 8) & 0xFF));
                data.append(static_cast<char>(quantity & 0xFF));
                data.append(static_cast<char>(registerCount * 2));
                data.append(payload);
            } else {
                trafficMonitor_->appendInfo("Error: Unsupported write function code");
                return;
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
    
    connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
        [this](const QByteArray& data) {
            if (!worker_) return;
            
            trafficMonitor_->appendInfo(QString("Sending Raw Data: %1").arg(QString(data.toHex(' ').toUpper())));
            
            worker_->sendRaw(data);
            
            controlWidget_->updateStats(true, -1);
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
