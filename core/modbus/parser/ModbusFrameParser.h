#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QMetaType>
#include <optional>
#include <modbus/base/ModbusTypes.h>

namespace modbus::core::parser {

// 解析协议类型
enum class ProtocolType {
    Unknown,
    Tcp,
    Rtu
};

// 帧类型
enum class FrameType {
    Unknown,
    Request,
    Response,
    Exception
};

// 单个寄存器/线圈的数据项
struct DataItem {
    uint16_t address;           // 寄存器/线圈地址
    QByteArray rawBytes;        // 原始字节
    QVariant value;             // 解析后的值 (bool, uint16, int16, etc.)
    QString hexString;          // 十六进制显示
    QString binaryString;       // 二进制显示
    QString desc;               // 描述 (例如 "Register 40001")
};

// 完整解析结果
struct ParseResult {
    bool isValid = false;
    ProtocolType protocol = ProtocolType::Unknown;
    FrameType type = FrameType::Unknown;
    QString error;
    QDateTime timestamp;

    // 基础字段
    uint8_t slaveId = 0;
    modbus::base::FunctionCode functionCode = modbus::base::FunctionCode::ReadCoils;
    bool isException = false;
    modbus::base::ExceptionCode exceptionCode = modbus::base::ExceptionCode::None;
    uint16_t expectedQuantity = 0;

    // TCP 特定字段
    uint16_t transactionId = 0;
    uint16_t protocolId = 0;
    uint16_t length = 0;

    // RTU 特定字段
    uint16_t checksum = 0;      // 接收到的 CRC
    uint16_t calculatedChecksum = 0; // 计算出的 CRC
    bool checksumValid = false;

    // 解析出的数据列表
    QVector<DataItem> dataItems;
    
    QByteArray rawFrame;

    // 原始 PDU 数据 (去除头部和校验)
    QByteArray pduData;
};

class ModbusFrameParser {
public:
    // 静态工具类，无需构造
    ModbusFrameParser() = delete;

    /**
     * @brief 解析 Modbus 帧
     * @param frame 原始帧数据
     * @param type 指定协议类型 (若为 Unknown 则自动检测)
     * @param startAddress 起始地址 (仅用于 Response 帧解析数据地址，Request 帧会自动从报文中提取)
     * @param expectedQuantity 期望数量 (用于 Read Response 限定有效位/寄存器个数)
     */
    static ParseResult parse(const QByteArray& frame,
                             ProtocolType type = ProtocolType::Unknown,
                             uint16_t startAddress = 0,
                             uint16_t expectedQuantity = 0);

private:
    static bool detectTcp(const QByteArray& frame);
    static bool detectRtu(const QByteArray& frame);
    
    static ParseResult parseTcp(const QByteArray& frame, uint16_t startAddress, uint16_t expectedQuantity);
    static ParseResult parseRtu(const QByteArray& frame, uint16_t startAddress, uint16_t expectedQuantity);
    
    // PDU 解析通用逻辑
    static void parsePdu(ParseResult& result, const QByteArray& pdu, uint16_t startAddress, uint16_t expectedQuantity);
    static bool hasAddressInPdu(modbus::base::FunctionCode functionCode, const QByteArray& pdu);
    static uint16_t extractAddressFromPdu(const QByteArray& pdu);
    static uint16_t determineEffectiveStartAddress(modbus::base::FunctionCode functionCode,
                                                   const QByteArray& pdu,
                                                   uint16_t userStartAddress);
    
    // 辅助函数
    static QString getExceptionString(modbus::base::ExceptionCode code);
};

} // namespace modbus::core::parser

Q_DECLARE_METATYPE(modbus::core::parser::ProtocolType)
Q_DECLARE_METATYPE(modbus::core::parser::ParseResult)
