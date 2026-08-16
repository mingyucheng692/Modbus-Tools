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
#include "common/ThreadGuard.h"
#include "infra/io/SerialChannel.h"
#include "infra/io/TcpChannel.h"
#include <QMetaObject>
#include <QSerialPort>
#include <QThread>
#include <spdlog/spdlog.h>

namespace modbus::factory {

namespace {

// Thread/worker teardown lives in core::common::ThreadGuard — the single
// project-wide implementation of these sequences (see ThreadGuard.h).
std::shared_ptr<QThread> makeManagedThread()
{
    return std::shared_ptr<QThread>(new QThread(), &core::common::ThreadGuard::releaseThread);
}

std::shared_ptr<dispatch::ModbusWorker> makeManagedWorker(
    const std::shared_ptr<session::ModbusClient>& client,
    QThread* workerThread)
{
    return std::shared_ptr<dispatch::ModbusWorker>(
        new dispatch::ModbusWorker(client, workerThread, nullptr),
        [](dispatch::ModbusWorker* worker) {
            core::common::ThreadGuard::releaseWorker(worker, [worker]() { worker->stop(); });
        });
}

std::unique_ptr<io::IChannel> createChannel(const base::ModbusConfig& config, QThread* ioThread) {
    switch (config.mode) {
    case base::ModbusMode::RTU:
    case base::ModbusMode::ASCII: {
        auto serial = std::make_unique<io::SerialChannel>();
        io::SerialConfig serialConfig = io::toSerialConfig(config);
        serial->setConfig(serialConfig);
        serial->moveToThread(ioThread);
        return serial;
    }
    case base::ModbusMode::TCP: {
        auto tcp = std::make_unique<io::TcpChannel>();
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
    auto channel = createChannel(config, ioThreadRaw);
    if (!channel) {
        SPDLOG_ERROR("ModbusFactory: failed to create channel for mode={}",
                      static_cast<int>(config.mode));
        return std::nullopt;
    }
    // The channel's shared_ptr deliberately captures the IO thread's
    // shared_ptr: ModbusClient keeps a channel reference and the async-
    // deleted worker keeps the client, so the channel regularly OUTLIVES
    // this stack's ioThread member. The capture keeps the QThread object
    // alive until after channel deletion (no dangling owner-thread pointer
    // in ChannelBase's guard) and ThreadGuard::releaseChannel performs the
    // thread-affine teardown.
    stack.channel = std::shared_ptr<io::IChannel>(
        channel.release(),
        [ioThread = stack.ioThread](io::IChannel* ch) {
            core::common::ThreadGuard::releaseChannel(ch, ioThread);
        });

    // 2. 创建传输层策略 (Protocol)
    auto transport = createTransport(config);
    if (!transport) {
        SPDLOG_ERROR("ModbusFactory: failed to create transport for mode={}",
                      static_cast<int>(config.mode));
        return std::nullopt;
    }

    // 3. 创建客户端会话 (Session)
    stack.client = std::make_shared<session::ModbusClient>(stack.channel, transport);
    stack.client->setConfig(config);

    // 4. 创建工作线程 (Dispatch)
    stack.worker = makeManagedWorker(stack.client, stack.thread.get());
    SPDLOG_INFO("ModbusFactory: stack created mode={}", static_cast<int>(config.mode));
    return std::move(stack);
}

} // namespace modbus::factory
