/**
 * @file ModbusFrameParser.h
 * @brief Header file for ModbusFrameParser.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QMetaType>
#include <optional>
#include <QStringList>
#include <modbus/base/ModbusTypes.h>

namespace modbus::parser {

// 解析协议类型
enum class ProtocolType {
    Unknown,
    Tcp,
    Rtu,
    Ascii
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
    bool isForced = false;
    ProtocolType protocol = ProtocolType::Unknown;
    FrameType type = FrameType::Unknown;
    QString error;
    QStringList warnings;
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

/**
 * @brief 解析 Modbus 帧
 * @param frame 原始帧数据
 * @param type 指定协议类型 (若为 Unknown 则自动检测)
 * @param startAddress 起始地址 (仅用于 Response 帧解析数据地址，Request 帧会自动从报文中提取)
 * @param expectedQuantity 期望数量 (用于 Read Response 限定有效位/寄存器个数)
 */
ParseResult parse(const QByteArray& frame,
                  ProtocolType type = ProtocolType::Unknown,
                  uint16_t startAddress = 0,
                  uint16_t expectedQuantity = 0,
                  bool force = false,
                  modbus::base::RegisterOrder order = modbus::base::RegisterOrder::ABCD);

/**
 * @brief Apply register byte/word order transformation to the parse result.
 * @param result The parse result to modify.
 * @param order The target register order.
 */
void applyRegisterOrder(ParseResult& result, modbus::base::RegisterOrder order);

/**
 * @brief 解析 PDU 数据段
 */
void parsePdu(ParseResult& result,
              const QByteArray& pdu,
              uint16_t startAddress,
              uint16_t expectedQuantity,
              modbus::base::RegisterOrder order = modbus::base::RegisterOrder::ABCD);

} // namespace modbus::parser

Q_DECLARE_METATYPE(modbus::parser::ProtocolType)
Q_DECLARE_METATYPE(modbus::parser::ParseResult)
