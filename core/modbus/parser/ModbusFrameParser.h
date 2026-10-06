// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QMetaType>
#include <optional>
#include <QStringList>
#include <modbus/base/ModbusTypes.h>

namespace modbus::parser {

enum class ProtocolType {
    Unknown,
    Tcp,
    Rtu,
    Ascii
};

enum class FrameType {
    Unknown,
    Request,
    Response,
    Exception
};

// Role: Single register or coil entry extracted from parsed Modbus PDU.
// Thread-safety: Thread-compatible (plain value struct).
// Ownership: Value semantics.
struct DataItem {
    uint16_t address = 0;
    QByteArray rawBytes;
    QVariant value;
    QString hexString;
    QString binaryString;
    QString desc;
};

// Role: Structured result of Modbus frame dissection across TCP, RTU, and ASCII variants.
// Thread-safety: Thread-compatible (plain value struct).
// Ownership: Value semantics.
struct ParseResult {
    bool isValid = false;
    bool isForced = false;
    ProtocolType protocol = ProtocolType::Unknown;
    FrameType type = FrameType::Unknown;
    QString error;
    QStringList warnings;
    QDateTime timestamp;

    uint8_t slaveId = 0;
    modbus::base::FunctionCode functionCode = modbus::base::FunctionCode::ReadCoils;
    bool isException = false;
    modbus::base::ExceptionCode exceptionCode = modbus::base::ExceptionCode::None;
    uint16_t expectedQuantity = 0;

    // TCP specific
    uint16_t transactionId = 0;
    uint16_t protocolId = 0;
    uint16_t length = 0;

    // RTU specific
    uint16_t checksum = 0;           // Wire CRC [RTU]
    uint16_t calculatedChecksum = 0; // Locally calculated CRC [RTU]
    bool checksumValid = false;

    QVector<DataItem> dataItems;
    QByteArray rawFrame;
    QByteArray pduData;
};

// Dissects a raw Modbus frame view into structured fields and register items.
// If type is Unknown, protocol heuristics (MBAP length vs RTU CRC) are applied.
[[nodiscard]] ParseResult parse(QByteArrayView frame,
                                ProtocolType type = ProtocolType::Unknown,
                                uint16_t startAddress = 0,
                                uint16_t expectedQuantity = 0,
                                bool force = false,
                                modbus::base::RegisterOrder order = modbus::base::RegisterOrder::ABCD) noexcept;

void applyRegisterOrder(ParseResult& result, modbus::base::RegisterOrder order) noexcept;

// Dissects raw PDU payload into individual coil or register data items.
void parsePdu(ParseResult& result,
              QByteArrayView pdu,
              uint16_t startAddress,
              uint16_t expectedQuantity,
              modbus::base::RegisterOrder order = modbus::base::RegisterOrder::ABCD) noexcept;

} // namespace modbus::parser

Q_DECLARE_METATYPE(modbus::parser::ProtocolType)
Q_DECLARE_METATYPE(modbus::parser::ParseResult)
