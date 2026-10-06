// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <memory>
#include <optional>
#include <QThread>
#include "infra/io/IChannel.h"
#include "../transport/ITransport.h"
#include "../dispatch/ModbusWorker.h"
#include "../base/ModbusConfig.h"

namespace modbus::session { class ModbusClient; }

namespace modbus::factory {

// Role: Aggregates fully-assembled Modbus protocol stack components (channel, client, thread, worker).
// Thread-safety: Thread-safe factory output; individual component thread affinities govern usage.
// Ownership: Shared ownership of components; lifecycle managed by caller via std::shared_ptr.
struct ModbusStack {
    std::shared_ptr<io::IChannel> channel;
    std::shared_ptr<session::ModbusClient> client;
    std::shared_ptr<QThread> thread;
    std::shared_ptr<dispatch::ModbusWorker> worker;
};

// Assembles a complete Modbus protocol stack based on provided configuration.
// Returns std::nullopt when the channel or transport cannot be created;
// callers never receive a half-initialised stack.
std::optional<ModbusStack> createStack(const base::ModbusConfig& config);

} // namespace modbus::factory
