/**
 * @file ModbusTypes.h
 * @brief Strong type definitions for Modbus application layer.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <chrono>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "../../../../core/AppConstants.h"

namespace ui::application::modbus {

struct PollSpec {
    uint8_t functionCode = 0;
    uint16_t startAddress = 0;
    uint16_t quantity = 0;
    uint8_t slaveId = static_cast<uint8_t>(app::constants::Values::Modbus::kDefaultSlaveId);
};

struct ModbusTimingParams {
    std::chrono::milliseconds timeout{app::constants::Values::Modbus::kDefaultTimeoutMs};
    int retryCount = app::constants::Values::Modbus::kDefaultRetryCount;
    std::chrono::milliseconds retryInterval{app::constants::Values::Modbus::kDefaultRetryIntervalMs};
};

enum class RequestKind {
    Read,
    Write,
    Poll,
    Raw
};

class IRequestDispatcher {
public:
    virtual ~IRequestDispatcher() = default;
    virtual void dispatch(const ::modbus::base::Pdu& request, int slaveId, int requestId) = 0;
    virtual void dispatchRaw(const QByteArray& data) = 0;
};

} // namespace ui::application::modbus
