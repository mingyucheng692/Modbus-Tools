#include "ModbusFrameParser.h"
#include <QtEndian>
#include <QDataStream>
#include <QCoreApplication>

namespace modbus::core::parser {

ParseResult ModbusFrameParser::parse(const QByteArray& frame, ProtocolType type, uint16_t startAddress)
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();

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
            result.error = QCoreApplication::translate("ModbusFrameParser", "Unable to identify protocol (neither valid TCP nor RTU)");
            return result;
        }
    }

    result.protocol = type;

    // 2. 根据协议解析头部
    if (type == ProtocolType::Tcp) {
        return parseTcp(frame, startAddress);
    } else {
        return parseRtu(frame, startAddress);
    }
}

bool ModbusFrameParser::detectTcp(const QByteArray& frame)
{
    // TCP 最小长度: MBAP(7) + FC(1) = 8 bytes
    if (frame.size() < 8) return false;

    // 检查 Protocol ID (Bytes 2-3) 必须为 0
    uint16_t protoId = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(frame.constData() + 2));
    if (protoId != 0) return false;

    // 检查 Length (Bytes 4-5)
    uint16_t len = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(frame.constData() + 4));
    if (len < 2 || len > 260) return false; // Length 包括 UnitId(1) + PDU

    // 检查总长度是否匹配
    if (frame.size() < (6 + len)) return false;

    return true;
}

bool ModbusFrameParser::detectRtu(const QByteArray& frame)
{
    // RTU 最小长度: SlaveId(1) + FC(1) + CRC(2) = 4 bytes
    if (frame.size() < 4) return false;

    // 检查 CRC
    uint16_t receivedCrc = qFromLittleEndian<uint16_t>(reinterpret_cast<const uchar*>(frame.constData() + frame.size() - 2));
    uint16_t calcCrc = calculateCrc(frame.left(frame.size() - 2));

    return receivedCrc == calcCrc;
}

ParseResult ModbusFrameParser::parseTcp(const QByteArray& frame, uint16_t startAddress)
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();
    result.protocol = ProtocolType::Tcp;

    if (frame.size() < 8) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "Frame too short for Modbus TCP");
        return result;
    }

    QDataStream stream(frame);
    stream.setByteOrder(QDataStream::BigEndian);

    // 解析 MBAP Header
    stream >> result.transactionId;
    stream >> result.protocolId;
    stream >> result.length;
    stream >> result.slaveId;

    // 完整性检查
    if (frame.size() < (6 + result.length)) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "Incomplete TCP frame. Expected %1 bytes, got %2").arg(6 + result.length).arg(frame.size());
        return result;
    }

    // 提取 PDU (从第 8 字节开始，长度 = Length - 1 (UnitId))
    int pduSize = result.length - 1;
    if (pduSize <= 0) {
        result.isValid = false;
        result.error = "Invalid PDU length";
        return result;
    }

    QByteArray pdu = frame.mid(7, pduSize);
    parsePdu(result, pdu, startAddress);

    return result;
}

ParseResult ModbusFrameParser::parseRtu(const QByteArray& frame, uint16_t startAddress)
{
    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();
    result.protocol = ProtocolType::Rtu;

    if (frame.size() < 4) {
        result.isValid = false;
        result.error = "Frame too short for Modbus RTU";
        return result;
    }

    // 解析头部
    result.slaveId = static_cast<uint8_t>(frame[0]);
    
    // CRC 校验
    uint16_t receivedCrc = qFromLittleEndian<uint16_t>(reinterpret_cast<const uchar*>(frame.constData() + frame.size() - 2));
    uint16_t calcCrc = calculateCrc(frame.left(frame.size() - 2));

    result.checksum = receivedCrc;
    result.calculatedChecksum = calcCrc;
    result.checksumValid = (receivedCrc == calcCrc);

    if (!result.checksumValid) {
        result.isValid = false;
        result.error = QCoreApplication::translate("ModbusFrameParser", "CRC Mismatch. Expected %1, Got %2").arg(calcCrc, 4, 16, QChar('0')).arg(receivedCrc, 4, 16, QChar('0')).toUpper();
        return result;
    }

    // 提取 PDU (去除首部 SlaveId 和尾部 CRC)
    QByteArray pdu = frame.mid(1, frame.size() - 3);
    parsePdu(result, pdu, startAddress);

    return result;
}

