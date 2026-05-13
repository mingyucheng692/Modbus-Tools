/**
 * @file BackoffCalculator.h
 * @brief Header file for BackoffCalculator.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <cstdint>

namespace modbus::session {

class BackoffCalculator {
public:
    struct Config {
        int baseIntervalMs = 100;
        int maxIntervalMs = 5000;
        double backoffFactor = 2.0;
        int jitterPercent = 20;
    };

    explicit BackoffCalculator(const Config& config);

    [[nodiscard]] int calculateMs(int attempt) const;

private:
    static int sanitizeDelayMs(int value);

    Config config_;
};

} // namespace modbus::session