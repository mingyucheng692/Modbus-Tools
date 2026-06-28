/**
 * @file RequestSubmissionService.cpp
 * @brief Implementation of RequestSubmissionService.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RequestSubmissionService.h"
#include "modbus/base/ModbusDataHelper.h"
#include <QRegularExpression>
#include <QCoreApplication>
#include <limits>

namespace ui::application::modbus {

RequestSubmissionService::RequestSubmissionService(QObject* parent)
    : QObject(parent) {
}

RequestSubmissionService::ReadRequestResult RequestSubmissionService::buildReadRequest(
    const PollSpec& spec, RequestKind kind) {
    ReadRequestResult result;

    using namespace ::modbus::request;

    ReadRequestSpec factorySpec;
    factorySpec.functionCode = spec.functionCode;
    factorySpec.startAddress = spec.startAddress;
    factorySpec.quantity = spec.quantity;

    auto buildResult = factory_.buildReadRequest(factorySpec);
    if (!buildResult.ok()) {
        result.ok = false;
        result.error = QString::fromStdString(buildResult.error);
        return result;
    }

    result.pdu = std::move(*buildResult.pdu);
    result.requestId = nextRequestId();
    trackRequest(result.requestId, kind, spec.startAddress);
    result.ok = true;
    return result;
}

RequestSubmissionService::WriteRequestResult RequestSubmissionService::buildWriteRequest(
    uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId, int quantity) {
    WriteRequestResult result;

    using namespace ::modbus::base;
    using namespace ::modbus::request;

    QString trimmed = dataStr.trimmed();
    QByteArray rawBytes;

    if (fc == 0x05) {
        bool coilOn = false;
        if (fmt == QStringLiteral("Decimal")) {
            bool ok = false;
            int value = trimmed.toInt(&ok);
            if (!ok || (value != 0 && value != 1)) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid decimal value for 0x05");
                return result;
            }
            coilOn = (value != 0);
        } else if (fmt == QStringLiteral("Binary")) {
            QString cleaned = trimmed;
            cleaned.remove(QRegularExpression(QStringLiteral("[^0-1]")));
            if (cleaned.isEmpty() || cleaned.size() > 1) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid binary value for 0x05 (expected 0 or 1)");
                return result;
            }
            coilOn = (cleaned == QStringLiteral("1"));
        } else {
            QByteArray bytes = ModbusDataHelper::parseHex(trimmed);
            if (bytes.isEmpty()) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid hex value for 0x05");
                return result;
            }
            if (bytes.size() == 1) {
                uint8_t raw = static_cast<uint8_t>(bytes[0]);
                if (raw != 0x00 && raw != 0x01) {
                    result.ok = false;
                    result.error = QCoreApplication::translate("RequestSubmissionService",
                        "Invalid hex value for 0x05");
                    return result;
                }
                coilOn = raw != 0x00;
            } else {
                uint16_t raw = (static_cast<uint8_t>(bytes[0]) << 8) | static_cast<uint8_t>(bytes[1]);
                if (raw != 0x0000 && raw != 0xFF00) {
                    result.ok = false;
                    result.error = QCoreApplication::translate("RequestSubmissionService",
                        "Invalid hex value for 0x05");
                    return result;
                }
                coilOn = raw == 0xFF00;
            }
        }
        // Encode coil value as raw bytes for factory
        uint16_t coilVal = coilOn ? 0xFF00 : 0x0000;
        rawBytes.append(static_cast<char>((coilVal >> 8) & 0xFF));
        rawBytes.append(static_cast<char>(coilVal & 0xFF));
    } else if (fc == 0x06) {
        if (trimmed.isEmpty()) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "Empty value for 0x06");
            return result;
        }
        if (fmt == QStringLiteral("Decimal")) {
            bool ok = false;
            uint value = trimmed.toUInt(&ok, 10);
            if (!ok || value > 65535) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid decimal value for 0x06");
                return result;
            }
            rawBytes.append(static_cast<char>((value >> 8) & 0xFF));
            rawBytes.append(static_cast<char>(value & 0xFF));
        } else if (fmt == QStringLiteral("Binary")) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "Binary format not supported for registers (0x06)");
            return result;
        } else {
            QByteArray bytes = ModbusDataHelper::parseHex(trimmed);
            if (bytes.isEmpty() || bytes.size() > 2) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid hex value for 0x06");
                return result;
            }
            rawBytes = bytes;
        }
    } else if (fc == 0x0F) {
        if (quantity <= 0) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "Invalid quantity for 0x0F");
            return result;
        }

        if (fmt == QStringLiteral("Binary")) {
            QString bits = trimmed;
            bits.remove(QRegularExpression(QStringLiteral("[^0-1]")));
            if (bits.size() != quantity) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Binary bit count (%1) does not match Quantity (%2)")
                    .arg(bits.size()).arg(quantity);
                return result;
            }
            rawBytes = ModbusDataHelper::parseBinary(bits);
        } else if (fmt == QStringLiteral("Hex")) {
            rawBytes = ModbusDataHelper::parseHex(trimmed);
        } else {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "0x0F requires Hex or Binary data");
            return result;
        }
    } else if (fc == 0x10) {
        if (trimmed.isEmpty()) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "Empty value for 0x10");
            return result;
        }
        if (quantity <= 0) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "Invalid quantity for 0x10");
            return result;
        }
        if (fmt == QStringLiteral("Decimal")) {
            bool okList = false;
            rawBytes = ModbusDataHelper::parseDecimalList(trimmed, okList);
            if (!okList) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid decimal list for 0x10");
                return result;
            }
        } else {
            rawBytes = ModbusDataHelper::parseHex(trimmed);
            if (rawBytes.isEmpty() || (rawBytes.size() % 2 != 0)) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid hex value for 0x10");
                return result;
            }
        }
    }

    // Delegate PDU building to the factory
    WriteRequestSpec spec;
    spec.functionCode = fc;
    spec.startAddress = static_cast<uint16_t>(addr);
    spec.rawData = std::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(rawBytes.constData()),
        static_cast<size_t>(rawBytes.size()));
    spec.quantity = static_cast<uint16_t>(quantity);

    auto buildResult = factory_.buildWriteRequest(spec);
    if (!buildResult.ok()) {
        result.ok = false;
        result.error = QString::fromStdString(buildResult.error);
        return result;
    }

    result.pdu = std::move(*buildResult.pdu);
    result.requestId = nextRequestId();
    trackRequest(result.requestId, RequestKind::Write, static_cast<uint16_t>(addr));
    result.ok = true;
    return result;
}

bool RequestSubmissionService::validateRawData(const QByteArray& data, QString* errorOut) {
    if (data.isEmpty()) {
        if (errorOut) {
            *errorOut = QCoreApplication::translate("RequestSubmissionService",
                "Raw data is empty");
        }
        return false;
    }
    return true;
}

std::optional<RequestTrackingInfo> RequestSubmissionService::lookup(int requestId) const {
    auto itStart = requestStart_.find(requestId);
    auto itKind = requestKinds_.find(requestId);
    auto itAddr = requestAddrs_.find(requestId);
    if (itStart == requestStart_.end() || itKind == requestKinds_.end() || itAddr == requestAddrs_.end()) {
        return std::nullopt;
    }

    RequestTrackingInfo info;
    info.kind = itKind->second;
    info.address = itAddr->second;
    info.startTime = itStart->second;
    return info;
}

std::optional<RequestTrackingInfo> RequestSubmissionService::lookupAndRemove(int requestId) {
    auto itStart = requestStart_.find(requestId);
    auto itKind = requestKinds_.find(requestId);
    auto itAddr = requestAddrs_.find(requestId);
    if (itStart == requestStart_.end() || itKind == requestKinds_.end() || itAddr == requestAddrs_.end()) {
        return std::nullopt;
    }

    RequestTrackingInfo info;
    info.kind = itKind->second;
    info.address = itAddr->second;
    info.startTime = itStart->second;

    requestStart_.erase(itStart);
    requestKinds_.erase(itKind);
    requestAddrs_.erase(itAddr);

    return info;
}

void RequestSubmissionService::clearAll() {
    requestStart_.clear();
    requestKinds_.clear();
    requestAddrs_.clear();
}

int RequestSubmissionService::nextRequestId() {
    if (requestId_ == std::numeric_limits<int>::max()) {
        requestId_ = 0;
    }
    return ++requestId_;
}

void RequestSubmissionService::trackRequest(int requestId, RequestKind kind, uint16_t addr) {
    requestStart_[requestId] = std::chrono::steady_clock::now();
    requestKinds_[requestId] = kind;
    requestAddrs_[requestId] = addr;
    emit txCountUpdated();
}

} // namespace ui::application::modbus
