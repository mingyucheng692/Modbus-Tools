#include "ModbusFactory.h"
#include "../transport/ModbusRtuTransport.h"
#include "../transport/ModbusTcpTransport.h"
#include "../session/ModbusClient.h"
#include "../../io/SerialChannel.h"
#include "../../io/TcpChannel.h"
#include <QMetaObject>
#include <QSerialPort>
#include <QThread>

namespace modbus::factory {

namespace {

void disposeThread(QThread* thread)
{
    if (!thread) {
        return;
    }

    if (thread->isRunning()) {
        QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater, Qt::UniqueConnection);
        return;
    }

    if (thread->thread() == QThread::currentThread()) {
        delete thread;
    } else {
        thread->deleteLater();
    }
}

void disposeWorker(dispatch::ModbusWorker* worker)
{
    if (!worker) {
        return;
    }

    QThread* objectThread = worker->thread();
    if (objectThread && objectThread->isRunning()) {
        QMetaObject::invokeMethod(worker, [worker, objectThread]() {
            QObject::connect(worker, &QObject::destroyed, objectThread, &QThread::quit, Qt::UniqueConnection);
            worker->stop();
            worker->deleteLater();
        }, Qt::QueuedConnection);
        return;
    }

    delete worker;
}

} // namespace

ModbusStack ModbusFactory::createStack(const base::ModbusConfig& config) {
    ModbusStack stack;
    // 1. 创建底层通道 (IO)
    stack.channel = createChannel(config);
    if (!stack.channel) return stack;

    // 2. 创建传输层策略 (Protocol)
    auto transport = createTransport(config);
    if (!transport) return stack;

    // 3. 创建客户端会话 (Session)
    stack.client = std::make_shared<session::ModbusClient>(stack.channel, transport);
    stack.client->setConfig(config);

    // 4. 创建工作线程 (Dispatch)
    QThread* threadRaw = new QThread();
    stack.thread = std::shared_ptr<QThread>(threadRaw, &disposeThread);

    auto workerRaw = new dispatch::ModbusWorker(stack.client, stack.thread.get(), nullptr);
    stack.worker = std::shared_ptr<dispatch::ModbusWorker>(workerRaw, &disposeWorker);
    return stack;
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
        return std::make_shared<transport::ModbusRtuTransport>();
    } else {
        return std::make_shared<transport::ModbusTcpTransport>();
    }
}

} // namespace modbus::factory
