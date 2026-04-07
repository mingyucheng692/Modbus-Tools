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
    stack.thread = std::shared_ptr<QThread>(new QThread(), [](QThread* thread) {
        if (thread) {
            if (thread->isRunning()) {
                thread->requestInterruption();
                thread->quit();
                thread->wait(); // 强制确定性等待线程结束，防止悬挂
            }
            delete thread; // 纯 C++ RAII 释放
        }
    });

    stack.channel->moveToThread(stack.thread.get());

    auto workerRaw = new dispatch::ModbusWorker(stack.client, stack.thread.get(), nullptr);
    stack.worker = std::shared_ptr<dispatch::ModbusWorker>(workerRaw, [](dispatch::ModbusWorker* worker) {
        if (worker) {
            worker->stop(); // 发起停止请求
            // 为了安全地 delete 一个生活在其他线程的 QObject，我们先把它 move 回当前执行析构的线程
            // 这样直接 delete worker 就不会报 QObject 跨线程销毁的错误了。
            // 如果所属的 QThread 正在运行，可以通过 invokeMethod 将其 move 出来
            QThread* currentThread = QThread::currentThread();
            QThread* objectThread = worker->thread();
            if (objectThread && objectThread != currentThread && objectThread->isRunning()) {
                // 阻塞调用 moveToThread
                QMetaObject::invokeMethod(worker, [worker, currentThread]() {
                    worker->moveToThread(currentThread);
                }, Qt::BlockingQueuedConnection);
            } else if (objectThread && objectThread != currentThread) {
                // 线程已死，直接强制转移（极少发生）
                worker->moveToThread(currentThread);
            }
            delete worker; // 确定性 RAII 释放，不依赖事件循环的 deleteLater
        }
    });
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
