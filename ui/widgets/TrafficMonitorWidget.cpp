#include "TrafficMonitorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QDateTime>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QEvent>

namespace ui::widgets {

TrafficMonitorWidget::TrafficMonitorWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TrafficMonitorWidget::~TrafficMonitorWidget() = default;

void TrafficMonitorWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    groupBox_ = new QGroupBox(this);
    auto layout = new QVBoxLayout(groupBox_);

    // Toolbar
    auto toolbarLayout = new QHBoxLayout();
    
    autoScrollCheck_ = new QCheckBox(this);
    autoScrollCheck_->setChecked(true);
    toolbarLayout->addWidget(autoScrollCheck_);

    showTxCheck_ = new QCheckBox(this);
    showTxCheck_->setChecked(true);
    toolbarLayout->addWidget(showTxCheck_);

    showRxCheck_ = new QCheckBox(this);
    showRxCheck_->setChecked(true);
    toolbarLayout->addWidget(showRxCheck_);

    clearBtn_ = new QPushButton(this);
    toolbarLayout->addWidget(clearBtn_);
    
    saveBtn_ = new QPushButton(this);
    toolbarLayout->addWidget(saveBtn_);
    
    toolbarLayout->addStretch();
    layout->addLayout(toolbarLayout);

    // Log List
    logList_ = new QListWidget(this);
    logList_->setAlternatingRowColors(true);
    logList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    logList_->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(logList_);

    mainLayout->addWidget(groupBox_);

    // Connections
    connect(clearBtn_, &QPushButton::clicked, this, &TrafficMonitorWidget::clear);
    connect(saveBtn_, &QPushButton::clicked, this, &TrafficMonitorWidget::onSaveClicked);
    connect(logList_, &QListWidget::customContextMenuRequested, [this](const QPoint& pos) {
        QMenu menu(this);
        menu.addAction(tr("Copy"), this, &TrafficMonitorWidget::onCopyClicked);
        menu.exec(logList_->mapToGlobal(pos));
    });
    
    retranslateUi();
}

void TrafficMonitorWidget::appendTx(const QByteArray& data) {
    if (!showTxCheck_->isChecked()) return;

    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString content = formatData(data);
    QString itemText = tr("[%1] [TX] %2").arg(timeStr, content);
    
    auto item = new QListWidgetItem(itemText);
    item->setForeground(Qt::blue); // TX in Blue
    logList_->addItem(item);

    if (autoScrollCheck_->isChecked()) {
        logList_->scrollToBottom();
    }
}

void TrafficMonitorWidget::appendRx(const QByteArray& data) {
    if (!showRxCheck_->isChecked()) return;

    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString content = formatData(data);
    QString itemText = tr("[%1] [RX] %2").arg(timeStr, content);
    
    auto item = new QListWidgetItem(itemText);
    item->setForeground(Qt::darkGreen); // RX in Green
    logList_->addItem(item);

    if (autoScrollCheck_->isChecked()) {
        logList_->scrollToBottom();
    }
}

void TrafficMonitorWidget::appendInfo(const QString& message) {
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString itemText = tr("[%1] [INFO] %2").arg(timeStr, message);
    
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
    return QString(data.toHex(' ').toUpper());
}

void TrafficMonitorWidget::onSaveClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    for (int i = 0; i < logList_->count(); ++i) {
        out << logList_->item(i)->text() << "\n";
    }
}

void TrafficMonitorWidget::onCopyClicked() {
    QStringList selectedText;
    for (auto item : logList_->selectedItems()) {
        selectedText << item->text();
    }
    QApplication::clipboard()->setText(selectedText.join("\n"));
}

void TrafficMonitorWidget::retranslateUi() {
    if (groupBox_) groupBox_->setTitle(tr("Traffic Monitor"));
    if (autoScrollCheck_) autoScrollCheck_->setText(tr("Auto Scroll"));
    if (showTxCheck_) showTxCheck_->setText(tr("TX"));
    if (showRxCheck_) showRxCheck_->setText(tr("RX"));
    if (clearBtn_) clearBtn_->setText(tr("Clear"));
    if (saveBtn_) saveBtn_->setText(tr("Save"));
}

void TrafficMonitorWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
