/**
 * @file PendingSlaveTracker.cpp
 * @brief Implementation of PendingSlaveTracker.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "PendingSlaveTracker.h"

namespace modbus::transport {

void PendingSlaveTracker::setPending(uint8_t slaveId) {
    std::lock_guard<std::mutex> lock(mutex_);
    expectedResponseSlaveId_ = slaveId;
    hasPendingRequest_ = true;
}

PendingSlaveTracker::Outcome PendingSlaveTracker::check(uint8_t slaveId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!hasPendingRequest_) {
        return Outcome::NoPending;
    }
    if (slaveId != expectedResponseSlaveId_) {
        return Outcome::SlaveMismatch;
    }
    hasPendingRequest_ = false;
    expectedResponseSlaveId_ = 0;
    return Outcome::Matched;
}

uint8_t PendingSlaveTracker::expectedSlaveId() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return expectedResponseSlaveId_;
}

void PendingSlaveTracker::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    hasPendingRequest_ = false;
    expectedResponseSlaveId_ = 0;
}

} // namespace modbus::transport
