/**
 * @file SettingsService.h
 * @brief Header file for SettingsService.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../../core/common/ISettingsService.h"

#include <QObject>
#include <QHash>
#include <QSet>
#include <QSettings>
#include <memory>

class QTimer;

namespace infra::platform { class PathResolver; }

namespace ui::common {

class SettingsService : public QObject, public ::core::common::ISettingsService {
    Q_OBJECT

public:
    explicit SettingsService(infra::platform::PathResolver& pathResolver,
                             QObject* parent = nullptr);

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
    // dirtyKeys_ tracks keys pending batch write-back (debounced QTimer sync).
    // keysToRemove_ tracks legacy keys pending deletion (e.g. kLegacySerialBaudRate).
    // QSettings does not provide dirty/removal set queries, so these sets carry
    // real semantics distinct from values_.
    QSet<QString> dirtyKeys_;
    QSet<QString> keysToRemove_;
    std::unique_ptr<QSettings> settings_;
    QTimer* syncTimer_ = nullptr;
    infra::platform::PathResolver& pathResolver_;
};

} // namespace ui::common
