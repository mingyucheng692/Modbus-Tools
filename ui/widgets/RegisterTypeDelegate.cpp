// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "RegisterTypeDelegate.h"
#include <QComboBox>

namespace ui::widgets {

RegisterTypeDelegate::RegisterTypeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* RegisterTypeDelegate::createEditor(
    QWidget* parent,
    const QStyleOptionViewItem& /*option*/,
    const QModelIndex& /*index*/) const
{
    auto* combo = new QComboBox(parent);
    combo->setAutoFillBackground(true);
    QPalette pal = combo->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::Base));
    combo->setPalette(pal);
    combo->setStyleSheet(QStringLiteral("QComboBox { background-color: palette(base); }"));

    combo->addItem(tr("Default"), -1);
    combo->addItem(QStringLiteral("UInt16"), static_cast<int>(modbus::analyzer::RegisterDataType::UInt16));
    combo->addItem(QStringLiteral("Int16"), static_cast<int>(modbus::analyzer::RegisterDataType::Int16));
    combo->addItem(QStringLiteral("Float32"), static_cast<int>(modbus::analyzer::RegisterDataType::Float32));
    combo->addItem(QStringLiteral("Int32"), static_cast<int>(modbus::analyzer::RegisterDataType::Int32));
    combo->addItem(QStringLiteral("UInt32"), static_cast<int>(modbus::analyzer::RegisterDataType::UInt32));
    combo->addItem(QStringLiteral("Float64"), static_cast<int>(modbus::analyzer::RegisterDataType::Float64));
    return combo;
}

void RegisterTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    const QVariant data = index.data(Qt::UserRole);
    int typeVal = -1;
    if (data.isValid()) {
        typeVal = data.toInt();
    }
    const int idx = combo->findData(typeVal);
    if (idx >= 0) {
        combo->setCurrentIndex(idx);
    } else {
        combo->setCurrentIndex(0);
    }
}

void RegisterTypeDelegate::setModelData(
    QWidget* editor,
    QAbstractItemModel* model,
    const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo || !model) return;

    const int selectedValue = combo->currentData().toInt();
    const QVariant currentUserData = index.data(Qt::UserRole);
    if (currentUserData.isValid() && currentUserData.toInt() == selectedValue) {
        return;
    }

    const QString storedDefault = index.data(Qt::UserRole + 1).toString();
    const QString displayText = (selectedValue == -1)
        ? (storedDefault.isEmpty() ? tr("Default") : storedDefault)
        : combo->currentText();

    // Update DisplayRole before UserRole so table observers keyed on UserRole
    // ignore the first notification and only re-render once per commit.
    model->setData(index, displayText, Qt::DisplayRole);
    model->setData(index, selectedValue, Qt::UserRole);
}

void RegisterTypeDelegate::updateEditorGeometry(
    QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& /*index*/) const
{
    if (editor) {
        editor->setGeometry(option.rect);
    }
}

} // namespace ui::widgets