void ModbusFrameParser::parsePdu(ParseResult& result, const QByteArray& pdu, uint16_t startAddress)
{
    if (pdu.isEmpty()) {
        result.isValid = false;
        result.error = "Empty PDU";
        return;
    }

    uint8_t fcByte = static_cast<uint8_t>(pdu[0]);
    result.isException = (fcByte & 0x80) != 0;
    
    if (result.isException) {
        result.functionCode = static_cast<modbus::base::FunctionCode>(fcByte & 0x7F);
        result.type = FrameType::Exception;
        if (pdu.size() >= 2) {
            result.exceptionCode = static_cast<modbus::base::ExceptionCode>(pdu[1]);
            result.error = QString("Exception Response: Code %1").arg(static_cast<int>(result.exceptionCode));
        }
        result.isValid = true;
        return;
    }

    result.functionCode = static_cast<modbus::base::FunctionCode>(fcByte);
    result.pduData = pdu.mid(1);
    result.isValid = true;

    // 根据功能码判断是请求还是响应，并解析数据
    // 简单的启发式判断：
    // Read Request (01, 02, 03, 04): 长度通常固定为 5 (FC + StartAddr + Quantity)
    // Read Response: 长度 = 2 (FC + ByteCount) + N
    // Write Request (05, 06): 长度 = 5 (FC + Addr + Value)
    // Write Response (05, 06): 长度 = 5 (FC + Addr + Value) - 与 Request 相同，通常视为 Response 处理
    // Write Multiple Request (15, 16): 长度 = 6 + N (FC + Start + Quant + ByteCount + Data)
    // Write Multiple Response (15, 16): 长度 = 5 (FC + Start + Quant)
    
    // 这里我们主要解析数据部分。为了简化，我们假设用户如果提供了 startAddress，可能是为了解析 Response 数据。
    // 如果没有提供 startAddress，我们尝试从 Request 中提取。

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
            item.address = start;
            item.desc = QString("Request: Start Address %1, Quantity %2").arg(start).arg(quantity);
            result.dataItems.append(item);
        } else {
            // Response: [ByteCount(1)] [Data(N)]
            result.type = FrameType::Response;
            if (pdu.size() < 2) break;
            
            uint8_t byteCount;
            stream >> byteCount;
            
            // 解析数据
            if (result.functionCode == FunctionCode::ReadHoldingRegisters || 
                result.functionCode == FunctionCode::ReadInputRegisters) {
                // 寄存器数据 (每2字节一个)
                int count = byteCount / 2;
                for (int i = 0; i < count; ++i) {
                    uint16_t val;
                    stream >> val;
                    DataItem item;
                    item.address = startAddress + i;
                    item.value = val;
                    item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
                    item.desc = QString("Register %1").arg(item.address);
                    item.rawBytes = QByteArray::fromRawData(reinterpret_cast<const char*>(&val), 2); // 注意这里序问题，仅作示意
                    result.dataItems.append(item);
                }
            } else {
                // 线圈数据 (位操作)
                // 暂时只显示原始字节
                DataItem item;
                item.address = startAddress;
                item.desc = QString("Coil Data (Bytes: %1)").arg(byteCount);
                item.hexString = result.pduData.mid(1).toHex().toUpper();
                result.dataItems.append(item);
            }
        }
        break;
    }
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister: {
        // Request 和 Response 格式相同: [Addr(2)] [Value(2)]
        // 默认为 Request/Response (无法区分，视为操作)
        result.type = FrameType::Request; 
        uint16_t addr, val;
        stream >> addr >> val;
        
        DataItem item;
        item.address = addr;
        item.value = val;
        item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
        
        if (result.functionCode == FunctionCode::WriteSingleCoil) {
            item.desc = (val == 0xFF00) ? "Write Coil ON" : "Write Coil OFF";
        } else {
            item.desc = QString("Write Register %1").arg(addr);
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
            item.desc = QString("Response: Written %1 items starting at %2").arg(quant).arg(start);
            result.dataItems.append(item);
        } else {
            result.type = FrameType::Request;
            uint16_t start, quant;
            uint8_t byteCount;
            stream >> start >> quant >> byteCount;
            
            if (result.functionCode == FunctionCode::WriteMultipleRegisters) {
                int count = byteCount / 2;
                for (int i = 0; i < count; ++i) {
                    uint16_t val;
                    stream >> val;
                    DataItem item;
                    item.address = start + i;
                    item.value = val;
                    item.hexString = QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
                    item.desc = QString("Write Register %1").arg(item.address);
                    result.dataItems.append(item);
                }
            } else {
                 DataItem item;
                 item.address = start;
                 item.desc = QString("Write %1 Coils").arg(quant);
                 result.dataItems.append(item);
            }
        }
        break;
    }
    default:
        result.error = "Unsupported Function Code for deep parsing";
        break;
    }
}

uint16_t ModbusFrameParser::calculateCrc(const QByteArray& data)
{
    uint16_t crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<uint8_t>(c);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

} // namespace modbus::core::parser
