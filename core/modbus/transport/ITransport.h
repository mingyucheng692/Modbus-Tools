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

class ITransport {
public:
    virtual ~ITransport() = default;

    // 构建请求帧（将 PDU 封装为 ADU）
    virtual QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) = 0;

    virtual ParseResponseResult parseResponse(const QByteArray& adu) = 0;

    // 检查数据包是否完整（用于粘包处理）
    // RTU 的帧边界由 session 基于静默时间控制；transport 只负责结构/校验和判断。
    // 返回：0 表示不完整，>0 表示完整包的长度，-1 表示错误数据需丢弃
    virtual int checkIntegrity(const QByteArray& data) = 0;
};

} // namespace modbus::transport
