// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QString>
#include <QList>

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

} // namespace ui::common::data_helper
