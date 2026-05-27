/**
 * @file IPlatformProcessRunner.h
 * @brief Declares the elevated process launch abstraction.
 */

#pragma once

#include <QString>
#include <QStringList>

namespace infra::platform {

class IPlatformProcessRunner {
public:
    virtual ~IPlatformProcessRunner() noexcept = default;

    [[nodiscard]] virtual bool startElevated(const QString& executablePath,
                                             const QStringList& arguments,
                                             QString* errorMessage) = 0;
};

} // namespace infra::platform
