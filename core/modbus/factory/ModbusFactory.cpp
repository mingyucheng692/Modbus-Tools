/**
 * @file ModbusFactory.cpp
 * @brief Implementation of ModbusFactory.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusFactory.h"
#include "../transport/ModbusRtuTransport.h"
#include "../transport/ModbusTcpTransport.h"
#include "../session/ModbusClient.h"
#include "infra/io/SerialChannel.h"
#include "infra/io/TcpChannel.h"
#include <QMetaObject>
#include <QSerialPort>
#include <QThread>

namespace modbus::factory {

namespace {

void cleanupThread(QThread* thread);
void cleanupWorker(dispatch::ModbusWorker* worker);

std::shared_ptr<QThread> makeManagedThread()
{
    return std::shared_ptr<QThread>(new QThread(), &cleanupThread);
}

std::shared_ptr<dispatch::ModbusWorker> makeManagedWorker(
    const std::shared_ptr<session::IModbusClient>& client,
    QThread* workerThread)
{
    return std::shared_ptr<dispatch::ModbusWorker>(
        new dispatch::ModbusWorker(client, workerThread, nullptr),
        &cleanupWorker);
}

void cleanupThread(QThread* thread)
{
    if (!thread) {
        return;
    }

    if (thread->isRunning()) {
        // Releasing a started stack does not define thread shutdown policy here.
        // Supported callers must already have initiated quit()/wait() before release.
        QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater, Qt::UniqueConnection);
        return;
    }

    delete thread;
}

void cleanupWorker(dispatch::ModbusWorker* worker)
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
    stack.ioThread = makeManagedThread();
    stack.thread = makeManagedThread();
    QThread* ioThreadRaw = stack.ioThread.get();

    // 1. 创建底层通道 (IO)
    stack.channel = createChannel(config, ioThreadRaw);
    if (!stack.channel) return stack;

    // 2. 创建传输层策略 (Protocol)
    auto transport = createTransport(config);
    if (!transport) return stack;

    // 3. 创建客户端会话 (Session)
    stack.client = std::make_shared<session::ModbusClient>(stack.channel, transport);
    stack.client->setConfig(config);

    // 4. 创建工作线程 (Dispatch)
    stack.worker = makeManagedWorker(stack.client, stack.thread.get());
    return stack;
}

std::shared_ptr<io::IChannel> ModbusFactory::createChannel(const base::ModbusConfig& config, QThread* ioThread) {
    if (config.mode == base::ModbusMode::RTU) {
        auto serial = std::make_shared<io::SerialChannel>();
        io::SerialConfig serialConfig;
        serialConfig.portName = config.portName;
        serialConfig.baudRate = config.baudRate;
        serialConfig.dataBits = static_cast<QSerialPort::DataBits>(config.dataBits);
        serialConfig.stopBits = static_cast<QSerialPort::StopBits>(config.stopBits);
        serialConfig.parity = static_cast<QSerialPort::Parity>(config.parity);
        serial->setConfig(serialConfig);
        serial->moveToThread(ioThread);
        return serial;
    } else {
        auto tcp = std::make_shared<io::TcpChannel>();
        tcp->setEndpoint(config.ipAddress, config.port);
        tcp->moveToThread(ioThread);
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
