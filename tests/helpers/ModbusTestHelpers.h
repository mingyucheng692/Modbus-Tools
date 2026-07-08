#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "modbus/base/ModbusConfig.h"
#include "modbus/base/ModbusFrame.h"
#include "modbus/session/IModbusClient.h"

#include <QByteArray>

// ---------------------------------------------------------------------------
// Factory functions — reduce boilerplate when constructing common test objects
// ---------------------------------------------------------------------------

namespace modbus::test {

inline modbus::base::ModbusConfig MakeModbusConfig(modbus::base::ModbusMode mode,
                                                    int timeoutMs = 500,
                                                    int retries = 1) {
    modbus::base::ModbusConfig cfg;
    cfg.mode = mode;
    cfg.timeoutMs = timeoutMs;
    cfg.retries = retries;
    cfg.retryIntervalMs = 50;
    cfg.slaveId = 1;
    return cfg;
}

inline modbus::base::Pdu MakePdu(modbus::base::FunctionCode fc,
                                  const QByteArray& data = {}) {
    return modbus::base::Pdu(fc, data);
}

/// Helper: build a minimal read-holding-registers request Pdu
inline modbus::base::Pdu MakeReadHoldingRegistersRequest(uint16_t addr = 0,
                                                          uint16_t qty = 1) {
    QByteArray payload(4, '\0');
    payload[0] = static_cast<char>((addr >> 8) & 0xFF);
    payload[1] = static_cast<char>(addr & 0xFF);
    payload[2] = static_cast<char>((qty >> 8) & 0xFF);
    payload[3] = static_cast<char>(qty & 0xFF);
    return modbus::base::Pdu(modbus::base::FunctionCode::ReadHoldingRegisters,
                              payload);
}

/// Shorter alias — common test PDU
inline modbus::base::Pdu MakeReadHoldingRegistersPdu(uint16_t addr = 0,
                                                      uint16_t qty = 1) {
    return MakeReadHoldingRegistersRequest(addr, qty);
}

/// Helper: build a write-multiple-registers request Pdu (for broadcast tests)
inline modbus::base::Pdu MakeWriteMultipleRegistersPdu(uint16_t addr = 0,
                                                        uint16_t qty = 1) {
    const int dataBytes = static_cast<int>(qty) * 2;
    QByteArray payload(5 + dataBytes, '\0');  // 4B header + 1B byteCount + data
    payload[0] = static_cast<char>((addr >> 8) & 0xFF);
    payload[1] = static_cast<char>(addr & 0xFF);
    payload[2] = static_cast<char>((qty >> 8) & 0xFF);
    payload[3] = static_cast<char>(qty & 0xFF);
    payload[4] = static_cast<char>(dataBytes); // byte count
    return modbus::base::Pdu(modbus::base::FunctionCode::WriteMultipleRegisters,
                              payload);
}

} // namespace modbus::test

// ---------------------------------------------------------------------------
// Assertion macros — self-describing, one-line checks for common patterns.
// Each macro uses GTEST's streaming assertion to produce clear failure messages.
// ---------------------------------------------------------------------------

#define ASSERT_RESPONSE_SUCCESS(response)                                      \
    do {                                                                       \
        ASSERT_FALSE((response).isError())                                     \
            << "Expected success but got error: "                              \
            << (response).error.toStdString();                                 \
    } while (false)

#define ASSERT_RESPONSE_ERROR(response, msgContains)                           \
    do {                                                                       \
        ASSERT_TRUE((response).isError())                                      \
            << "Expected error but response was successful";                   \
        ASSERT_TRUE((response).error.contains(msgContains))                    \
            << "Expected error containing \"" << (msgContains) << "\" "       \
            << "but got \"" << (response).error.toStdString() << "\"";        \
    } while (false)