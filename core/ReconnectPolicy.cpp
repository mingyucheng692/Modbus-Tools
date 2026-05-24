/**
 * @file ReconnectPolicy.cpp
 * @brief Implementation of ReconnectPolicy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ReconnectPolicy.h"

namespace io {

ReconnectPolicy::ReconnectPolicy(int maxRetries, int delayMs)
    : maxRetries_(maxRetries)
    , delayMs_(delayMs)
{
}

bool ReconnectPolicy::exhausted() const
{
    return maxRetries_ > 0 && attemptCount_ >= maxRetries_;
}

bool ReconnectPolicy::shouldRetry() const
{
    if (maxRetries_ <= 0) {
        return false;
    }
    return attemptCount_ < maxRetries_;
}

void ReconnectPolicy::onFailed()
{
    if (attemptCount_ < maxRetries_ || maxRetries_ <= 0) {
        ++attemptCount_;
    }
}

void ReconnectPolicy::onSuccess()
{
    reset();
}

void ReconnectPolicy::reset()
{
    attemptCount_ = 0;
}

int ReconnectPolicy::attemptCount() const
{
    return attemptCount_;
}

int ReconnectPolicy::nextDelayMs() const
{
    return delayMs_;
}

void ReconnectPolicy::setMaxRetries(int maxRetries)
{
    maxRetries_ = maxRetries;
}

void ReconnectPolicy::setDelayMs(int delayMs)
{
    delayMs_ = delayMs;
}

int ReconnectPolicy::maxRetries() const
{
    return maxRetries_;
}

int ReconnectPolicy::delayMs() const
{
    return delayMs_;
}

QString ReconnectPolicy::statusString() const
{
    return QStringLiteral("Attempt %1/%2")
        .arg(attemptCount_)
        .arg(maxRetries_ > 0 ? QString::number(maxRetries_) : QStringLiteral("\u221E"));
}

} // namespace io