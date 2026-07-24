/**
 * @file ModbusFactory.cpp
 * @brief Implementation of ModbusFactory.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusFactory.h"
#include "../transport/ModbusSerialTransport.h"
#include "../transport/ModbusTcpTransport.h"
#include "../session/ModbusClient.h"
#include "infra/io/SerialChannel.h"
#include "infra/io/TcpChannel.h"
#include <QMetaObject>
#include <QSerialPort>
#include <QThread>
#include <spdlog/spdlog.h>

namespace modbus::factory {

namespace {

void cleanupThread(QThread* thread);
void cleanupWorker(dispatch::ModbusWorker* worker);

std::shared_ptr<QThread> makeManagedThread()
{
    return std::shared_ptr<QThread>(new QThread(), &cleanupThread);
}

std::shared_ptr<dispatch::ModbusWorker> makeManagedWorker(
    const std::shared_ptr<session::ModbusClient>& client,
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
        thread->quit();
        if (QThread::currentThread() != thread && thread->wait(1000)) {
            delete thread;
            return;
        }
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

std::shared_ptr<io::IChannel> createChannel(const base::ModbusConfig& config, QThread* ioThread) {
    switch (config.mode) {
    case base::ModbusMode::RTU:
    case base::ModbusMode::ASCII: {
        auto serial = std::make_shared<io::SerialChannel>();
        io::SerialConfig serialConfig = io::toSerialConfig(config);
        serial->setConfig(serialConfig);
        serial->moveToThread(ioThread);
        return serial;
    }
    case base::ModbusMode::TCP: {
        auto tcp = std::make_shared<io::TcpChannel>();
        tcp->setEndpoint(config.ipAddress, config.port);
        tcp->moveToThread(ioThread);
        return tcp;
    }
    }

    return nullptr;
}

std::shared_ptr<transport::ITransport> createTransport(const base::ModbusConfig& config) {
    switch (config.mode) {
    case base::ModbusMode::RTU:
        return std::make_shared<transport::ModbusSerialTransport>(transport::SerialFraming::Rtu);
    case base::ModbusMode::ASCII:
        return std::make_shared<transport::ModbusSerialTransport>(transport::SerialFraming::Ascii);
    case base::ModbusMode::TCP:
        return std::make_shared<transport::ModbusTcpTransport>();
    }

    return nullptr;
}

} // namespace

std::optional<ModbusStack> createStack(const base::ModbusConfig& config) {
    ModbusStack stack;
    stack.ioThread = makeManagedThread();
    stack.thread = makeManagedThread();
    QThread* ioThreadRaw = stack.ioThread.get();

    // 1. 创建底层通道 (IO)
    stack.channel = createChannel(config, ioThreadRaw);
    if (!stack.channel) {
        spdlog::error("ModbusFactory: failed to create channel for mode={}",
                      static_cast<int>(config.mode));
        return std::nullopt;
    }

    // 2. 创建传输层策略 (Protocol)
    auto transport = createTransport(config);
    if (!transport) {
        spdlog::error("ModbusFactory: failed to create transport for mode={}",
                      static_cast<int>(config.mode));
        return std::nullopt;
    }

    // 3. 创建客户端会话 (Session)
    stack.client = std::make_shared<session::ModbusClient>(stack.channel, transport);
    stack.client->setConfig(config);

    // 4. 创建工作线程 (Dispatch)
    stack.worker = makeManagedWorker(stack.client, stack.thread.get());
    spdlog::info("ModbusFactory: stack created mode={}", static_cast<int>(config.mode));
    return std::move(stack);
}

} // namespace modbus::factory
