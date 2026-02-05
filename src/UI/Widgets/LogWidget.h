#pragma once
#include <QWidget>
#include <QListView>
#include "LogModel.h"

class LogWidget : public QWidget {
    Q_OBJECT
public:
    explicit LogWidget(QWidget* parent = nullptr);

private slots:
    void onLogReceived(const QString& msg, int level);

private:
    QListView* listView_;
    LogModel* model_;
};
