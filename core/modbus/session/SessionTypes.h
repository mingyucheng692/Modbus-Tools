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

#include "../base/ModbusTypes.h"
#include "common/Result.h"

namespace modbus::session {

/// Orthogonal to transport state: tracks whether the Modbus device is
/// actually responding, not just whether the TCP socket is open.
/// Transitions:
///   Unknown -> Healthy  (first successful request)
///   Unknown -> Unresponsive (first failed request)
///   Healthy -> Unresponsive (request failure after being healthy)
///   Unresponsive -> Healthy (request success after being unresponsive)
enum class SessionHealth {
    Unknown,      // Transport established, no Modbus exchange yet
    Healthy,      // At least one request succeeded
    Unresponsive  // Most recent request failed (timeout, error, etc.)
};

enum class ModbusResponseKind {
    Success,
    NoResponseExpected,
    Error
};

// Aliased to ModbusError so legacy callers remain backward-compatible
using RequestError = base::ModbusError;

// 响应结果类型：包含 Pdu 或错误信息
struct ModbusResponse {
    ModbusResponseKind kind = ModbusResponseKind::Error;
    base::ModbusError errorCode = base::ModbusError::None;
    base::ExceptionCode exceptionCode = base::ExceptionCode::None;
    base::Pdu pdu;
    QString error;
    int rttMs = -1;
    int attemptCount = 0;

    bool isError() const { return kind == ModbusResponseKind::Error; }
    bool isNoResponseExpected() const { return kind == ModbusResponseKind::NoResponseExpected; }
    bool isBusy() const { return errorCode == base::ModbusError::Busy; }
    int retryCount() const { return attemptCount > 0 ? attemptCount - 1 : 0; }

    [[nodiscard]] ::common::Result<base::Pdu, base::ModbusError> toResult() const {
        if (!isError()) {
            return ::common::Result<base::Pdu, base::ModbusError>::ok(pdu);
        }
        return ::common::Result<base::Pdu, base::ModbusError>::err(
            errorCode != base::ModbusError::None ? errorCode : base::ModbusError::Unknown);
    }

    static ModbusResponse Success(base::Pdu pdu, int rttMs = -1, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Success;
        response.errorCode = base::ModbusError::None;
        response.pdu = std::move(pdu);
        response.rttMs = rttMs;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse NoResponseExpected(base::Pdu pdu, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::NoResponseExpected;
        response.errorCode = base::ModbusError::None;
        response.pdu = std::move(pdu);
        response.rttMs = -1;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse Error(base::ModbusError errorCode,
                                QString error = QString(),
                                int attemptCount = 0,
                                base::ExceptionCode exceptionCode = base::ExceptionCode::None) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Error;
        response.errorCode = errorCode;
        response.exceptionCode = exceptionCode;
        response.error = error.isEmpty() ? QString::fromLatin1(base::toString(errorCode)) : std::move(error);
        response.rttMs = -1;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse Error(QString error, int attemptCount = 0) {
        ModbusResponse response;
        response.kind = ModbusResponseKind::Error;
        response.errorCode = base::ModbusError::None;
        response.error = std::move(error);
        response.rttMs = -1;
        response.attemptCount = attemptCount;
        return response;
    }

    static ModbusResponse Busy(QString error = QString()) {
        return Error(base::ModbusError::Busy,
                     error.isEmpty() ? QStringLiteral("Request already in progress") : std::move(error));
    }
};

} // namespace modbus::session

Q_DECLARE_METATYPE(modbus::session::ModbusResponse)
