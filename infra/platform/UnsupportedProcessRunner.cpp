/**
 * @file UnsupportedProcessRunner.cpp
 * @brief Implements the default non-supported elevated process launcher.
 */

#include "infra/platform/UnsupportedProcessRunner.h"

#include "infra/platform/PlatformInfo.h"

namespace infra::platform {

bool UnsupportedProcessRunner::startElevated(const QString& executablePath,
                                             const QStringList& arguments,
                                             QString* errorMessage)
{
    Q_UNUSED(executablePath);
    Q_UNUSED(arguments);

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Elevated process launch is not implemented for %1 in the infra/platform skeleton.")
                            .arg(platformDisplayName());
    }

    return false;
}

} // namespace infra::platform
