#include "TrafficMonitorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QDateTime>
#include <QGroupBox>
#include <QListWidgetItem>

namespace ui::widgets {

TrafficMonitorWidget::TrafficMonitorWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TrafficMonitorWidget::~TrafficMonitorWidget() = default;

void TrafficMonitorWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto groupBox = new QGroupBox("Traffic Monitor", this);
    auto layout = new QVBoxLayout(groupBox);

    // Toolbar
    auto toolbarLayout = new QHBoxLayout();
    
    displayModeBox_ = new QComboBox(this);
    displayModeBox_->addItem("Hex View");
    displayModeBox_->addItem("ASCII View");
    toolbarLayout->addWidget(displayModeBox_);

    autoScrollCheck_ = new QCheckBox("Auto Scroll", this);
    autoScrollCheck_->setChecked(true);
    toolbarLayout->addWidget(autoScrollCheck_);

    clearBtn_ = new QPushButton("Clear", this);
    toolbarLayout->addWidget(clearBtn_);
    
    toolbarLayout->addStretch();
    layout->addLayout(toolbarLayout);

    // Log List
    logList_ = new QListWidget(this);
    logList_->setAlternatingRowColors(true);
    layout->addWidget(logList_);

    mainLayout->addWidget(groupBox);

    // Connections
    connect(clearBtn_, &QPushButton::clicked, this, &TrafficMonitorWidget::clear);
    connect(displayModeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){
        // In a real app, we might want to refresh the list with new format.
        // For now, it only affects new items.
        // Ideally we store raw data and re-render.
        // But for simplicity in Phase 3, we just keep it as is.
    });
}

void TrafficMonitorWidget::appendTx(const QByteArray& data) {
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString content = formatData(data);
    QString itemText = QString("[%1] [TX] %2").arg(timeStr, content);
    
    auto item = new QListWidgetItem(itemText);
    item->setForeground(Qt::blue); // TX in Blue
    logList_->addItem(item);

    if (autoScrollCheck_->isChecked()) {
        logList_->scrollToBottom();
    }
}

void TrafficMonitorWidget::appendRx(const QByteArray& data) {
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString content = formatData(data);
    QString itemText = QString("[%1] [RX] %2").arg(timeStr, content);
    
    auto item = new QListWidgetItem(itemText);
    item->setForeground(Qt::darkGreen); // RX in Green
    logList_->addItem(item);

    if (autoScrollCheck_->isChecked()) {
        logList_->scrollToBottom();
    }
}

void TrafficMonitorWidget::appendInfo(const QString& message) {
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString itemText = QString("[%1] [INFO] %2").arg(timeStr, message);
    
    auto item = new QListWidgetItem(itemText);
    item->setForeground(Qt::gray);
    logList_->addItem(item);

    if (autoScrollCheck_->isChecked()) {
        logList_->scrollToBottom();
    }
}

void TrafficMonitorWidget::clear() {
    logList_->clear();
}

QString TrafficMonitorWidget::formatData(const QByteArray& data) const {
    if (displayModeBox_->currentIndex() == 1) { // ASCII
        return QString::fromLatin1(data);
    }
    // Hex Mode
    return QString(data.toHex(' ').toUpper());
}

} // namespace ui::widgets
