/**
 * @file IPlatformPaths.h
 * @brief Declares the platform-aware writable path abstraction.
 */

#pragma once

#include <QString>

namespace infra::platform {

class IPlatformPaths {
public:
    virtual ~IPlatformPaths() noexcept = default;

    [[nodiscard]] virtual QString appDataLocation() const = 0;
    [[nodiscard]] virtual QString appConfigLocation() const = 0;
    [[nodiscard]] virtual QString tempLocation() const = 0;
};

} // namespace infra::platform
