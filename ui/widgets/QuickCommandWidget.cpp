#include "QuickCommandWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include <QEvent>
#include <QCoreApplication>

namespace ui::widgets {

// Helper Dialog for Adding/Editing Commands
class CommandEditDialog : public QDialog {
public:
    CommandEditDialog(QWidget* parent = nullptr, const QuickCommand& cmd = {}) : QDialog(parent) {
        setWindowTitle(QCoreApplication::translate("CommandEditDialog", "Command Editor"));
        auto layout = new QVBoxLayout(this);

        auto nameLayout = new QHBoxLayout();
        nameLayout->addWidget(new QLabel(QCoreApplication::translate("CommandEditDialog", "Name:")));
        nameEdit_ = new QLineEdit(cmd.name);
        nameLayout->addWidget(nameEdit_);
        layout->addLayout(nameLayout);

        auto dataLayout = new QHBoxLayout();
        dataLayout->addWidget(new QLabel(QCoreApplication::translate("CommandEditDialog", "Data:")));
        dataEdit_ = new QLineEdit(cmd.data);
        dataLayout->addWidget(dataEdit_);
        layout->addLayout(dataLayout);

        hexCheck_ = new QCheckBox(QCoreApplication::translate("CommandEditDialog", "Hex Mode"));
        hexCheck_->setChecked(cmd.isHex);
        layout->addWidget(hexCheck_);

        auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
    }

    QuickCommand getCommand() const {
        return {nameEdit_->text(), dataEdit_->text(), hexCheck_->isChecked()};
    }

private:
    QLineEdit* nameEdit_;
    QLineEdit* dataEdit_;
    QCheckBox* hexCheck_;
};

QuickCommandWidget::QuickCommandWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

QuickCommandWidget::~QuickCommandWidget() = default;

void QuickCommandWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Toolbar
    auto toolbar = new QHBoxLayout();
    addBtn_ = new QPushButton(this);
    editBtn_ = new QPushButton(this);
    removeBtn_ = new QPushButton(this);
    sendBtn_ = new QPushButton(this);
    exportBtn_ = new QPushButton(this);
    importBtn_ = new QPushButton(this);

    toolbar->addWidget(addBtn_);
    toolbar->addWidget(editBtn_);
    toolbar->addWidget(removeBtn_);
    toolbar->addWidget(sendBtn_);
    toolbar->addStretch();
    toolbar->addWidget(importBtn_);
    toolbar->addWidget(exportBtn_);
    mainLayout->addLayout(toolbar);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"", "", ""});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable inline editing for now
    mainLayout->addWidget(table_);

    // Connections
    connect(addBtn_, &QPushButton::clicked, this, &QuickCommandWidget::onAddClicked);
    connect(editBtn_, &QPushButton::clicked, this, &QuickCommandWidget::onEditClicked);
    connect(removeBtn_, &QPushButton::clicked, this, &QuickCommandWidget::onRemoveClicked);
    connect(sendBtn_, &QPushButton::clicked, this, &QuickCommandWidget::onSendClicked);
    connect(table_, &QTableWidget::cellDoubleClicked, this, &QuickCommandWidget::onTableDoubleClicked);
    connect(exportBtn_, &QPushButton::clicked, this, &QuickCommandWidget::onExportClicked);
    connect(importBtn_, &QPushButton::clicked, this, &QuickCommandWidget::onImportClicked);

    retranslateUi();
}

void QuickCommandWidget::updateRow(int row, const QuickCommand& cmd) {
    table_->setItem(row, 0, new QTableWidgetItem(cmd.name));
    table_->setItem(row, 1, new QTableWidgetItem(cmd.data));
    table_->setItem(row, 2, new QTableWidgetItem(cmd.isHex ? tr("HEX") : tr("ASCII")));
    
    // Store original data in UserRole if needed, but we can reconstruct from UI for now
    table_->item(row, 0)->setData(Qt::UserRole, cmd.isHex); 
}

QList<QuickCommand> QuickCommandWidget::getCommands() const {
    QList<QuickCommand> commands;
    for (int i = 0; i < table_->rowCount(); ++i) {
        QuickCommand cmd;
        cmd.name = table_->item(i, 0)->text();
        cmd.data = table_->item(i, 1)->text();
        // Format is in col 2, but we stored isHex in col 0 UserRole
        cmd.isHex = table_->item(i, 0)->data(Qt::UserRole).toBool();
        commands.append(cmd);
    }
    return commands;
}

