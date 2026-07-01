/**
 * @file SettingsService.h
 * @brief Header file for SettingsService.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "IAppConfig.h"
#include "ISettingsService.h"

#include <QObject>
#include <QHash>
#include <QSet>
#include <memory>

class QTimer;

namespace ui::common {

class SettingsService : public QObject, public ui::common::ISettingsService {
    Q_OBJECT

public:
    explicit SettingsService(QObject* parent = nullptr,
                             std::unique_ptr<IAppConfig> appConfig = {});

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
    QSet<QString> loadedKeys_;
    QSet<QString> dirtyKeys_;
    QSet<QString> keysToRemove_;
    std::unique_ptr<IAppConfig> appConfig_;
    QTimer* syncTimer_ = nullptr;
};

} // namespace ui::common
