/**
 * @file IModbusClient.h
 * @brief Header file for IModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <memory>
#include <QString>
#include <QMetaType>
#include "../base/ModbusFrame.h"
#include "../base/ModbusConfig.h"

namespace modbus::session {

enum class ModbusResponseKind {
    Success,
    NoResponseExpected,
    Error
};

// Structured error code so the UI can distinguish "soft lock contention"
// (Busy) from genuine transport/protocol errors and show a precise message.
enum class RequestError {
    None,
    Busy
};

// 响应结果类型：包含 Pdu 或错误信息
//
// Note: isSuccess / responseReceived / noResponseExpected / retryCount are
// kept for backward compatibility but are fully derivable from `kind` and
// `attemptCount`. New code should prefer the accessor methods below
// (isError(), isNoResponseExpected(), hasPdu()) or derive from `attemptCount`
// (retryCount == max(0, attemptCount - 1)) instead of reading these fields.
struct ModbusResponse {
    ModbusResponseKind kind = ModbusResponseKind::Error;
    RequestError errorCode = RequestError::None;
    base::Pdu pdu;
    [[deprecated("use !isError() instead (true for Success and NoResponseExpected)")]]
    bool isSuccess = false;
    QString error;
    int rttMs = -1;
    [[deprecated("use (kind == ModbusResponseKind::Success) instead")]]
    bool responseReceived = false;
    [[deprecated("use isNoResponseExpected() instead")]]
    bool noResponseExpected = false;
    int attemptCount = 0;
    [[deprecated("derive from attemptCount: max(0, attemptCount - 1)")]]
    int retryCount = 0;

    bool hasPdu() const { return kind != ModbusResponseKind::Error; }
    bool isError() const { return kind == ModbusResponseKind::Error; }
    bool isNoResponseExpected() const { return kind == ModbusResponseKind::NoResponseExpected; }
    bool isBusy() const { return errorCode == RequestError::Busy; }

    static ModbusResponse Success(base::Pdu pdu, int rttMs = -1, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Success;
        response.pdu = std::move(pdu);
        response.isSuccess = true;
        response.rttMs = rttMs;
        response.responseReceived = true;
        response.noResponseExpected = false;
        response.attemptCount = attemptCount;
        response.retryCount = attemptCount > 0 ? attemptCount - 1 : 0;
        return response;
    }

    static ModbusResponse NoResponseExpected(base::Pdu pdu, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::NoResponseExpected;
        response.pdu = std::move(pdu);
        response.isSuccess = true;
        response.rttMs = -1;
        response.responseReceived = false;
        response.noResponseExpected = true;
        response.attemptCount = attemptCount;
        response.retryCount = attemptCount > 0 ? attemptCount - 1 : 0;
        return response;
    }
    
    static ModbusResponse Error(QString error, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Error;
        response.error = std::move(error);
        response.isSuccess = false;
        response.rttMs = -1;
        response.responseReceived = false;
        response.noResponseExpected = false;
        response.attemptCount = attemptCount;
        response.retryCount = attemptCount > 0 ? attemptCount - 1 : 0;
        return response;
    }

    static ModbusResponse Busy(QString error) {
        ModbusResponse response = Error(std::move(error));
        response.errorCode = RequestError::Busy;
        return response;
    }
};

class IModbusClient {
public:
    virtual ~IModbusClient() noexcept = default;

    // 发送请求并等待响应（由 Worker 线程保证不阻塞 UI）
    // slaveId: 如果为 -1，则使用 setConfig 设置的默认 slaveId
    virtual ModbusResponse sendRequest(const base::Pdu& request, int slaveId = -1) = 0;

    // 发送原始数据（不经过 PDU 封装，直接写入通道）
    // 用于非标准测试
    virtual void sendRaw(const QByteArray& data) = 0;

    // 连接/断开
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual QString lastChannelError() = 0;
    
    // 中止当前正在进行的后台请求（用于关机/退出）
    virtual void abort() = 0;
    
    // 更新配置
    virtual void setConfig(const base::ModbusConfig& config) = 0;
};

} // namespace modbus::session

Q_DECLARE_METATYPE(modbus::session::ModbusResponse)
