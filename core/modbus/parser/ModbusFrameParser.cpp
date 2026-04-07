#include "ModbusFrameParser.h"
#include "AppConstants.h"
#include "../base/ModbusProtocolChecks.h"
#include <QtEndian>
#include <QDataStream>
#include <QCoreApplication>
#include <bitset>

namespace modbus::core::parser {

namespace {

QString formatFunctionCodeHex(uint8_t functionCode)
{
    return QString("%1").arg(functionCode, 2, 16, QChar('0')).toUpper();
}

QString formatFrameHex(const QByteArray& frame)
{
    return frame.toHex(' ').toUpper();
}

} // namespace

QString ModbusFrameParser::getExceptionString(modbus::base::ExceptionCode code)
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

ParseResult ModbusFrameParser::parse(const QByteArray& frame,
                                     ProtocolType type,
                                     uint16_t startAddress,
                                     uint16_t expectedQuantity)
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();
    result.rawFrame = frame;
    result.expectedQuantity = expectedQuantity;

    if (frame.isEmpty()) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "Empty frame data");
        return result;
    }

    // 1. 协议检测
    if (type == ProtocolType::Unknown) {
        if (detectTcp(frame)) {
            type = ProtocolType::Tcp;
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
        return parseTcp(frame, startAddress, expectedQuantity);
    } else {
        return parseRtu(frame, startAddress, expectedQuantity);
    }
}

bool ModbusFrameParser::detectTcp(const QByteArray& frame)
{
    return base::inspectTcpAdu(frame) == frame.size();
}

bool ModbusFrameParser::detectRtu(const QByteArray& frame)
{
    return base::inspectRtuAdu(frame) == frame.size();
}

ParseResult ModbusFrameParser::parseTcp(const QByteArray& frame, uint16_t startAddress, uint16_t expectedQuantity)
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();
    result.protocol = ProtocolType::Tcp;
    result.rawFrame = frame;
    result.expectedQuantity = expectedQuantity;

    base::TcpAduFields fields;
    const int integrity = base::inspectTcpAdu(frame, &fields);
    if (integrity == 0) {
        result.isValid = false;
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes")
                           .arg(frame.size());
        return result;
    }
    if (integrity < 0) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "Invalid TCP MBAP header or length");
        return result;
    }
    if (integrity != frame.size()) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "TCP frame contains trailing bytes. Expected %1 bytes, got %2")
                           .arg(integrity)
                           .arg(frame.size());
        return result;
    }

    result.transactionId = fields.transactionId;
    result.protocolId = fields.protocolId;
    result.length = fields.length;
    result.slaveId = fields.unitId;

    // 提取 PDU (从第 8 字节开始，长度 = Length - 1 (UnitId))
    int pduSize = result.length - 1;
    if (pduSize <= 0) {
        result.isValid = false;
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2")
                           .arg(result.length)
                           .arg(pduSize);
        return result;
    }

    QByteArray pdu = frame.mid(7, pduSize);
    parsePdu(result, pdu, startAddress, expectedQuantity);

    return result;
}

ParseResult ModbusFrameParser::parseRtu(const QByteArray& frame, uint16_t startAddress, uint16_t expectedQuantity)
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();
    result.protocol = ProtocolType::Rtu;
    result.rawFrame = frame;
    result.expectedQuantity = expectedQuantity;

    base::RtuAduFields fields;
    const int integrity = base::inspectRtuAdu(frame, &fields);
    if (integrity == 0) {
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

    // 提取 PDU (去除首部 SlaveId 和尾部 CRC)
    QByteArray pdu = frame.mid(1, frame.size() - 3);
    parsePdu(result, pdu, startAddress, expectedQuantity);

    return result;
}

