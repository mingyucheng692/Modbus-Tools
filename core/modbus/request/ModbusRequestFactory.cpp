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

std::optional<modbus::base::Pdu> ModbusRequestFactory::buildReadRequest(const ReadRequestSpec& spec,
                                                                         std::string* errorOut) {
    using namespace modbus::base;

    QString error;
    auto result = ModbusPduBuilder::buildReadRequest(
        static_cast<FunctionCode>(spec.functionCode),
        static_cast<int>(spec.startAddress),
        static_cast<int>(spec.quantity),
        &error);

    if (!result) {
        if (errorOut) *errorOut = error.toStdString();
        return std::nullopt;
    }

    return std::move(result);
}

std::optional<modbus::base::Pdu> ModbusRequestFactory::buildWriteRequest(const WriteRequestSpec& spec,
                                                                          std::string* errorOut) {
    using namespace modbus::base;

    QString error;
    std::optional<Pdu> result;

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
            static_cast<int>(spec.startAddress), value, &error);
    } else if (spec.functionCode == 0x06) {
        uint16_t val = 0;
        if (spec.rawData.size() == 1) {
            val = spec.rawData[0];
        } else if (spec.rawData.size() >= 2) {
            val = (static_cast<uint16_t>(spec.rawData[0]) << 8) | spec.rawData[1];
        }
        result = ModbusPduBuilder::buildWriteSingleRegister(
            static_cast<int>(spec.startAddress), val, &error);
    } else if (spec.functionCode == 0x0F) {
        QByteArray bytes(reinterpret_cast<const char*>(spec.rawData.data()),
                         static_cast<qsizetype>(spec.rawData.size()));
        result = ModbusPduBuilder::buildWriteMultipleCoils(
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            bytes, &error);
    } else if (spec.functionCode == 0x10) {
        QByteArray bytes(reinterpret_cast<const char*>(spec.rawData.data()),
                         static_cast<qsizetype>(spec.rawData.size()));
        result = ModbusPduBuilder::buildWriteMultipleRegisters(
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            bytes, &error);
    } else {
        if (errorOut) *errorOut = "Unsupported function code";
        return std::nullopt;
    }

    if (!result) {
        if (errorOut) *errorOut = error.toStdString();
        return std::nullopt;
    }

    return std::move(result);
}

} // namespace modbus::request
