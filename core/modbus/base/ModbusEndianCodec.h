// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArrayView>
#include <QtEndian>
#include <cstring>

namespace modbus::base {

// Returns true if offset is in range and buffer has at least sizeof(T) bytes; value unchanged on failure.
template <typename T>
[[nodiscard]] bool readBigEndian(QByteArrayView data, qsizetype offset, T& value) noexcept
{
    if (offset < 0 || offset + static_cast<qsizetype>(sizeof(T)) > data.size()) {
        return false;
    }
    T rawValue = 0;
    std::memcpy(&rawValue, data.constData() + offset, sizeof(T));
    value = qFromBigEndian(rawValue);
    return true;
}

// Caller must ensure destination buffer has at least sizeof(T) bytes available from offset.
template <typename T>
void writeBigEndian(QByteArray& data, qsizetype offset, T value) noexcept
{
    const T encoded = qToBigEndian(value);
    std::memcpy(data.data() + offset, &encoded, sizeof(T));
}

// Encodes value in big-endian byte order and appends sizeof(T) bytes to dest.
template <typename T>
void appendBigEndian(QByteArray& dest, T value) noexcept
{
    const T encoded = qToBigEndian(value);
    dest.append(reinterpret_cast<const char*>(&encoded), sizeof(T));
}

} // namespace modbus::base
