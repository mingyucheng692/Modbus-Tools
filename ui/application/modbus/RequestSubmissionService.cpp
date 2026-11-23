// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "RequestSubmissionService.h"
#include <QCoreApplication>
#include <limits>

namespace ui::application::modbus {

RequestSubmissionService::RequestBuildResult RequestSubmissionService::buildReadRequest(
    const PollSpec& spec, RequestKind kind) {
    RequestBuildResult result;

    using namespace ::modbus::base;

    QString error;
    std::optional<Pdu> buildResult;
    if (function_traits::isWriteOperation(spec.functionCode)) {
        buildResult = pdu_builder::buildWriteRequest(
            static_cast<FunctionCode>(spec.functionCode),
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            spec.writeData,
            &error);
    } else {
        buildResult = pdu_builder::buildReadRequest(
            static_cast<FunctionCode>(spec.functionCode),
            static_cast<int>(spec.startAddress),
            static_cast<int>(spec.quantity),
            &error);
    }
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
    const ModbusExecuteSpec& spec, ::modbus::base::ModbusMode mode) const {
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
    const ModbusExecuteSpec& spec, ::modbus::base::ModbusMode mode) const {
    if (spec.isRawMode) {
        if (spec.rawAdu.isEmpty()) {
            return {false, QCoreApplication::translate("RequestSubmissionService",
                "Raw data cannot be empty")};
        }
        if (spec.rawAdu.size() > config::Modbus::kMaxAduSize) {
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
    const ModbusExecuteSpec& spec, RequestKind kind) {
    RequestBuildResult result;
    auto val = validateSpec(spec);
    if (!val.isValid) {
        result.ok = false;
        result.error = val.errorMessage;
        return result;
    }

    using namespace ::modbus::base;
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
    requestTracking_[requestId] = info;
    if (onTxCountUpdated) {
        onTxCountUpdated();
    }
}

} // namespace ui::application::modbus
