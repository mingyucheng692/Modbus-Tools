/**
 * @file ModbusSerialTransport.h
 * @brief Header file for ModbusSerialTransport.
 *
 * Merged transport for Modbus RTU and ASCII serial modes using a
 * SerialFraming strategy parameter.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ITransport.h"
#include <QByteArray>
#include <cstdint>
#include <optional>

namespace modbus::transport {

/**
 * @brief Serial framing mode for ModbusSerialTransport.
 */
enum class SerialFraming {
    Rtu,   ///< Modbus RTU — binary framing with CRC-16.
    Ascii  ///< Modbus ASCII — hex-encoded framing with LRC.
};

/**
 * @brief Modbus serial transport layer (ADU build/parse) for RTU and ASCII.
 *
 * Uses SerialFraming to select between RTU (CRC-16, binary) and ASCII
 * (LRC, hex-encoded with ':' / "\\r\\n") framing.
 */
class ModbusSerialTransport : public ITransport {
public:
    explicit ModbusSerialTransport(SerialFraming framing);

    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;

private:
    SerialFraming framing_;
    std::optional<uint8_t> pendingSlave_;
};

} // namespace modbus::transport