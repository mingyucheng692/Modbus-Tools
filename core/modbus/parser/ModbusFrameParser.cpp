/**
 * @file ModbusFrameParser.cpp
 * @brief Implementation of ModbusFrameParser.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusFrameParser.h"
#include "Config.h"
#include "../base/ModbusEndianCodec.h"
#include "../base/ModbusProtocolChecks.h"
#include <QtEndian>
#include <QCoreApplication>
#include <spdlog/spdlog.h>
#include <bitset>
#include <cstring>

namespace modbus::parser {

namespace {

QString formatFunctionCodeHex(uint8_t functionCode)
{
    return QString("%1").arg(functionCode, 2, 16, QChar('0')).toUpper();
}

QString formatFrameHex(QByteArrayView frame)
{
    return frame.toByteArray().toHex(' ').toUpper();
}

QString getExceptionString(modbus::base::ExceptionCode code)
{
    using namespace modbus::base;
    switch (code) {
    case ExceptionCode::IllegalFunction:
        return QCoreApplication::translate("ModbusFrameParser", "Illegal Function");
    case ExceptionCode::IllegalDataAddress:
        return QCoreApplication::translate("ModbusFrameParser", "Illegal Data Address");
    case ExceptionCode::IllegalDataValue:
        return QCoreApplication::translate("ModbusFrameParser", "Illegal Data Value");
    case ExceptionCode::ServerDeviceFailure:
        return QCoreApplication::translate("ModbusFrameParser", "Server Device Failure");
    case ExceptionCode::Acknowledge:
        return QCoreApplication::translate("ModbusFrameParser", "Acknowledge");
    case ExceptionCode::ServerDeviceBusy:
        return QCoreApplication::translate("ModbusFrameParser", "Server Device Busy");
    case ExceptionCode::NegativeAcknowledge:
        return QCoreApplication::translate("ModbusFrameParser", "Negative Acknowledge");
    case ExceptionCode::MemoryParityError:
        return QCoreApplication::translate("ModbusFrameParser", "Memory Parity Error");
    case ExceptionCode::GatewayPathUnavailable:
        return QCoreApplication::translate("ModbusFrameParser", "Gateway Path Unavailable");
    case ExceptionCode::GatewayTargetDeviceFailed:
        return QCoreApplication::translate("ModbusFrameParser", "Gateway Target Device Failed To Respond");
    default:
        return QCoreApplication::translate("ModbusFrameParser", "Unknown Exception");
    }
}

bool detectTcp(QByteArrayView frame) noexcept
{
    return base::inspectTcpAdu(frame) == frame.size();
}

bool detectRtu(QByteArrayView frame) noexcept
{
    return base::inspectRtuAdu(frame) == frame.size();
}

bool detectAscii(QByteArrayView frame) noexcept
{
    return base::inspectAsciiAdu(frame) == frame.size();
}

bool hasAddressInPdu(modbus::base::FunctionCode functionCode, QByteArrayView pdu) noexcept
{
    using namespace modbus::base;
    if (pdu.size() < 5) {
        return false;
    }

    switch (functionCode) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs:
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters:
        return pdu.size() == 5;
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters:
        return true;
    default:
        return false;
    }
}

uint16_t extractAddressFromPdu(QByteArrayView pdu) noexcept
{
    if (pdu.size() < 3) {
        return 0;
    }

    uint16_t address = 0;
    return modbus::base::readBigEndian<uint16_t>(pdu, 1, address) ? address : 0;
}

uint16_t determineEffectiveStartAddress(modbus::base::FunctionCode functionCode,
                                        QByteArrayView pdu,
                                        uint16_t userStartAddress) noexcept
{
    if (hasAddressInPdu(functionCode, pdu)) {
        return extractAddressFromPdu(pdu);
    }
    if (userStartAddress != 0) {
        return userStartAddress;
    }
    return 0;
}

ParseResult parseTcp(QByteArrayView frame, uint16_t startAddress, uint16_t expectedQuantity, bool force, modbus::base::RegisterOrder order) noexcept
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTimeUtc();
    result.protocol = ProtocolType::Tcp;
    result.rawFrame = frame.toByteArray();
    result.expectedQuantity = expectedQuantity;
    result.isForced = force;

    base::TcpAduFields fields;
    const int integrity = base::inspectTcpAdu(frame, &fields);
    if (!force) {
        if (integrity == 0) {
            SPDLOG_DEBUG("FrameParser: reject reason=tcp_too_short frameLen={}", frame.size());
            result.isValid = false;
            result.error = QCoreApplication::translate(
                               "ModbusFrameParser",
                               "Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes")
                               .arg(frame.size());
            return result;
        }
        if (integrity < 0) {
            SPDLOG_DEBUG("FrameParser: reject reason=tcp_invalid_mbap frameLen={}", frame.size());
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "Invalid TCP MBAP header or length");
            return result;
        }
        if (integrity != frame.size()) {
            SPDLOG_DEBUG("FrameParser: reject reason=tcp_trailing_bytes expected={} frameLen={}", integrity, frame.size());
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "TCP frame contains trailing bytes. Expected %1 bytes, got %2")
                               .arg(integrity)
                               .arg(frame.size());
            return result;
        }
    } else {
        if (frame.size() < 7) {
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "Frame too short for TCP MBAP");
            return result;
        }
        if (integrity < 0) {
            result.warnings << QCoreApplication::translate("ModbusFrameParser", "Warning: Invalid TCP MBAP header or length (Forced)");
        }
        if (integrity > 0 && integrity != frame.size()) {
             result.warnings << QCoreApplication::translate("ModbusFrameParser", "Warning: TCP frame contains trailing bytes (Forced)");
        }
    }

    result.transactionId = fields.transactionId;
    result.protocolId = fields.protocolId;
    result.length = fields.length;
    result.slaveId = fields.unitId;

    // 提取 PDU (从第 8 字节开始，长度 = Length - 1 (UnitId))
    const qsizetype pduSize = force ? (frame.size() - 7) : (result.length - 1);
    if (pduSize <= 0 || (7 + pduSize > frame.size())) {
        result.isValid = false;
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2")
                           .arg(result.length)
                           .arg(pduSize);
        return result;
    }

    QByteArrayView pdu = frame.sliced(7, pduSize);
    parsePdu(result, pdu, startAddress, expectedQuantity, order);

    return result;
}

ParseResult parseRtu(QByteArrayView frame, uint16_t startAddress, uint16_t expectedQuantity, bool force, modbus::base::RegisterOrder order) noexcept
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTimeUtc();
    result.protocol = ProtocolType::Rtu;
    result.rawFrame = frame.toByteArray();
    result.expectedQuantity = expectedQuantity;
    result.isForced = force;

    base::RtuAduFields fields;
    const int integrity = base::inspectRtuAdu(frame, &fields);
    if (!force && integrity == 0) {
        result.isValid = false;
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Frame too short for Modbus RTU. Expected at least 5 bytes, got %1")
                           .arg(frame.size());
        return result;
    }

    // 解析头部
    result.slaveId = fields.slaveId;
    result.checksum = fields.receivedCrc;
    result.calculatedChecksum = fields.calculatedCrc;
    result.checksumValid = (integrity > 0);

    if (!force) {
        if (integrity < 0) {
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "CRC Mismatch. Expected %1, Got %2")
                               .arg(fields.calculatedCrc, 4, 16, QChar('0'))
                               .arg(fields.receivedCrc, 4, 16, QChar('0'))
                               .toUpper();
            return result;
        }
        if (integrity != frame.size()) {
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "RTU frame contains trailing bytes. Expected %1 bytes, got %2")
                               .arg(integrity)
                               .arg(frame.size());
            return result;
        }
    } else {
        if (frame.size() < 3) {
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "Frame too short for RTU");
            return result;
        }
        if (integrity < 0) {
            result.warnings << QCoreApplication::translate("ModbusFrameParser", "Warning: CRC Mismatch. Expected %1, Got %2 (Forced)")
                               .arg(fields.calculatedCrc, 4, 16, QChar('0'))
                               .arg(fields.receivedCrc, 4, 16, QChar('0'))
                               .toUpper();
        }
        if (integrity > 0 && integrity != frame.size()) {
            result.warnings << QCoreApplication::translate("ModbusFrameParser", "Warning: RTU frame contains trailing bytes (Forced)");
        }
    }

    // 提取 PDU (去除首部 SlaveId 和尾部 CRC)
    const qsizetype pduLen = qMax<qsizetype>(0, frame.size() - 3);
    const QByteArrayView pdu = (frame.size() >= 3) ? frame.sliced(1, pduLen) : QByteArrayView();
    parsePdu(result, pdu, startAddress, expectedQuantity, order);

    return result;
}

ParseResult parseAscii(QByteArrayView frame,
                      uint16_t startAddress,
                      uint16_t expectedQuantity,
                      bool force,
                      modbus::base::RegisterOrder order) noexcept
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTimeUtc();
    result.protocol = ProtocolType::Ascii;
    result.rawFrame = frame.toByteArray();
    result.expectedQuantity = expectedQuantity;
    result.isForced = force;

    base::AsciiAduFields fields;
    const int integrity = base::inspectAsciiAdu(frame, &fields);
    if (!force) {
        if (integrity == 0) {
            SPDLOG_DEBUG("FrameParser: reject reason=ascii_missing_crlf frameLen={}", frame.size());
            result.isValid = false;
            result.error = QCoreApplication::translate(
                "ModbusFrameParser",
                "Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload");
            return result;
        }
        if (integrity < 0) {
            SPDLOG_DEBUG("FrameParser: reject reason=ascii_lrc_mismatch frameLen={}", frame.size());
            result.isValid = false;
            result.error = QCoreApplication::translate(
                "ModbusFrameParser",
                "Invalid Modbus ASCII frame or LRC mismatch");
            return result;
        }
        if (integrity != frame.size()) {
            SPDLOG_DEBUG("FrameParser: reject reason=ascii_trailing_bytes expected={} frameLen={}", integrity, frame.size());
            result.isValid = false;
            result.error = QCoreApplication::translate(
                "ModbusFrameParser",
                "ASCII frame contains trailing bytes. Expected %1 bytes, got %2")
                .arg(integrity)
                .arg(frame.size());
            return result;
        }
    } else if (integrity < 0) {
        result.warnings << QCoreApplication::translate(
            "ModbusFrameParser", "Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)");
    }

    result.slaveId = fields.slaveId;
    result.checksum = fields.receivedLrc;
    result.calculatedChecksum = fields.calculatedLrc;
    result.checksumValid = (integrity > 0);

    const qsizetype pduLen = qMax<qsizetype>(0, fields.binaryAdu.size() - 2);
    const QByteArrayView pdu = (fields.binaryAdu.size() >= 2)
        ? QByteArrayView(fields.binaryAdu).sliced(1, pduLen)
        : QByteArrayView();
    parsePdu(result, pdu, startAddress, expectedQuantity, order);
    return result;
}

} // namespace

ParseResult parse(QByteArrayView frame,
                  ProtocolType type,
                  uint16_t startAddress,
                  uint16_t expectedQuantity,
                  bool force,
                  modbus::base::RegisterOrder order) noexcept
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTimeUtc();
    result.rawFrame = frame.toByteArray();
    result.expectedQuantity = expectedQuantity;
    result.isForced = force;

    if (frame.isEmpty()) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "Empty frame data");
        return result;
    }

    // 1. 协议检测
    if (type == ProtocolType::Unknown) {
        if (detectTcp(frame)) {
            // An eight-byte RTU request at address zero with quantity two can
            // also look like an MBAP header. Validate the TCP PDU before giving
            // it priority over a CRC-valid RTU frame. Explicit type bounds this
            // probe to one call (it cannot enter auto-detection again).
            auto tcpCandidate = parse(frame, ProtocolType::Tcp, startAddress, expectedQuantity, force, order);
            if (tcpCandidate.isValid || !detectRtu(frame)) return tcpCandidate;
            type = ProtocolType::Rtu;
        } else if (detectAscii(frame)) {
            type = ProtocolType::Ascii;
        } else if (detectRtu(frame)) {
            type = ProtocolType::Rtu;
        } else {
            result.isValid = false;
            result.error = QCoreApplication::translate(
                               "ModbusFrameParser",
                               "Unable to identify protocol. Frame length: %1 bytes, data: %2")
                               .arg(frame.size())
                               .arg(formatFrameHex(frame));
            return result;
        }
    }

    result.protocol = type;

    // 2. 根据协议解析头部
    if (type == ProtocolType::Tcp) {
        result = parseTcp(frame, startAddress, expectedQuantity, force, order);
    } else if (type == ProtocolType::Ascii) {
        result = parseAscii(frame, startAddress, expectedQuantity, force, order);
    } else {
        result = parseRtu(frame, startAddress, expectedQuantity, force, order);
    }

    // 3. 应用诊断字节序变换
    if (result.isValid) {
        applyRegisterOrder(result, order);
    }

    return result;
}

void parsePdu(ParseResult& result,
              QByteArrayView pdu,
              uint16_t startAddress,
              uint16_t expectedQuantity,
              modbus::base::RegisterOrder order) noexcept
{
    Q_UNUSED(order);
    if (pdu.isEmpty()) {
        result.isValid = false;
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Empty PDU. Function code is missing from the frame");
        return;
    }

    uint8_t fcByte = static_cast<uint8_t>(pdu[0]);
    result.isException = (fcByte & 0x80) != 0;
    
    if (result.isException) {
        result.functionCode = static_cast<modbus::base::FunctionCode>(fcByte & 0x7F);
        result.type = FrameType::Exception;
        if (pdu.size() < 2) {
            result.isValid = false;
            result.error = QCoreApplication::translate(
                               "ModbusFrameParser",
                               "Exception PDU too short for function 0x%1. Expected 2 bytes, got %2")
                               .arg(formatFunctionCodeHex(fcByte & 0x7F))
                               .arg(pdu.size());
            return;
        }
        result.exceptionCode = static_cast<modbus::base::ExceptionCode>(pdu[1]);
        const QString exceptionDesc = getExceptionString(result.exceptionCode);
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Modbus exception: %1 (code %2)")
                           .arg(exceptionDesc)
                           .arg(static_cast<int>(result.exceptionCode));
        result.isValid = true;
        return;
    }

    const QByteArray finalPduData = (pdu.size() > 1) ? pdu.sliced(1).toByteArray() : QByteArray();

    result.functionCode = static_cast<modbus::base::FunctionCode>(fcByte);
    result.isValid = true;
    const uint16_t effectiveStartAddress = determineEffectiveStartAddress(result.functionCode, pdu, startAddress);

    using namespace modbus::base;

    switch (result.functionCode) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs:
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters: {
        // 区分 Request 和 Response
        // Request: [FC(1)] [StartAddr(2)] [Quantity(2)] -> Total PDU size 5
        if (pdu.size() == 5) {
            result.type = FrameType::Request;
            uint16_t start = 0;
            uint16_t quantity = 0;
            if (!readBigEndian<uint16_t>(pdu, 1, start) ||
                !readBigEndian<uint16_t>(pdu, 3, quantity)) {
                result.isValid = false;
                result.error = QCoreApplication::translate("ModbusFrameParser", "Failed to read request fields");
                break;
            }
            DataItem item;
            item.address = effectiveStartAddress;
            result.dataItems.append(item);
            result.expectedQuantity = quantity;
        } else {
            // Response: [FC(1)] [ByteCount(1)] [Data(N)]
            result.type = FrameType::Response;
            if (pdu.size() < 2) {
                result.isValid = false;
                result.error = QCoreApplication::translate(
                                   "ModbusFrameParser",
                                   "Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2")
                                   .arg(formatFunctionCodeHex(fcByte))
                                   .arg(pdu.size());
                break;
            }
            
            uint8_t byteCount = static_cast<uint8_t>(pdu[1]);
            const qsizetype payloadSize = pdu.size() - 2;
            if (byteCount != payloadSize) {
                result.isValid = false;
                result.error = QCoreApplication::translate(
                                   "ModbusFrameParser",
                                   "Byte count mismatch for function 0x%1. Declared %2, actual %3")
                                   .arg(formatFunctionCodeHex(fcByte))
                                   .arg(byteCount)
                                   .arg(payloadSize);
                break;
            }
            
            // 解析数据
            if (result.functionCode == FunctionCode::ReadHoldingRegisters || 
                result.functionCode == FunctionCode::ReadInputRegisters) {
                if (expectedQuantity != 0 && static_cast<int>(byteCount) != static_cast<int>(expectedQuantity) * 2) {
                    result.isValid = false;
                    result.error = QCoreApplication::translate(
                                       "ModbusFrameParser",
                                       "Register response byte count does not match expected quantity. Declared %1, expected %2")
                                       .arg(byteCount)
                                       .arg(expectedQuantity * 2);
                    break;
                }
                if ((byteCount % 2) != 0) {
                    result.isValid = false;
                    result.error = QCoreApplication::translate(
                                       "ModbusFrameParser",
                                       "Register byte count must be even, got %1")
                                       .arg(byteCount);
                    break;
                }
                // 寄存器数据 (每2字节一个)
                int count = byteCount / 2;
                result.dataItems.reserve(count);
                for (int i = 0; i < count; ++i) {
                    uint16_t val = 0;
                    if (!readBigEndian<uint16_t>(pdu, 2 + i * 2, val)) {
                        result.isValid = false;
                        result.error = QCoreApplication::translate("ModbusFrameParser", "Truncated register data");
                        break;
                    }
                    DataItem item;
                    item.address = effectiveStartAddress + i;
                    item.value = val;
                    item.rawBytes.resize(2);
                    item.rawBytes[0] = static_cast<char>((val >> 8) & 0xFF);
                    item.rawBytes[1] = static_cast<char>(val & 0xFF);
                    item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
                    
                    // Binary string
                    QString binStr = QString::number(val, 2).rightJustified(16, '0');
                    // Insert spaces for readability (every 4 bits)
                    for (int k = 12; k > 0; k -= 4) binStr.insert(k, ' ');
                    item.binaryString = binStr;

                    result.dataItems.append(item);
                }
            } else {
                // 线圈数据 (位操作) - 展开每一位
                // Byte 0: Coils 0-7 (LSB is Coil 0)
                // Byte 1: Coils 8-15
                const int totalBits = byteCount * 8;
                if (expectedQuantity != 0) {
                    if (expectedQuantity > totalBits || expectedQuantity <= totalBits - 8) {
                        result.isValid = false;
                        result.error = QCoreApplication::translate(
                                           "ModbusFrameParser",
                                           "Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits")
                                           .arg(byteCount)
                                           .arg(expectedQuantity);
                        break;
                    }
                }
                const int bitsToParse = expectedQuantity == 0 ? totalBits : qMin<int>(expectedQuantity, totalBits);
                result.dataItems.reserve(bitsToParse);
                int currentBitAddress = 0;
                for (int i = 0; i < byteCount; ++i) {
                    uint8_t byteVal = static_cast<uint8_t>(pdu[2 + i]);
                    for (int bit = 0; bit < 8; ++bit) {
                        if (currentBitAddress >= bitsToParse) {
                            break;
                        }
                        bool isOn = (byteVal >> bit) & 0x01;
                        DataItem item;
                        item.address = effectiveStartAddress + currentBitAddress;
                        item.value = isOn;
                        item.rawBytes = QByteArray(1, static_cast<char>(isOn ? 1 : 0));
                        item.hexString = isOn ? "01" : "00";
                        item.binaryString = isOn ? "1" : "0";
                        result.dataItems.append(item);
                        currentBitAddress++;
                    }
                }
            }
        }
        break;
    }
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister: {
        // Request 和 Response 格式相同: [Addr(2)] [Value(2)]
        if (pdu.size() != 5) {
            result.isValid = false;
            result.error = QCoreApplication::translate(
                               "ModbusFrameParser",
                               "Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2")
                               .arg(formatFunctionCodeHex(fcByte))
                               .arg(pdu.size() - 1);
            break;
        }
        result.type = FrameType::Unknown;
        uint16_t addr = 0;
        uint16_t val = 0;
        if (!readBigEndian<uint16_t>(pdu, 1, addr) ||
            !readBigEndian<uint16_t>(pdu, 3, val)) {
            result.isValid = false;
            result.error = QCoreApplication::translate("ModbusFrameParser", "Failed to read write value");
            break;
        }
        
        DataItem item;
        item.address = addr;
        item.value = val;
        item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
        
        if (result.functionCode == FunctionCode::WriteSingleCoil) {
            bool isOn = (val == 0xFF00);
            item.rawBytes = QByteArray(1, static_cast<char>(isOn ? 1 : 0));
            item.binaryString = isOn ? "1" : "0";
        } else {
            item.rawBytes.resize(2);
            item.rawBytes[0] = static_cast<char>((val >> 8) & 0xFF);
            item.rawBytes[1] = static_cast<char>(val & 0xFF);
            QString binStr = QString::number(val, 2).rightJustified(16, '0');
            for (int k = 12; k > 0; k -= 4) binStr.insert(k, ' ');
            item.binaryString = binStr;
        }
        result.dataItems.append(item);
        break;
    }
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters: {
        // Request: [Start(2)] [Quant(2)] [ByteCount(1)] [Data(N)]
        // Response: [Start(2)] [Quant(2)]
        if (pdu.size() == 5) {
            result.type = FrameType::Response;
            uint16_t start = 0;
            uint16_t quant = 0;
            if (!readBigEndian<uint16_t>(pdu, 1, start) ||
                !readBigEndian<uint16_t>(pdu, 3, quant)) {
                result.isValid = false;
                result.error = QCoreApplication::translate("ModbusFrameParser", "Failed to read response echo");
                break;
            }
            DataItem item;
            item.address = start;
            result.dataItems.append(item);
            result.expectedQuantity = quant;
        } else {
            result.type = FrameType::Request;
            if (pdu.size() < 6) {
                result.isValid = false;
                result.error = QCoreApplication::translate(
                                   "ModbusFrameParser",
                                   "Write request PDU too short");
                break;
            }
            uint16_t start = 0;
            uint16_t quant = 0;
            if (!readBigEndian<uint16_t>(pdu, 1, start) ||
                !readBigEndian<uint16_t>(pdu, 3, quant)) {
                result.isValid = false;
                result.error = QCoreApplication::translate("ModbusFrameParser", "Failed to read write request fields");
                break;
            }
            uint8_t byteCount = static_cast<uint8_t>(pdu[5]);
            const qsizetype payloadBytes = pdu.size() - 6;
            if (byteCount != payloadBytes) {
                result.isValid = false;
                result.error = QCoreApplication::translate(
                                   "ModbusFrameParser",
                                   "Write request byte count mismatch. Declared %1, actual %2")
                                   .arg(byteCount)
                                   .arg(payloadBytes);
                break;
            }
            
            if (result.functionCode == FunctionCode::WriteMultipleRegisters) {
                if (static_cast<int>(byteCount) != static_cast<int>(quant) * 2) {
                    result.isValid = false;
                    result.error = QCoreApplication::translate(
                                       "ModbusFrameParser",
                                       "Register write byte count does not match quantity. Declared %1, expected %2")
                                       .arg(byteCount)
                                       .arg(quant * 2);
                    break;
                }
                if ((byteCount % 2) != 0) {
                    result.isValid = false;
                    result.error = QCoreApplication::translate(
                                       "ModbusFrameParser",
                                       "Register byte count must be even, got %1")
                                       .arg(byteCount);
                    break;
                }
                int count = byteCount / 2;
                result.dataItems.reserve(count);
                for (int i = 0; i < count; ++i) {
                    uint16_t val = 0;
                    if (!readBigEndian<uint16_t>(pdu, 6 + i * 2, val)) {
                        result.isValid = false;
                        result.error = QCoreApplication::translate("ModbusFrameParser", "Truncated register data");
                        break;
                    }
                    DataItem item;
                    item.address = start + i;
                    item.value = val;
                    item.rawBytes.resize(2);
                    item.rawBytes[0] = static_cast<char>((val >> 8) & 0xFF);
                    item.rawBytes[1] = static_cast<char>(val & 0xFF);
                    item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
                    
                    QString binStr = QString::number(val, 2).rightJustified(16, '0');
                    for (int k = 12; k > 0; k -= 4) binStr.insert(k, ' ');
                    item.binaryString = binStr;

                    result.dataItems.append(item);
                }
            } else {
                 const int expectedCoilBytes = (quant + 7) / 8;
                 if (byteCount != expectedCoilBytes) {
                     result.isValid = false;
                     result.error = QCoreApplication::translate(
                                        "ModbusFrameParser",
                                        "Coil write byte count does not match quantity. Declared %1, expected %2")
                                        .arg(byteCount)
                                        .arg(expectedCoilBytes);
                     break;
                 }
                 // Write Multiple Coils Request - Expand bits
                 result.dataItems.reserve(quant);
                 int currentBitAddress = 0;
                 for (int i = 0; i < byteCount; ++i) {
                     uint8_t byteVal = static_cast<uint8_t>(pdu[6 + i]);
                     // Only parse up to 'quant' bits
                     for (int bit = 0; bit < 8; ++bit) {
                         if (currentBitAddress >= quant) break;

                         bool isOn = (byteVal >> bit) & 0x01;
                         DataItem item;
                         item.address = start + currentBitAddress;
                         item.value = isOn;
                         item.rawBytes = QByteArray(1, static_cast<char>(isOn ? 1 : 0));
                         item.hexString = isOn ? "01" : "00";
                         item.binaryString = isOn ? "1" : "0";
                         result.dataItems.append(item);
                         currentBitAddress++;
                     }
                 }
            }
        }
        break;
    }
    default:
        SPDLOG_DEBUG("FrameParser: reject reason=unsupported_fc fc=0x{:02x}", fcByte);
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Unsupported function code 0x%1 for deep parsing")
                           .arg(formatFunctionCodeHex(fcByte));
        break;
    }

    result.pduData = finalPduData;
}

void applyRegisterOrder(ParseResult& result, modbus::base::RegisterOrder order) noexcept
{
    using namespace modbus::base;
    if (order == RegisterOrder::ABCD || result.dataItems.isEmpty()) {
        return;
    }

    // Identify if the data items are registers (not coils/bits)
    // For ReadHolding/InputRegisters, values are uint16_t
    const bool isRegister = (result.functionCode == FunctionCode::ReadHoldingRegisters ||
                             result.functionCode == FunctionCode::ReadInputRegisters ||
                             result.functionCode == FunctionCode::WriteSingleRegister ||
                             result.functionCode == FunctionCode::WriteMultipleRegisters);

    if (!isRegister) {
        return;
    }

    auto swapRegisterBytes = [](DataItem& item) {
        if (item.value.typeId() == QMetaType::Char) return; // Should not happen for registers
        uint16_t val = item.value.toUInt();
        uint16_t swapped = static_cast<uint16_t>((val << 8) | (val >> 8));
        item.value = swapped;
        item.rawBytes.resize(2);
        item.rawBytes[0] = static_cast<char>((swapped >> 8) & 0xFF);
        item.rawBytes[1] = static_cast<char>(swapped & 0xFF);
        item.hexString = QString("%1").arg(swapped, 4, 16, QChar('0')).toUpper();
        
        QString binStr = QString::number(swapped, 2).rightJustified(16, '0');
        for (int k = 12; k > 0; k -= 4) binStr.insert(k, ' ');
        item.binaryString = binStr;
    };

    int count = result.dataItems.size();
    for (int i = 0; i < count; i += 2) {
        if (i + 1 < count) {
            // Processing pair [i, i+1]
            DataItem& item1 = result.dataItems[i];
            DataItem& item2 = result.dataItems[i + 1];

            switch (order) {
            case RegisterOrder::BADC:
                swapRegisterBytes(item1);
                swapRegisterBytes(item2);
                break;
            case RegisterOrder::CDAB: {
                // Swap the RAW values between items, but keep addresses fixed
                QVariant val1 = item1.value;
                QString hex1 = item1.hexString;
                QString bin1 = item1.binaryString;
                QByteArray raw1 = item1.rawBytes;

                item1.value = item2.value;
                item1.hexString = item2.hexString;
                item1.binaryString = item2.binaryString;
                item1.rawBytes = item2.rawBytes;

                item2.value = val1;
                item2.hexString = hex1;
                item2.binaryString = bin1;
                item2.rawBytes = raw1;
                break;
            }
            case RegisterOrder::DCBA: {
                // Swap and Swap Bytes
                QVariant val1 = item1.value;
                QString hex1 = item1.hexString;
                QString bin1 = item1.binaryString;
                QByteArray raw1 = item1.rawBytes;

                item1.value = item2.value;
                item1.hexString = item2.hexString;
                item1.binaryString = item2.binaryString;
                item1.rawBytes = item2.rawBytes;
                swapRegisterBytes(item1);

                item2.value = val1;
                item2.hexString = hex1;
                item2.binaryString = bin1;
                item2.rawBytes = raw1;
                swapRegisterBytes(item2);
                break;
            }
            default:
                break;
            }
        } else {
            // Handling the last single register in an odd-sized sequence
            if (order == RegisterOrder::BADC || order == RegisterOrder::DCBA) {
                swapRegisterBytes(result.dataItems[i]);
            }
        }
    }
}

} // namespace modbus::parser
