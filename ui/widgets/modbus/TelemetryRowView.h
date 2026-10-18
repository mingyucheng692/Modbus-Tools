// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QColor>
#include <QString>
#include <cstdint>
#include "common/TrafficEvent.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui::widgets::modbus {

/// Role: Lightweight row-level cache for telemetry table rendering and status derivation.
/// Thread-safety: Thread-compatible value structure.
/// Ownership: Value object stored in TelemetryTableModel ring buffer.
struct TelemetryRowView {
    uint64_t rowId = 0;              // Monotonic row ID to prevent ring buffer eviction index drift
    ::ui::common::TrafficEvent event;
    ::modbus::parser::ProtocolType protocol = ::modbus::parser::ProtocolType::Unknown;
    quint64 sessionGeneration = 0;
    ::modbus::parser::ParseResult parseResult; // Lightweight parse result cache
    QString statusText;
    QColor statusColor;
};

[[nodiscard]] inline void deriveRowStatus(const ::ui::common::TrafficEvent& event,
                                         const ::modbus::parser::ParseResult& parseResult,
                                         QString& outText,
                                         QColor& outColor) noexcept {
    using ::ui::common::TrafficDirection;
    using ::ui::common::TrafficEventLevel;
    using ::modbus::parser::ProtocolType;

    if (event.direction == TrafficDirection::None) {
        switch (event.level) {
        case TrafficEventLevel::Error:
            outText = QStringLiteral("Error");
            outColor = QColor(220, 53, 69);
            break;
        case TrafficEventLevel::Warning:
            outText = QStringLiteral("Warning");
            outColor = QColor(255, 193, 7);
            break;
        case TrafficEventLevel::Info:
        default:
            outText = QStringLiteral("Info");
            outColor = QColor(13, 110, 253);
            break;
        }
        return;
    }

    if (event.direction == TrafficDirection::Tx) {
        outText = QStringLiteral("-");
        outColor = QColor(108, 117, 125);
        return;
    }

    // Rx handling
    if (parseResult.isException) {
        outText = QStringLiteral("Exception (0x%1)")
                      .arg(static_cast<int>(parseResult.exceptionCode), 2, 16, QChar('0')).toUpper();
        outColor = QColor(220, 53, 69);
        return;
    }

    if (!parseResult.isValid) {
        if (parseResult.protocol == ProtocolType::Rtu && !parseResult.checksumValid) {
            outText = QStringLiteral("CRC Error");
            outColor = QColor(220, 53, 69);
        } else if (parseResult.protocol == ProtocolType::Ascii && !parseResult.checksumValid) {
            outText = QStringLiteral("LRC Error");
            outColor = QColor(220, 53, 69);
        } else {
            outText = QStringLiteral("Malformed");
            outColor = QColor(255, 140, 0);
        }
        return;
    }

    outText = QStringLiteral("Success");
    outColor = QColor(25, 135, 84);
}

} // namespace ui::widgets::modbus

