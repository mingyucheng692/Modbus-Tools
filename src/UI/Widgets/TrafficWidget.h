#pragma once
#include <QWidget>
#include <QListView>
#include "TrafficModel.h"

class TrafficWidget : public QWidget {
    Q_OBJECT
public:
    explicit TrafficWidget(QWidget* parent = nullptr);

signals:
    void frameSelected(size_t index);
    
private:
    QListView* listView_;
    TrafficModel* model_;
};
