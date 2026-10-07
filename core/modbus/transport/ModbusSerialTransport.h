// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "ITransport.h"
#include <QByteArray>
#include <cstdint>
#include <optional>

namespace modbus::transport {

enum class SerialFraming {
    Rtu,   // Modbus RTU: binary framing with CRC-16.
    Ascii  // Modbus ASCII: hex-encoded framing with LRC.
};

// Role: Serial transport layer constructing and validating RTU and ASCII ADU frames.
// Thread-safety: Thread-compatible; accessed from the owner communication thread.
// Ownership: Owned by ModbusClient / ModbusStack composition root via std::unique_ptr<ITransport>.
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
