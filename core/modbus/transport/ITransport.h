#pragma once

#include <QByteArray>
#include <optional>
#include "../base/ModbusFrame.h"

namespace modbus::transport {

class ITransport {
public:
    virtual ~ITransport() = default;

    // 构建请求帧（将 PDU 封装为 ADU）
    virtual QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) = 0;

    // 解析响应帧（从 ADU 解析出 PDU）
    // 返回值使用 std::optional 表示解析可能失败（如校验错误、长度不足）
    // 或者直接抛出异常？考虑到嵌入式/实时性，这里用 optional 或 error code 更合适
    // 为了简单，我们返回 Pdu，如果解析失败则返回空 Pdu 或抛出异常。
    // 考虑到后续可能需要区分“校验错”和“数据不完整”，这里设计为：
    // 输入完整的数据包（假设 io 层已经分包完成），输出 Pdu
    virtual std::optional<base::Pdu> parseResponse(const QByteArray& adu) = 0;

    // 检查数据包是否完整（用于粘包处理）
    // 返回：0 表示不完整，>0 表示完整包的长度，-1 表示错误数据需丢弃
    virtual int checkIntegrity(const QByteArray& data) = 0;
};

} // namespace modbus::transport
