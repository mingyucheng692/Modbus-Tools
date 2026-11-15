// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QByteArrayView>
#include <QMap>
#include <QCoreApplication>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/base/ModbusByteOrder.h"
#include "modbus/base/RegisterValueDecoder.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace modbus::analyzer::value_formatter {

enum class FrameDecodeCapability {
    FullRegisters,   ///< 0x03/0x04 RX response, 0x10 TX write request (continuous multi-registers)
    SingleRegister,  ///< 0x06 TX & RX (single 16-bit register)
    DiscreteBits,    ///< 0x01/0x02 RX, 0x05 TX/RX, 0x0F TX request (bit/coils)
    NoDataPayload    ///< Read requests (0x01..0x04 TX), Write echoes (0x0F/0x10 RX), exceptions, SYS/invalid
};

struct FrameDecodeEvaluation {
    FrameDecodeCapability capability = FrameDecodeCapability::NoDataPayload;
    bool enableDecodeCombo = false;
    bool enableOrderCombo = false;
    bool allowMultiWordTypes = false;
    QString capabilityHint;
    QByteArray registerBytes;
    uint16_t startAddress = 0;
    int registerCount = 0;
};

// Role: Stateless formatting utilities converting raw and decoded Modbus register values to human-readable strings.
// Thread-safety: Thread-safe (reentrant pure functions).
// Ownership: None (namespace of free functions).

[[nodiscard]] FrameDecodeEvaluation evaluateFrameDecodeCapability(
    const ::modbus::parser::ParseResult& pr,
    bool isTx
);

[[nodiscard]] QString formatDecodedSummary(
    const ::modbus::parser::ParseResult& pr,
    const FrameDecodeEvaluation& eval,
    RegisterDataType type,
    modbus::base::RegisterOrder order,
    const QMap<uint16_t, DataMetadata>& metadata = {}
);

QString formatDecimalValue(const QVariant& value, NumberDisplayMode mode);

double numericValueForDisplay(const QVariant& value, NumberDisplayMode mode, bool* ok = nullptr);

QString formatScaledValue(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

QString formatScaledValue(QByteArrayView rawBytes, const DataMetadata& meta, RegisterDataType type, modbus::base::RegisterOrder order);

QString buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

QString buildDescriptionTooltip(QByteArrayView rawBytes, const DataMetadata& meta, RegisterDataType type, modbus::base::RegisterOrder order);

QString formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex);

QString formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary);

} // namespace modbus::analyzer::value_formatter

namespace modbus::codec {

// Formats decoded register value multiplied by scale. Hosted in analyzer
// (its only consumer) while staying in the modbus::codec namespace for
// source compatibility with former base-layer call sites.
// Guarantees that if decoded.isNanOrInf is true, multiplication is bypassed
// to avoid NaN pollution.
[[nodiscard]] QString formatEngineeringValue(
    const DecodedRegisterValue& decoded,
    double scale,
    modbus::analyzer::RegisterDataType type
);

} // namespace modbus::codec
