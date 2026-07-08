/**
 * @file TrafficEvent.h
 * @brief Unified traffic event model for monitor rendering.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

namespace ui::common {

enum class TrafficDirection {
    None,
    Tx,
    Rx
};

enum class TrafficEventLevel {
    Info,
    Warning,
    Error
};

enum class TrafficRequestType {
    Unknown,
    ManualRead,
    ManualWrite,
    Poll,
    RawSend,
    Connection
};

inline const char* toString(TrafficDirection d) {
    switch (d) {
    case TrafficDirection::Tx: return "Tx";
    case TrafficDirection::Rx: return "Rx";
    default: return "None";
    }
}

inline const char* toString(TrafficRequestType t) {
    switch (t) {
    case TrafficRequestType::ManualRead:  return "ManualRead";
    case TrafficRequestType::ManualWrite: return "ManualWrite";
    case TrafficRequestType::Poll:        return "Poll";
    case TrafficRequestType::RawSend:     return "RawSend";
    case TrafficRequestType::Connection:  return "Connection";
    default: return "Unknown";
    }
}

struct TrafficEvent {
    QDateTime timestamp = QDateTime::currentDateTimeUtc();
    TrafficDirection direction = TrafficDirection::None;
    TrafficEventLevel level = TrafficEventLevel::Info;
    TrafficRequestType requestType = TrafficRequestType::Unknown;
    bool isPoll = false;
    QString summary;
    QByteArray payload;
};

} // namespace ui::common
