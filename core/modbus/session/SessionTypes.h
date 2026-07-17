/**
 * @file SessionTypes.h
 * @brief Header file for Modbus session response types.
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
struct ModbusResponse {
    ModbusResponseKind kind = ModbusResponseKind::Error;
    RequestError errorCode = RequestError::None;
    base::Pdu pdu;
    QString error;
    int rttMs = -1;
    int attemptCount = 0;

    bool isError() const { return kind == ModbusResponseKind::Error; }
    bool isNoResponseExpected() const { return kind == ModbusResponseKind::NoResponseExpected; }
    bool isBusy() const { return errorCode == RequestError::Busy; }
    int retryCount() const { return attemptCount > 0 ? attemptCount - 1 : 0; }

    static ModbusResponse Success(base::Pdu pdu, int rttMs = -1, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Success;
        response.pdu = std::move(pdu);
        response.rttMs = rttMs;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse NoResponseExpected(base::Pdu pdu, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::NoResponseExpected;
        response.pdu = std::move(pdu);
        response.rttMs = -1;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse Error(QString error, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Error;
        response.error = std::move(error);
        response.rttMs = -1;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse Busy(QString error) {
        ModbusResponse response = Error(std::move(error));
        response.errorCode = RequestError::Busy;
        return response;
    }
};

} // namespace modbus::session

Q_DECLARE_METATYPE(modbus::session::ModbusResponse)
