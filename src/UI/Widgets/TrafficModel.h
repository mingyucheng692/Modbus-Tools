#pragma once
#include <QAbstractListModel>
#include "Core/Logging/TrafficLog.h"

class TrafficModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit TrafficModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private slots:
    void onFrameAdded(size_t index);
    void onCleared();
};
