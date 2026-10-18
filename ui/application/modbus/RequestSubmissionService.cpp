// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "RequestSubmissionService.h"
#include "modbus/base/ModbusEndianCodec.h"
#include "common/ModbusDataHelper.h"
#include <QRegularExpression>
#include <QCoreApplication>
#include <limits>

namespace ui::application::modbus {

RequestSubmissionService::RequestBuildResult RequestSubmissionService::buildReadRequest(
    const PollSpec& spec, RequestKind kind) {
    RequestBuildResult result;

    using namespace ::ui::common;
    using namespace ::modbus::base;

    QString error;
    auto buildResult = pdu_builder::buildReadRequest(
        static_cast<FunctionCode>(spec.functionCode),
        static_cast<int>(spec.startAddress),
        static_cast<int>(spec.quantity),
        &error);
    if (!buildResult) {
        result.ok = false;
        result.error = error;
        return result;
    }

    result.pdu = std::move(*buildResult);
    result.requestId = nextRequestId();
    result.traceId = nextTraceId();
    trackRequest(result.requestId, kind, spec.startAddress, result.traceId);
    result.ok = true;
    return result;
}

RequestSubmissionService::RequestBuildResult RequestSubmissionService::buildWriteRequest(
    uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId, int quantity) {
    RequestBuildResult result;

    using namespace ::ui::common;
    using namespace ::modbus::base;

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
            QByteArray bytes = data_helper::parseHex(trimmed);
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
        appendBigEndian(rawBytes, coilVal);
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
            using namespace ::modbus::base;
            appendBigEndian(rawBytes, static_cast<uint16_t>(value));
        } else if (fmt == QStringLiteral("Binary")) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService",
                "Binary format not supported for registers (0x06)");
            return result;
        } else {
            QByteArray bytes = data_helper::parseHex(trimmed);
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
            rawBytes = data_helper::parseBinary(bits);
        } else if (fmt == QStringLiteral("Hex")) {
            rawBytes = data_helper::parseHex(trimmed);
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
            rawBytes = data_helper::parseDecimalList(trimmed, okList);
            if (!okList) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid decimal list for 0x10");
                return result;
            }
        } else {
            rawBytes = data_helper::parseHex(trimmed);
            if (rawBytes.isEmpty() || (rawBytes.size() % 2 != 0)) {
                result.ok = false;
                result.error = QCoreApplication::translate("RequestSubmissionService",
                    "Invalid hex value for 0x10");
                return result;
            }
        }
    }

    // Delegate PDU building to pdu_builder
    QString error;
    auto buildResult = pdu_builder::buildWriteRequest(
        static_cast<FunctionCode>(fc),
        addr,
        quantity,
        rawBytes,
        &error);
    if (!buildResult) {
        result.ok = false;
        result.error = error;
        return result;
    }

    result.pdu = std::move(*buildResult);
    result.requestId = nextRequestId();
    result.traceId = nextTraceId();
    trackRequest(result.requestId, RequestKind::Write, static_cast<uint16_t>(addr), result.traceId);
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
    auto it = requestTracking_.find(requestId);
    if (it == requestTracking_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<RequestTrackingInfo> RequestSubmissionService::lookupAndRemove(int requestId) {
    auto it = requestTracking_.find(requestId);
    if (it == requestTracking_.end()) {
        return std::nullopt;
    }

    RequestTrackingInfo info = it->second;
    requestTracking_.erase(it);
    return info;
}

void RequestSubmissionService::clearAll() {
    requestTracking_.clear();
}

int RequestSubmissionService::nextRequestId() {
    if (requestId_ == std::numeric_limits<int>::max()) {
        requestId_ = 0;
    }
    return ++requestId_;
}

TraceId RequestSubmissionService::nextTraceId() {
    return traceIdCounter_.fetch_add(1, std::memory_order_relaxed);
}

RequestSubmissionService::ValidationResult RequestSubmissionService::validateBroadcastGate(
    const ModbusExecuteSpec& spec, SessionMode mode) const {
    Q_UNUSED(mode);
    if (spec.slaveId > config::Modbus::kMaxSlaveId) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Slave ID (%1) exceeds maximum (%2)").arg(spec.slaveId).arg(config::Modbus::kMaxSlaveId)};
    }
    if (spec.slaveId == 0) {
        if (!::modbus::base::function_traits::isBroadcastAllowed(0, spec.functionCode)) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "Broadcast read is forbidden per IEC 61158")};
        }
    }
    return {true, QString()};
}

RequestSubmissionService::ValidationResult RequestSubmissionService::validateQuantityAndRangeGate(
    const ModbusExecuteSpec& spec) const {
    using namespace ::modbus::base;
    if (!function_traits::isReadOperation(spec.functionCode) &&
        !function_traits::isWriteOperation(spec.functionCode)) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Unsupported function code: %1").arg(spec.functionCode)};
    }

    const uint16_t maxQty = function_traits::maxAllowedQuantity(spec.functionCode);
    if (spec.quantity < config::Modbus::kMinQuantity) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Quantity must be at least 1")};
    }
    if (maxQty > 0 && spec.quantity > maxQty) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Quantity (%1) exceeds maximum allowed (%2) for function code %3")
            .arg(spec.quantity).arg(maxQty).arg(spec.functionCode)};
    }
    if (function_traits::isSingleWrite(spec.functionCode) && spec.quantity != 1) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Single write quantity must be 1")};
    }

    const uint32_t endAddress = static_cast<uint32_t>(spec.startAddress) + spec.quantity;
    if (endAddress > 65536U) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Address range overflow: start=%1, quantity=%2 exceeds 65536")
            .arg(spec.startAddress).arg(spec.quantity)};
    }
    return {true, QString()};
}

