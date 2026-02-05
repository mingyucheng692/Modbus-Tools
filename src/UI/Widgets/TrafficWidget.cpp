#include "TrafficWidget.h"
#include <QVBoxLayout>

TrafficWidget::TrafficWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    
    listView_ = new QListView(this);
    model_ = new TrafficModel(this);
    listView_->setModel(model_);
    listView_->setUniformItemSizes(true);
    listView_->setAutoScroll(true);
    
    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    listView_->setFont(font);

    layout->addWidget(listView_);
    
    connect(listView_->selectionModel(), &QItemSelectionModel::currentChanged, 
        [this](const QModelIndex& current, const QModelIndex&){
            if (current.isValid()) emit frameSelected(current.row());
        });
}
