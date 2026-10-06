// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <optional>
#include "../base/ModbusFrame.h"

namespace modbus::transport {

enum class ParseResponseStatus {
    Ok,
    Unmatched,
    Invalid
};

struct ParseResponseResult {
    ParseResponseStatus status = ParseResponseStatus::Invalid;
    std::optional<base::Pdu> pdu;
};

// Role: Interface abstracting Modbus Application Data Unit (ADU) framing, integrity checks, and matching.
// Thread-safety: Implementations must document concurrency semantics (typically thread-compatible or protected via internal mutex).
// Ownership: Owned by session layer (ModbusClient) via std::shared_ptr<ITransport>.
class ITransport {
public:
    virtual ~ITransport() noexcept = default;

    // Encapsulates PDU into wire ADU frame (e.g. MBAP prefix for TCP, CRC16 suffix for RTU).
    virtual QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) = 0;

    virtual ParseResponseResult parseResponse(const QByteArray& adu) = 0;

    // Checks ADU completeness and integrity for stream deframing.
    // RTU frame boundaries are governed by session silent interval (t3.5); transport validates structural length and checksum.
    // Returns: 0 if incomplete, >0 package byte length if complete, -1 if corrupted and must be dropped.
    virtual int checkIntegrity(const QByteArray& data) = 0;

    // Clears internal transaction state (e.g. pending TCP transaction map) to prevent stale state pollution.
    virtual void resetPendingState() = 0;
};

} // namespace modbus::transport
