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
    Warning
};

enum class TrafficRequestType {
    Unknown,
    ManualRead,
    ManualWrite,
    Poll,
    RawSend,
    Connection
};

struct TrafficEvent {
    QDateTime timestamp = QDateTime::currentDateTime();
    TrafficDirection direction = TrafficDirection::None;
    TrafficEventLevel level = TrafficEventLevel::Info;
    TrafficRequestType requestType = TrafficRequestType::Unknown;
    bool isPoll = false;
    QString summary;
    QByteArray payload;
};

} // namespace ui::common