RequestSubmissionService::ValidationResult RequestSubmissionService::validateWirePayloadGate(
    const ModbusExecuteSpec& spec) const {
    using namespace ::modbus::base;
    if (spec.functionCode == 0x05) {
        if (spec.writeData.size() != 2) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "FC 05 write data payload must be exactly 2 bytes")};
        }
        const uint16_t coilVal = (static_cast<uint8_t>(spec.writeData[0]) << 8) |
                                  static_cast<uint8_t>(spec.writeData[1]);
        if (coilVal != 0x0000 && coilVal != 0xFF00) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "FC 05 coil value must strictly be 0x0000 (OFF) or 0xFF00 (ON)")};
        }
    } else if (spec.functionCode == 0x06) {
        if (spec.writeData.size() != 2) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "FC 06 write data payload must be exactly 2 bytes")};
        }
    } else if (spec.functionCode == 0x0F) {
        const int expectedBytes = (spec.quantity + 7) / 8;
        if (spec.writeData.size() != expectedBytes) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "FC 0F write coils payload size (%1) does not match expected (%2) bytes")
                .arg(spec.writeData.size()).arg(expectedBytes)};
        }
    } else if (spec.functionCode == 0x10) {
        const int expectedBytes = spec.quantity * 2;
        if (spec.writeData.size() != expectedBytes) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "FC 10 write registers payload size (%1) does not match expected (%2) bytes")
                .arg(spec.writeData.size()).arg(expectedBytes)};
        }
    }

    int wirePdu = 5;
    if (function_traits::isMultipleWrite(spec.functionCode)) {
        wirePdu = 6 + spec.writeData.size();
    }
    if (wirePdu > 253) {
        return {false, QCoreApplication::translate("RequestSubmissionService",
            "Wire PDU length (%1 bytes) exceeds maximum 253 bytes").arg(wirePdu)};
    }
    return {true, QString()};
}

RequestSubmissionService::ValidationResult RequestSubmissionService::validateSpec(
    const ModbusExecuteSpec& spec, SessionMode mode) const {
    if (spec.isRawMode) {
        if (spec.rawPduOrAdu.isEmpty()) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "Raw data cannot be empty")};
        }
        if (spec.rawPduOrAdu.size() > config::Modbus::kMaxAduSize) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "Raw frame exceeds maximum size (%1 bytes)").arg(config::Modbus::kMaxAduSize)};
        }
        return {true, QString()};
    }

    auto g1 = validateBroadcastGate(spec, mode);
    if (!g1.isValid) return g1;

    auto g2 = validateQuantityAndRangeGate(spec);
    if (!g2.isValid) return g2;

    return validateWirePayloadGate(spec);
}

RequestSubmissionService::RequestBuildResult RequestSubmissionService::buildExecuteRequest(
    const ModbusExecuteSpec& spec, SessionMode mode, RequestKind kind) {
    RequestBuildResult result;
    auto val = validateSpec(spec, mode);
    if (!val.isValid) {
        result.ok = false;
        result.error = val.errorMessage;
        return result;
    }

    using namespace ::modbus::base;
    if (spec.isRawMode) {
        if (spec.rawPduOrAdu.size() < 1) {
            result.ok = false;
            result.error = QCoreApplication::translate("RequestSubmissionService", "Raw data too short");
            return result;
        }
        const FunctionCode fc = static_cast<FunctionCode>(static_cast<uint8_t>(spec.rawPduOrAdu[0]));
        result.pdu = Pdu(fc, spec.rawPduOrAdu.mid(1));
        result.requestId = nextRequestId();
        result.traceId = nextTraceId();
        trackRequest(result.requestId, RequestKind::Raw, 0, result.traceId);
        result.ok = true;
        return result;
    }

    QString error;
    std::optional<Pdu> pduOpt;
    if (function_traits::isReadOperation(spec.functionCode)) {
        pduOpt = pdu_builder::buildReadRequest(
            static_cast<FunctionCode>(spec.functionCode),
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            &error);
    } else {
        pduOpt = pdu_builder::buildWriteRequest(
            static_cast<FunctionCode>(spec.functionCode),
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            spec.writeData,
            &error);
    }

    if (!pduOpt) {
        result.ok = false;
        result.error = error;
        return result;
    }

    result.pdu = std::move(*pduOpt);
    result.requestId = nextRequestId();
    result.traceId = nextTraceId();
    trackRequest(result.requestId, kind, spec.startAddress, result.traceId);
    result.ok = true;
    return result;
}

void RequestSubmissionService::trackRequest(int requestId, RequestKind kind, uint16_t addr,
                                            TraceId traceId) {
    RequestTrackingInfo info;
    info.kind = kind;
    info.address = addr;
    info.traceId = traceId;
    info.startTime = std::chrono::steady_clock::now();
    requestTracking_[requestId] = info;
    if (onTxCountUpdated) {
        onTxCountUpdated();
    }
}

} // namespace ui::application::modbus
