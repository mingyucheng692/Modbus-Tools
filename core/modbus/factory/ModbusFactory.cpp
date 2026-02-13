#include "ModbusFactory.h"
#include "../transport/RtuTransport.h"
#include "../transport/TcpTransport.h"
#include "../session/ModbusClient.h"
#include "../../io/SerialChannel.h"
#include "../../io/TcpChannel.h"
#include <QSerialPort>

namespace modbus::factory {

std::unique_ptr<dispatch::ModbusWorker> ModbusFactory::createWorker(const base::ModbusConfig& config) {
    // 1. 创建底层通道 (IO)
    auto channel = createChannel(config);
    if (!channel) return nullptr;

    // 2. 创建传输层策略 (Protocol)
    auto transport = createTransport(config);
    if (!transport) return nullptr;

    // 3. 创建客户端会话 (Session)
    auto client = std::make_shared<session::ModbusClient>(channel, transport);
    client->setConfig(config);

    // 4. 创建工作线程 (Dispatch)
    return std::make_unique<dispatch::ModbusWorker>(client);
}

std::shared_ptr<io::IChannel> ModbusFactory::createChannel(const base::ModbusConfig& config) {
    if (config.mode == base::ModbusMode::RTU) {
        auto serial = std::make_shared<io::SerialChannel>();
        io::SerialConfig serialConfig;
        serialConfig.portName = config.portName;
        serialConfig.baudRate = config.baudRate;
        serialConfig.dataBits = static_cast<QSerialPort::DataBits>(config.dataBits);
        serialConfig.stopBits = static_cast<QSerialPort::StopBits>(config.stopBits);
        serialConfig.parity = static_cast<QSerialPort::Parity>(config.parity);
        serial->setConfig(serialConfig);
        return serial;
    } else {
        auto tcp = std::make_shared<io::TcpChannel>();
        tcp->setEndpoint(config.ipAddress, config.port);
        return tcp;
    }
}

std::shared_ptr<transport::ITransport> ModbusFactory::createTransport(const base::ModbusConfig& config) {
    if (config.mode == base::ModbusMode::RTU) {
        return std::make_shared<transport::RtuTransport>();
    } else {
        return std::make_shared<transport::TcpTransport>();
    }
}

} // namespace modbus::factory
