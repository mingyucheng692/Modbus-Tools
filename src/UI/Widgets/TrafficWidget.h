#pragma once
#include <QWidget>
#include <QListView>
#include "TrafficModel.h"

#include <QPushButton>
#include <QCheckBox>

class TrafficWidget : public QWidget {
    Q_OBJECT
public:
    explicit TrafficWidget(QWidget* parent = nullptr);

signals:
    void frameSelected(size_t index);
    
private slots:
    void onClearClicked();
    void onExportClicked();

private:
    QListView* listView_;
    TrafficModel* model_;
    QPushButton* clearBtn_;
    QPushButton* exportBtn_;
    QCheckBox* autoScrollCheck_;
};
