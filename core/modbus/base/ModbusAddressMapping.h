// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <cstdint>
#include <optional>

namespace modbus::address {

enum class AddressBase {
    Offset0Based = 0,
    PlcAddress1Based = 1
};

struct AddressMappingResult {
    uint16_t pduAddress = 0;
    bool isValid = false;
    QString errorMessage;
    std::optional<uint8_t> suggestedFunctionCode = std::nullopt;
};

// Translates user input address string to 0-based PDU address.
// Supports hexadecimal (0x... prefix or ...H suffix) and decimal strings.
// In PlcAddress1Based mode, handles Modicon 5-digit and 6-digit prefixes:
// - 40001-49999 / 400001-465536 -> Holding Register (FC03)
// - 30001-39999 / 300001-365536 -> Input Register (FC04)
// - 10001-19999 / 100001-165536 -> Discrete Input (FC02)
// - 00001-09999 / 000001-065536 -> Coil (FC01)
// Plain 1-based values (1 ~ 65536) map to PDU (input - 1).
[[nodiscard]] AddressMappingResult toPduAddress(const QString& inputAddress, AddressBase base);

[[nodiscard]] QString toDisplayAddress(uint16_t pduAddress, AddressBase base, bool asHex = false);

} // namespace modbus::address