void ModbusFrameParser::parsePdu(ParseResult& result,
                                 const QByteArray& pdu,
                                 uint16_t startAddress,
                                 uint16_t expectedQuantity)
{
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

    result.functionCode = static_cast<modbus::base::FunctionCode>(fcByte);
    result.pduData = pdu.mid(1);
    result.isValid = true;
    const uint16_t effectiveStartAddress = determineEffectiveStartAddress(result.functionCode, pdu, startAddress);

    using namespace modbus::base;
    QDataStream stream(result.pduData);
    stream.setByteOrder(QDataStream::BigEndian);

    switch (result.functionCode) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs:
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters: {
        // 区分 Request 和 Response
        // Request: [StartAddr(2)] [Quantity(2)] -> Total PDU size 1+4=5
        if (pdu.size() == 5) {
            result.type = FrameType::Request;
            uint16_t start, quantity;
            stream >> start >> quantity;
            DataItem item;
            item.address = effectiveStartAddress;
            result.dataItems.append(item);
            result.expectedQuantity = quantity;
        } else {
            // Response: [ByteCount(1)] [Data(N)]
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
            
            uint8_t byteCount;
            stream >> byteCount;
            if (byteCount != static_cast<uint8_t>(result.pduData.size() - 1)) {
                result.isValid = false;
                result.error = QCoreApplication::translate(
                                   "ModbusFrameParser",
                                   "Byte count mismatch for function 0x%1. Declared %2, actual %3")
                                   .arg(formatFunctionCodeHex(fcByte))
                                   .arg(byteCount)
                                   .arg(result.pduData.size() - 1);
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
                for (int i = 0; i < count; ++i) {
                    uint16_t val;
                    stream >> val;
                    DataItem item;
                    item.address = effectiveStartAddress + i;
                    item.value = val;
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
                int currentBitAddress = 0;
                for (int i = 0; i < byteCount; ++i) {
                    uint8_t byteVal = static_cast<uint8_t>(result.pduData[1 + i]); // +1 to skip byteCount
                    for (int bit = 0; bit < 8; ++bit) {
                        if (currentBitAddress >= bitsToParse) {
                            break;
                        }
                        bool isOn = (byteVal >> bit) & 0x01;
                        DataItem item;
                        item.address = effectiveStartAddress + currentBitAddress;
                        item.value = isOn;
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
        if (result.pduData.size() != 4) {
            result.isValid = false;
            result.error = QCoreApplication::translate(
                               "ModbusFrameParser",
                               "Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2")
                               .arg(formatFunctionCodeHex(fcByte))
                               .arg(result.pduData.size());
            break;
        }
        result.type = FrameType::Unknown;
        uint16_t addr, val;
        stream >> addr >> val;
        
        DataItem item;
        item.address = addr;
        item.value = val;
        item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
        
        if (result.functionCode == FunctionCode::WriteSingleCoil) {
            bool isOn = (val == 0xFF00);
            item.binaryString = isOn ? "1" : "0";
        } else {
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
            uint16_t start, quant;
            stream >> start >> quant;
            DataItem item;
            item.address = start;
            result.dataItems.append(item);
            result.expectedQuantity = quant;
        } else {
            result.type = FrameType::Request;
            uint16_t start, quant;
            uint8_t byteCount;
            stream >> start >> quant >> byteCount;
            const int payloadBytes = qMax(0, result.pduData.size() - 5);
            if (result.pduData.size() < 5 || byteCount != payloadBytes) {
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
                for (int i = 0; i < count; ++i) {
                    uint16_t val;
                    stream >> val;
                    DataItem item;
                    item.address = start + i;
                    item.value = val;
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
                 int currentBitAddress = 0;
                 for (int i = 0; i < byteCount; ++i) {
                     uint8_t byteVal;
                     stream >> byteVal;
                     // Only parse up to 'quant' bits
                     for (int bit = 0; bit < 8; ++bit) {
                         if (currentBitAddress >= quant) break;

                         bool isOn = (byteVal >> bit) & 0x01;
                         DataItem item;
                         item.address = start + currentBitAddress;
                         item.value = isOn;
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
        result.error = QCoreApplication::translate(
                           "ModbusFrameParser",
                           "Unsupported function code 0x%1 for deep parsing")
                           .arg(formatFunctionCodeHex(fcByte));
        break;
    }
}

bool ModbusFrameParser::hasAddressInPdu(modbus::base::FunctionCode functionCode, const QByteArray& pdu)
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

uint16_t ModbusFrameParser::extractAddressFromPdu(const QByteArray& pdu)
{
    if (pdu.size() < 3) {
        return 0;
    }

    return qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(pdu.constData() + 1));
}

uint16_t ModbusFrameParser::determineEffectiveStartAddress(modbus::base::FunctionCode functionCode,
                                                           const QByteArray& pdu,
                                                           uint16_t userStartAddress)
{
    if (hasAddressInPdu(functionCode, pdu)) {
        return extractAddressFromPdu(pdu);
    }
    if (userStartAddress != 0) {
        return userStartAddress;
    }
    return 0;
}

} // namespace modbus::core::parser
