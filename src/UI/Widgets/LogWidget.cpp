#include "LogWidget.h"
#include <QVBoxLayout>
#include "Core/Logging/LogManager.h"

LogWidget::LogWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    listView_ = new QListView(this);
    model_ = new LogModel(this);
    listView_->setModel(model_);
    listView_->setUniformItemSizes(true);
    
    // Auto scroll logic (simplified)
    listView_->setAutoScroll(true);

    layout->addWidget(listView_);
    
    // Connect to LogManager
    if (auto sink = LogManager::instance().getQtSinkBase()) {
        connect(sink, &QtLogSinkBase::logReceived, this, &LogWidget::onLogReceived);
    }
}

void LogWidget::onLogReceived(const QString& msg, int level) {
    model_->addLog(msg, level);
    listView_->scrollToBottom();
}
