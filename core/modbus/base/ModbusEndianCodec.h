/**
 * @file ModbusEndianCodec.h
 * @brief Big-endian read helpers shared across the Modbus stack.
 *
 * Consolidates the three previously duplicated readBigEndianUInt16 implementations
 * (ModbusProtocolChecks.cpp, ModbusFrameParser.cpp, RequestValidator.cpp) into a
 * single template. Semantics are identical to the originals: bounds-checked,
 * host-endian output via qFromBigEndian, no exception on out-of-range access.
 */

#pragma once

#include <QByteArrayView>
#include <QtEndian>
#include <cstring>

namespace modbus::base {

/**
 * @brief Reads a big-endian integer of type @p T from @p data at @p offset.
 *
 * @tparam T   Integer type to read (e.g. uint16_t, uint32_t).
 * @param data   Source buffer view.
 * @param offset Byte offset within @p data where the value begins.
 * @param value  Out-parameter receiving the host-endian value on success.
 *               Left unchanged on failure.
 * @return true if @p offset is in range and @p data has at least sizeof(T) bytes
 *         from @p offset; false otherwise.
 */
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

} // namespace modbus::base
