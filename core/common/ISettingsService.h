/**
 * @file ISettingsService.h
 * @brief Abstract settings storage interface (read/write/persist).
 *
 * Lives in core/common so that core-layer controllers (e.g. SettingsController)
 * can depend on it without creating a reverse dependency from core to ui.
 * ui::common re-exports this type as an alias for source compatibility.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QVariant>

namespace core::common {

class ISettingsService {
public:
    virtual ~ISettingsService() = default;

    virtual QVariant value(const QString& key) const = 0;
    virtual bool contains(const QString& key) const = 0;
    virtual void setValue(const QString& key, const QVariant& value) = 0;
    virtual QString configFilePath() const = 0;
    virtual void sync() = 0;
};

} // namespace core::common
