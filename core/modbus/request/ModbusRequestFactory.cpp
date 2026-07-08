/**
 * @file ModbusRequestFactory.cpp
 * @brief Implementation of ModbusRequestFactory.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusRequestFactory.h"
#include "modbus/base/ModbusPduBuilder.h"

namespace modbus::request {

BuildResult ModbusRequestFactory::buildReadRequest(const ReadRequestSpec& spec) {
    using namespace modbus::base;

    auto result = ModbusPduBuilder::buildReadRequest(
        static_cast<FunctionCode>(spec.functionCode),
        static_cast<int>(spec.startAddress),
        static_cast<int>(spec.quantity));

    if (!result.isOk()) {
        return BuildResult::fail(result.error().toStdString());
    }

    return BuildResult::ok(std::move(result.value()));
}

BuildResult ModbusRequestFactory::buildWriteRequest(const WriteRequestSpec& spec) {
    using namespace modbus::base;

    PduBuildResult result = PduBuildResult::fail(QStringLiteral("Unsupported function code"));

    if (spec.functionCode == 0x05) {
        // value: 0xFF00 = ON, 0x0000 = OFF
        bool value = false;
        if (spec.rawData.size() >= 2) {
            uint16_t raw = (static_cast<uint16_t>(spec.rawData[0]) << 8) | spec.rawData[1];
            value = (raw == 0xFF00);
        } else if (spec.rawData.size() == 1) {
            value = (spec.rawData[0] != 0x00);
        }
        result = ModbusPduBuilder::buildWriteSingleCoil(
            static_cast<int>(spec.startAddress), value);
    } else if (spec.functionCode == 0x06) {
        uint16_t val = 0;
        if (spec.rawData.size() == 1) {
            val = spec.rawData[0];
        } else if (spec.rawData.size() >= 2) {
            val = (static_cast<uint16_t>(spec.rawData[0]) << 8) | spec.rawData[1];
        }
        result = ModbusPduBuilder::buildWriteSingleRegister(
            static_cast<int>(spec.startAddress), val);
    } else if (spec.functionCode == 0x0F) {
        QByteArray bytes(reinterpret_cast<const char*>(spec.rawData.data()),
                         static_cast<qsizetype>(spec.rawData.size()));
        result = ModbusPduBuilder::buildWriteMultipleCoils(
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            bytes);
    } else if (spec.functionCode == 0x10) {
        QByteArray bytes(reinterpret_cast<const char*>(spec.rawData.data()),
                         static_cast<qsizetype>(spec.rawData.size()));
        result = ModbusPduBuilder::buildWriteMultipleRegisters(
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            bytes);
    }

    if (!result.isOk()) {
        return BuildResult::fail(result.error().toStdString());
    }

    return BuildResult::ok(std::move(result.value()));
}

} // namespace modbus::request
