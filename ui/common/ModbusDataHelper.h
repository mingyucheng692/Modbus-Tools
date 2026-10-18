// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QString>
#include <QList>
#include <optional>
#include "common/Result.h"

namespace ui::common::data_helper {

// Parsing utilities for HEX, Decimal, and Binary strings across UI views.

// Parses HEX string into QByteArray (ignores non-HEX characters).
QByteArray parseHex(const QString& input);

// Parses list of decimal numbers into Big-Endian 16-bit register bytes.
QByteArray parseDecimalList(const QString& input, bool& ok);

// Parses string of bits ('0'/'1') into LSB-first coil bytes.
QByteArray parseBinary(const QString& input);

// Parses string into integer with smart format detection (HEX 0x/H suffix, DEC).
int parseSmartInt(const QString& input, bool* ok = nullptr);

// Converts user-pasted text to clean target-radix representation.
[[nodiscard]] std::optional<QString> convertPastedText(const QString& text, int targetRadix) noexcept;

// Converts numbers in text between bases (10 and 16).
[[nodiscard]] QString convertRadix(const QString& text, int fromRadix, int toRadix) noexcept;

// Validates and packs bit string ('0'/'1') into LSB-first byte array.
[[nodiscard]] ::common::Result<QByteArray, QString> packCoilBits(const QString& bitStr, int expectedQuantity) noexcept;

// Validates and parses register values into big-endian byte array.
[[nodiscard]] ::common::Result<QByteArray, QString> parseRegisterWords(const QString& text, int radix, int expectedQuantity) noexcept;

} // namespace ui::common::data_helper
