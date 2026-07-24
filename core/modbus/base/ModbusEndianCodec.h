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

/**
 * @brief Writes a big-endian integer of type @p T into @p data at @p offset.
 *
 * Symmetric counterpart of readBigEndian.  The caller must ensure @p data
 * has at least sizeof(T) bytes available from @p offset (e.g. by calling
 * data.resize() beforehand).
 *
 * @tparam T   Integer type to write (e.g. uint16_t, uint32_t).
 * @param data   Destination byte array (must be pre-sized).
 * @param offset Byte offset within @p data where the value will be written.
 * @param value  Host-endian value to encode.
 */
template <typename T>
void writeBigEndian(QByteArray& data, qsizetype offset, T value) noexcept
{
    const T encoded = qToBigEndian(value);
    std::memcpy(data.data() + offset, &encoded, sizeof(T));
}

/**
 * @brief Appends a big-endian integer of type @p T to the end of @p dest.
 *
 * Convenience wrapper for dynamic-growth scenarios where the caller does not
 * want to pre-size the buffer.  Equivalent to encoding @p value in big-endian
 * byte order and appending the @c sizeof(T) bytes to @p dest.
 *
 * @tparam T   Integer type to append (e.g. uint8_t, uint16_t, uint32_t).
 * @param dest   Destination byte array (grows by sizeof(T) bytes).
 * @param value  Host-endian value to encode.
 */
template <typename T>
void appendBigEndian(QByteArray& dest, T value) noexcept
{
    const T encoded = qToBigEndian(value);
    dest.append(reinterpret_cast<const char*>(&encoded), sizeof(T));
}

} // namespace modbus::base
