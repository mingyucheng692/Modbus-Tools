// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "ISettingsService.h"

#include <QObject>
#include <QHash>
#include <QSet>
#include <QSettings>
#include <memory>

class QTimer;

namespace infra::platform { class PathResolver; }

namespace infra::config {

class SettingsService : public QObject, public ISettingsService {
    Q_OBJECT

public:
    explicit SettingsService(infra::platform::PathResolver& pathResolver,
                             QObject* parent = nullptr);
    ~SettingsService() noexcept override;

    QVariant value(const QString& key) const override;
    bool contains(const QString& key) const override;
    void setValue(const QString& key, const QVariant& value) override;
    QString configFilePath() const override;
    void sync() override;

private:
    void initializeDefaults();
    void load();
    void scheduleSync();

    QHash<QString, QVariant> defaults_;
    QHash<QString, QVariant> values_;
    QSet<QString> dirtyKeys_;
    std::unique_ptr<QSettings> settings_;
    QTimer* syncTimer_ = nullptr;
};

} // namespace infra::config
