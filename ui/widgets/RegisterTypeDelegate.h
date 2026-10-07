// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QStyledItemDelegate>
#include "analyzer/AnalyzerCommon.h"

namespace ui::widgets {

// Role: Delegate providing a QComboBox editor for register data types in table views.
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent hierarchy (typically QTableWidget).
class RegisterTypeDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit RegisterTypeDelegate(QObject* parent = nullptr);
    ~RegisterTypeDelegate() override = default;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

} // namespace ui::widgets
