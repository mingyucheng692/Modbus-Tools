#pragma once
#include <QAbstractListModel>
#include <vector>
#include <QColor>

struct LogEntry {
    QString message;
    int level; // 0=Trace, 1=Debug, 2=Info, 3=Warn, 4=Error, 5=Critical
};

class LogModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit LogModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void addLog(const QString& msg, int level);
    void clear();

private:
    std::vector<LogEntry> logs_;
};
