#pragma once

#include "ISettingsService.h"
#include <QObject>
#include <QHash>
#include <QSet>

class QTimer;

namespace ui::common {

class SettingsService : public QObject, public ISettingsService {
    Q_OBJECT

public:
    explicit SettingsService(QObject* parent = nullptr);

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
    QTimer* syncTimer_ = nullptr;
};

} // namespace ui::common
