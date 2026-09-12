/**
 * @file SettingsService.h
 * @brief Header file for SettingsService.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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
    /// One legacy -> canonical config key prefix mapping (Phase 9 in-flight
    /// migration). Old prefixes are pairwise disjoint — enforced by the
    /// KeyPrefixMigrations_TableIsPairwiseDisjoint unit test.
    struct KeyPrefixMigration {
        const char* oldPrefix;
        const char* newPrefix;
    };

    static constexpr KeyPrefixMigration kKeyPrefixMigrations[] = {
        // Network Debugger: three legacy key subspaces -> three mode segments.
        {"tcp_client/",  "network_debugger/client/"},
        {"tcp_server/",  "network_debugger/server/"},
        {"udp/",         "network_debugger/udp/"},
        // Serial Debugger: single-mode page, no mode segment.
        {"serial_port/", "serial_debugger/"},
        // Historical one-off (formerly hardcoded kLegacySerialBaudRate block):
        // the pre-Modbus-RTU serial page stored its baud under "serial/".
        {"serial/",      "modbus/rtu/serial/"},
    };

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
    void migrateLegacyKeys();
    void scheduleSync();

    QHash<QString, QVariant> defaults_;
    QHash<QString, QVariant> values_;
    // dirtyKeys_ tracks keys pending batch write-back (debounced QTimer sync).
    // keysToRemove_ tracks legacy keys pending deletion (in-flight migration).
    // QSettings does not provide dirty/removal set queries, so these sets carry
    // real semantics distinct from values_.
    QSet<QString> dirtyKeys_;
    QSet<QString> keysToRemove_;
    std::unique_ptr<QSettings> settings_;
    QTimer* syncTimer_ = nullptr;
    infra::platform::PathResolver& pathResolver_;
};

} // namespace infra::config

namespace ui::common {
    // Alias for compatibility
    using SettingsService = ::infra::config::SettingsService;
} // namespace ui::common
