// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QList>
#include <QString>

#include <QByteArray>

#include <deque>

namespace ui::widgets {

struct LogEntry {
    enum class Kind {
        Info,
        Tx,
        Rx,
        Warn,
        Error,
    };

    QString text;
    QColor color = Qt::gray;
    Kind kind = Kind::Info;
    QByteArray rawBytes;
    QString message;
    QString timestampText;
    QString clientLabel;
};

class LogListModel final : public QAbstractListModel {
    Q_OBJECT

public:
    explicit LogListModel(int maxBlockCount, QObject* parent = nullptr);
    explicit LogListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void appendEntries(const QList<LogEntry>& newEntries);
    void clearAll();
    void replaceEntries(const QList<LogEntry>& newEntries);
    QString lineAt(int row) const;
    LogEntry entryAt(int row) const;
    void setMaxBlockCount(int count);

private:
    std::deque<LogEntry> entries_;
    int maxBlockCount_;
};

} // namespace ui::widgets
