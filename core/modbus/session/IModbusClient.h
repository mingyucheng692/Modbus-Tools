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

// 响应结果类型：包含 Pdu 或错误信息
struct ModbusResponse {
    base::Pdu pdu;
    bool isSuccess = false;
    QString error;
    int rttMs = -1;
    bool responseReceived = false;
    bool noResponseExpected = false;
    int attemptCount = 0;
    int retryCount = 0;

    static ModbusResponse Success(base::Pdu pdu, int rttMs = -1, int attemptCount = 0) {
        ModbusResponse response;
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
        response.error = std::move(error);
        response.isSuccess = false;
        response.rttMs = -1;
        response.responseReceived = false;
        response.noResponseExpected = false;
        response.attemptCount = attemptCount;
        response.retryCount = attemptCount > 0 ? attemptCount - 1 : 0;
        return response;
    }
};

class IModbusClient {
public:
    virtual ~IModbusClient() = default;

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
