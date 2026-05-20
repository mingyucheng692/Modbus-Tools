/**
 * @file LogListModel.h
 * @brief Shared log list model for traffic/byte monitor widgets.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QList>
#include <QString>

namespace ui::widgets {

struct LogEntry {
    QString text;
    QColor color = Qt::gray;
};

class LogListModel final : public QAbstractListModel {
public:
    explicit LogListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void appendEntries(const QList<LogEntry>& newEntries);
    void clearAll();
    void replaceEntries(const QList<LogEntry>& newEntries);
    QString lineAt(int row) const;

private:
    QList<LogEntry> entries_;
};

} // namespace ui::widgets
