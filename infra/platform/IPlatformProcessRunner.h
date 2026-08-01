/**
 * @file IPlatformProcessRunner.h
 * @brief Declares the elevated and non-elevated process launch abstraction.
 */

#pragma once

#include <QString>
#include <QStringList>

namespace infra::platform {

class IPlatformProcessRunner {
public:
    virtual ~IPlatformProcessRunner() noexcept = default;

    [[nodiscard]] virtual bool supportsElevatedLaunch() const noexcept = 0;
    [[nodiscard]] virtual bool startElevated(const QString& executablePath,
                                             const QStringList& arguments,
                                             QString* errorMessage) = 0;

    /// Launches a process without elevation (CreateProcessW on Windows).
    /// Returns true on success. Callers should fall back to startElevated()
    /// when this returns false and elevation is acceptable.
    [[nodiscard]] virtual bool startNonElevated(const QString& executablePath,
                                                const QStringList& arguments,
                                                QString* errorMessage) = 0;
};

} // namespace infra::platform