void QuickCommandWidget::setCommands(const QList<QuickCommand>& commands) {
    table_->setRowCount(0);
    for (const auto& cmd : commands) {
        int row = table_->rowCount();
        table_->insertRow(row);
        updateRow(row, cmd);
    }
}

void QuickCommandWidget::onAddClicked() {
    CommandEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        auto cmd = dlg.getCommand();
        int row = table_->rowCount();
        table_->insertRow(row);
        updateRow(row, cmd);
    }
}

void QuickCommandWidget::onEditClicked() {
    int row = table_->currentRow();
    if (row < 0) return;

    QuickCommand currentCmd;
    currentCmd.name = table_->item(row, 0)->text();
    currentCmd.data = table_->item(row, 1)->text();
    currentCmd.isHex = table_->item(row, 0)->data(Qt::UserRole).toBool();

    CommandEditDialog dlg(this, currentCmd);
    if (dlg.exec() == QDialog::Accepted) {
        updateRow(row, dlg.getCommand());
    }
}

void QuickCommandWidget::onRemoveClicked() {
    int row = table_->currentRow();
    if (row >= 0) {
        table_->removeRow(row);
    }
}

void QuickCommandWidget::onSendClicked() {
    int row = table_->currentRow();
    if (row < 0) return;

    QString dataStr = table_->item(row, 1)->text();
    bool isHex = table_->item(row, 0)->data(Qt::UserRole).toBool();

    QByteArray data = parseData(dataStr, isHex);
    if (!data.isEmpty()) {
        emit sendRequested(data);
    }
}

void QuickCommandWidget::onTableDoubleClicked(int row, int /*column*/) {
    if (row < 0) return;
    
    // Select the row first if not selected
    table_->selectRow(row);
    onSendClicked();
}

QByteArray QuickCommandWidget::parseData(const QString& dataStr, bool isHex) {
    if (!isHex) {
        return dataStr.toUtf8();
    }
    
    // Hex parsing: remove spaces
    QString hex = dataStr;
    hex.remove(' ');
    return QByteArray::fromHex(hex.toUtf8());
}

void QuickCommandWidget::onExportClicked() {
    QString path = QFileDialog::getSaveFileName(this, tr("Export Commands"), "", tr("JSON Files (*.json)"));
    if (path.isEmpty()) return;

    QJsonArray arr;
    for (const auto& cmd : getCommands()) {
        QJsonObject obj;
        obj["name"] = cmd.name;
        obj["data"] = cmd.data;
        obj["isHex"] = cmd.isHex;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void QuickCommandWidget::onImportClicked() {
    QString path = QFileDialog::getOpenFileName(this, tr("Import Commands"), "", tr("JSON Files (*.json)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            QList<QuickCommand> cmds;
            QJsonArray arr = doc.array();
            for (const auto& val : arr) {
                QJsonObject obj = val.toObject();
                QuickCommand cmd;
                cmd.name = obj["name"].toString();
                cmd.data = obj["data"].toString();
                cmd.isHex = obj["isHex"].toBool();
                cmds.append(cmd);
            }
            setCommands(cmds);
        }
    }
}

void QuickCommandWidget::retranslateUi() {
    if (addBtn_) addBtn_->setText(tr("Add"));
    if (editBtn_) editBtn_->setText(tr("Edit"));
    if (removeBtn_) removeBtn_->setText(tr("Remove"));
    if (sendBtn_) sendBtn_->setText(tr("Send"));
    if (exportBtn_) exportBtn_->setText(tr("Export"));
    if (importBtn_) importBtn_->setText(tr("Import"));

    if (table_) {
        table_->setHorizontalHeaderLabels({tr("Name"), tr("Data"), tr("Format")});
        for (int row = 0; row < table_->rowCount(); ++row) {
            auto item = table_->item(row, 0);
            bool isHex = item ? item->data(Qt::UserRole).toBool() : false;
            if (table_->item(row, 2)) {
                table_->item(row, 2)->setText(isHex ? tr("HEX") : tr("ASCII"));
            }
        }
    }
}

void QuickCommandWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
