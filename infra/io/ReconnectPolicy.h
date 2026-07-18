/**
 * @file ReconnectPolicy.h
 * @brief Header file for ReconnectPolicy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QStringList>

namespace io {

class ReconnectPolicy {
public:
    explicit ReconnectPolicy(int maxRetries = 3, int delayMs = 3000);

    bool exhausted() const;
    bool shouldRetry() const;
    void onFailed();
    void onSuccess();
    void reset();

    int attemptCount() const;
    int nextDelayMs() const;

    void setMaxRetries(int maxRetries);
    void setDelayMs(int delayMs);

    int maxRetries() const;
    int delayMs() const;

    QString statusString() const;

private:
    int maxRetries_ = 3;
    int delayMs_ = 3000;
    int attemptCount_ = 0;
};

} // namespace io
