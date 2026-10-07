// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QVariant>

namespace infra::config {

class ISettingsService {
public:
    virtual ~ISettingsService() = default;

    virtual QVariant value(const QString& key) const = 0;
    virtual bool contains(const QString& key) const = 0;
    virtual void setValue(const QString& key, const QVariant& value) = 0;
    virtual QString configFilePath() const = 0;
    virtual void sync() = 0;
};

} // namespace infra::config
