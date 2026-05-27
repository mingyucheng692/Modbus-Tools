/**
 * @file QtStandardPlatformPaths.cpp
 * @brief Implements the default writable path abstraction.
 */

#include "infra/platform/QtStandardPlatformPaths.h"

#include <QStandardPaths>

namespace infra::platform {

QString QtStandardPlatformPaths::appDataLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString QtStandardPlatformPaths::appConfigLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString QtStandardPlatformPaths::tempLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

} // namespace infra::platform
