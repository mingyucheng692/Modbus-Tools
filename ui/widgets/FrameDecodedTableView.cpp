/**
 * @file FrameDecodedTableView.cpp
 * @brief Implementation of FrameDecodedTableView.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameDecodedTableView.h"
#include "widgets/RegisterTypeDelegate.h"
#include "analyzer/AnalyzerCommon.h"
#include "analyzer/AnalyzerExporter.h"
#include "analyzer/ValueFormatter.h"
#include "modbus/base/RegisterValueDecoder.h"

#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QItemSelectionModel>
#include <QSignalBlocker>

using namespace modbus::parser;
using namespace modbus::analyzer;

namespace ui::widgets {

FrameDecodedTableView::FrameDecodedTableView(QWidget* parent)
    : QTableWidget(parent)
{
    setupTable();
}

void FrameDecodedTableView::setupTable()
{
    setColumnCount(8);
    retranslateUi();

    setItemDelegateForColumn(4, new RegisterTypeDelegate(this));
    horizontalHeader()->setStretchLastSection(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTableWidget::customContextMenuRequested,
            this, &FrameDecodedTableView::onTableContextMenuRequested);
    connect(this, &QTableWidget::itemChanged,
            this, &FrameDecodedTableView::onItemChanged);

    if (selectionModel()) {
        connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &FrameDecodedTableView::onSelectionChanged);
    }
}

void FrameDecodedTableView::retranslateUi()
{
    setHorizontalHeaderLabels({
        tr("Address"),
        tr("Hex"),
        tr("Decimal"),
        tr("Binary"),
        tr("Type"),
        tr("Scale"),
        tr("Value"),
        tr("Description")
    });
}

void FrameDecodedTableView::setMetadataMap(const QMap<uint16_t, DataMetadata>& meta)
{
    metadataByAddress_ = meta;
    notifyStateChanged();
}

bool FrameDecodedTableView::hasCustomTypes() const
{
    for (auto it = metadataByAddress_.cbegin(); it != metadataByAddress_.cend(); ++it) {
        if (it.value().customType.has_value()) {
            return true;
        }
    }
    return false;
}

int FrameDecodedTableView::selectedRowCount() const
{
    return selectionModel() ? selectionModel()->selectedRows().size() : 0;
}

void FrameDecodedTableView::notifyStateChanged()
{
    emit selectionOrCustomTypeStateChanged(selectedRowCount(), hasCustomTypes());
}

uint16_t FrameDecodedTableView::rowAddress(int row) const
{
    if (row < 0 || row >= rowCount()) return 0;
    const QTableWidgetItem* addrItem = item(row, 0);
    if (!addrItem) return 0;
    const QVariant data = addrItem->data(Qt::UserRole);
    return data.isValid() ? static_cast<uint16_t>(data.toUInt()) : 0;
}

QList<int> FrameDecodedTableView::getSelectedPrimaryRowsSorted() const
{
    if (!selectionModel()) return {};
    const auto selectedIndexes = selectionModel()->selectedRows();
    QList<int> rows;
    rows.reserve(selectedIndexes.size());
    for (const auto& idx : selectedIndexes) {
        rows.append(idx.row());
    }
    std::sort(rows.begin(), rows.end());
    return rows;
}

void FrameDecodedTableView::renderData(const ParseResult& result,
                                       RegisterDataType globalType,
                                       modbus::base::RegisterOrder order,
                                       modbus::address::AddressBase addressBase)
{
    currentResult_ = result;
    globalDataType_ = globalType;
    registerOrder_ = order;
    addressBase_ = addressBase;

    isUpdatingTable_ = true;
    setRowCount(0);

    if (!result.isValid || result.dataItems.isEmpty()) {
        isUpdatingTable_ = false;
        notifyStateChanged();
        return;
    }

    setRowCount(result.dataItems.size());

    int subordinateRemaining = 0;
    RegisterDataType subordinateParentType = globalDataType_;
    int subordinateWordIndex = 0;
    uint16_t parentAddress = 0;

    for (int i = 0; i < result.dataItems.size(); ++i) {
        const auto& itemData = result.dataItems[i];
        const DataMetadata meta = metadataByAddress_.value(itemData.address);

        // 0: Address
        const QString dispAddr = modbus::address::toDisplayAddress(itemData.address, addressBase_, false);
        auto* addrItem = new QTableWidgetItem(QStringLiteral("%1 (0x%2)")
            .arg(dispAddr)
            .arg(QString::number(itemData.address, 16).toUpper().rightJustified(4, QLatin1Char('0'))));
        addrItem->setData(Qt::UserRole, itemData.address);
        addrItem->setFlags(addrItem->flags() & ~Qt::ItemIsEditable);
        setItem(i, 0, addrItem);

        // 1: Hex
        auto* hexItem = new QTableWidgetItem(value_formatter::formatHexValue(itemData.rawBytes, itemData.hexString));
        hexItem->setFlags(addrItem->flags());
        setItem(i, 1, hexItem);

        // 2: Decimal
        const NumberDisplayMode decMode = (globalDataType_ == RegisterDataType::Int16)
            ? NumberDisplayMode::Signed : NumberDisplayMode::Unsigned;
        auto* decItem = new QTableWidgetItem(value_formatter::formatDecimalValue(itemData.value, decMode));
        decItem->setFlags(addrItem->flags());
        setItem(i, 2, decItem);

        // 3: Binary
        auto* binItem = new QTableWidgetItem(value_formatter::formatBinaryValue(itemData.rawBytes, itemData.binaryString));
        binItem->setFlags(addrItem->flags());
        setItem(i, 3, binItem);

        if (subordinateRemaining > 0) {
            subordinateWordIndex++;
            subordinateRemaining--;

            // 4: Type
            const QString typeDesc = (subordinateWordIndex == 2 && registerWordsCount(subordinateParentType) == 2)
                ? QStringLiteral("[%1 Low-Word]").arg(registerDataTypeToString(subordinateParentType))
                : QStringLiteral("[%1 W%2]").arg(registerDataTypeToString(subordinateParentType)).arg(subordinateWordIndex);
            auto* typeItem = new QTableWidgetItem(typeDesc);
            typeItem->setFlags(addrItem->flags());
            typeItem->setForeground(QColor(128, 128, 128));
            setItem(i, 4, typeItem);

            // 5: Scale
            auto* scaleItem = new QTableWidgetItem(QStringLiteral("-"));
            scaleItem->setFlags(addrItem->flags());
            scaleItem->setForeground(QColor(128, 128, 128));
            setItem(i, 5, scaleItem);

            // 6: Value
            auto* valItem = new QTableWidgetItem(QStringLiteral("-"));
            valItem->setFlags(addrItem->flags());
            valItem->setForeground(QColor(128, 128, 128));
            setItem(i, 6, valItem);

            // 7: Description
            const QString descText = meta.description.isEmpty()
                ? tr("(Subordinate word of address %1)").arg(parentAddress)
                : meta.description;
            auto* descItem = new QTableWidgetItem(descText);
            descItem->setFlags(descItem->flags() | Qt::ItemIsEditable);
            descItem->setForeground(QColor(128, 128, 128));
            setItem(i, 7, descItem);
        } else {
            const bool isBoolType = (itemData.value.typeId() == QMetaType::Bool);
            const RegisterDataType effectiveType = meta.customType.value_or(globalDataType_);
            const int wordsNeeded = isBoolType ? 1 : registerWordsCount(effectiveType);

            QByteArray combinedBytes;
            const int availableWords = qMin(wordsNeeded, result.dataItems.size() - i);
            for (int w = 0; w < availableWords; ++w) {
                const auto& wItem = result.dataItems[i + w];
                if (wItem.rawBytes.size() >= 2) {
                    combinedBytes.append(wItem.rawBytes.left(2));
                } else if (wItem.value.isValid()) {
                    const uint16_t v = static_cast<uint16_t>(wItem.value.toUInt());
                    combinedBytes.append(static_cast<char>((v >> 8) & 0xFF));
                    combinedBytes.append(static_cast<char>(v & 0xFF));
                }
            }

            QString valText;
            QString tooltip;
            if (isBoolType) {
                valText = itemData.value.toBool() ? QStringLiteral("1") : QStringLiteral("0");
                tooltip = meta.description;
            } else if (availableWords < wordsNeeded) {
                valText = QStringLiteral("<Incomplete>");
                tooltip = tr("Incomplete register bytes for %1").arg(registerDataTypeToString(effectiveType));
            } else {
                subordinateRemaining = wordsNeeded - 1;
                subordinateParentType = effectiveType;
                subordinateWordIndex = 1;
                parentAddress = itemData.address;

                valText = value_formatter::formatScaledValue(combinedBytes, meta, effectiveType, registerOrder_);
                tooltip = value_formatter::buildDescriptionTooltip(combinedBytes, meta, effectiveType, registerOrder_);
            }

            // 4: Type
            auto* typeItem = new QTableWidgetItem();
            if (meta.customType.has_value()) {
                typeItem->setText(registerDataTypeToString(*meta.customType));
                typeItem->setData(Qt::UserRole, static_cast<int>(*meta.customType));
            } else {
                typeItem->setText(tr("Default (%1)").arg(registerDataTypeToString(globalDataType_)));
                typeItem->setData(Qt::UserRole, -1);
            }
            typeItem->setFlags(typeItem->flags() | Qt::ItemIsEditable);
            setItem(i, 4, typeItem);

            // 5: Scale
            auto* scaleItem = new QTableWidgetItem(QString::number(meta.scale, 'g', 12));
            scaleItem->setFlags(scaleItem->flags() | Qt::ItemIsEditable);
            setItem(i, 5, scaleItem);

            // 6: Value
            auto* valItem = new QTableWidgetItem(valText);
            valItem->setFlags(addrItem->flags());
            setItem(i, 6, valItem);

            // 7: Description
            auto* descItem = new QTableWidgetItem(meta.description);
            descItem->setFlags(descItem->flags() | Qt::ItemIsEditable);
            if (!tooltip.isEmpty()) {
                descItem->setToolTip(tooltip);
            }
            setItem(i, 7, descItem);
        }
    }

    isUpdatingTable_ = false;
    notifyStateChanged();
}

void FrameDecodedTableView::clearTable()
{
    currentResult_ = {};
    setRowCount(0);
    notifyStateChanged();
}

void FrameDecodedTableView::onItemChanged(QTableWidgetItem* itemChanged)
{
    if (isUpdatingTable_ || !itemChanged) return;
    const int col = itemChanged->column();
    if (col != 4 && col != 5 && col != 7) return;

    const uint16_t address = rowAddress(itemChanged->row());
    DataMetadata meta = metadataByAddress_.value(address);

    if (col == 4) {
        const QVariant typeVal = itemChanged->data(Qt::UserRole);
        if (typeVal.isValid() && typeVal.toInt() >= 0) {
            meta.customType = static_cast<RegisterDataType>(typeVal.toInt());
        } else {
            meta.customType = std::nullopt;
        }
        metadataByAddress_.insert(address, meta);
        if (currentResult_.isValid) {
            renderData(currentResult_, globalDataType_, registerOrder_, addressBase_);
        }
        emit metadataChanged();
        notifyStateChanged();
        return;
    } else if (col == 5) {
        bool ok = false;
        const double parsedScale = itemChanged->text().toDouble(&ok);
        if (!ok) {
            QSignalBlocker blocker(this);
            itemChanged->setText(QString::number(meta.scale, 'g', 12));
            return;
        }
        meta.scale = parsedScale;
        metadataByAddress_.insert(address, meta);
        if (currentResult_.isValid) {
            renderData(currentResult_, globalDataType_, registerOrder_, addressBase_);
        }
        emit metadataChanged();
        return;
    } else if (col == 7) {
        meta.description = itemChanged->text();
        metadataByAddress_.insert(address, meta);
        emit metadataChanged();
    }
}

void FrameDecodedTableView::resetSelectedToDefault()
{
    const auto selectedRows = getSelectedPrimaryRowsSorted();
    if (selectedRows.isEmpty()) return;

    for (int r : selectedRows) {
        const uint16_t addr = rowAddress(r);
        if (metadataByAddress_.contains(addr)) {
            auto meta = metadataByAddress_.value(addr);
            meta.customType = std::nullopt;
            metadataByAddress_.insert(addr, meta);
        }

        const auto* typeItem = item(r, 4);
        if (typeItem && !(typeItem->flags() & Qt::ItemIsEditable)) {
            for (int p = r - 1; p >= 0; --p) {
                const auto* pItem = item(p, 4);
                if (pItem && (pItem->flags() & Qt::ItemIsEditable)) {
                    const uint16_t pAddr = rowAddress(p);
                    if (metadataByAddress_.contains(pAddr)) {
                        auto pMeta = metadataByAddress_.value(pAddr);
                        pMeta.customType = std::nullopt;
                        metadataByAddress_.insert(pAddr, pMeta);
                    }
                    break;
                }
            }
        }
    }

    if (currentResult_.isValid) {
        renderData(currentResult_, globalDataType_, registerOrder_, addressBase_);
        if (selectionModel()) {
            QItemSelection selection;
            for (int r : selectedRows) {
                if (r < rowCount()) {
                    selection.select(model()->index(r, 0), model()->index(r, columnCount() - 1));
                }
            }
            selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
        }
    }
    emit metadataChanged();
    notifyStateChanged();
}

void FrameDecodedTableView::resetAllToDefault()
{
    for (auto it = metadataByAddress_.begin(); it != metadataByAddress_.end(); ++it) {
        it.value().customType = std::nullopt;
    }
    if (currentResult_.isValid) {
        renderData(currentResult_, globalDataType_, registerOrder_, addressBase_);
    }
    emit metadataChanged();
    notifyStateChanged();
}

void FrameDecodedTableView::batchSetSelectedType(RegisterDataType targetType)
{
    const auto selectedRows = getSelectedPrimaryRowsSorted();
    if (selectedRows.isEmpty()) return;

    const int wordsNeeded = registerWordsCount(targetType);

    for (int i = 0; i < selectedRows.size(); ) {
        int curRow = selectedRows[i];
        uint16_t primaryAddr = rowAddress(curRow);

        DataMetadata meta = metadataByAddress_.value(primaryAddr);
        meta.customType = targetType;
        metadataByAddress_.insert(primaryAddr, meta);

        for (int w = 1; w < wordsNeeded; ++w) {
            if (i + w < selectedRows.size() && selectedRows[i + w] == curRow + w) {
                uint16_t subAddr = rowAddress(curRow + w);
                if (metadataByAddress_.contains(subAddr)) {
                    auto subMeta = metadataByAddress_.value(subAddr);
                    subMeta.customType = std::nullopt;
                    metadataByAddress_.insert(subAddr, subMeta);
                }
            }
        }

        if (wordsNeeded > 1 && i + 1 < selectedRows.size() && selectedRows[i + 1] == curRow + 1) {
            int advance = 0;
            while (advance < wordsNeeded && (i + advance) < selectedRows.size()
                   && selectedRows[i + advance] == curRow + advance) {
                advance++;
            }
            i += advance;
        } else {
            i++;
        }
    }

    if (currentResult_.isValid) {
        renderData(currentResult_, globalDataType_, registerOrder_, addressBase_);
        if (selectionModel()) {
            QItemSelection selection;
            for (int r : selectedRows) {
                if (r < rowCount()) {
                    selection.select(model()->index(r, 0), model()->index(r, columnCount() - 1));
                }
            }
            selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
        }
    }
    emit metadataChanged();
    notifyStateChanged();
}

void FrameDecodedTableView::onSelectionChanged()
{
    notifyStateChanged();
}

void FrameDecodedTableView::onTableContextMenuRequested(const QPoint& pos)
{
    if (rowCount() == 0) return;

    const auto selectedRows = getSelectedPrimaryRowsSorted();
    const int count = selectedRows.size();
    const bool hasCustom = hasCustomTypes();

    QMenu menu(this);

    auto* resetSelectedAction = menu.addAction(
        count > 0 ? tr("Reset Selected Type to Default (%1)\tDelete").arg(count)
                  : tr("Reset Selected Type to Default\tDelete"));
    resetSelectedAction->setEnabled(count > 0);
    connect(resetSelectedAction, &QAction::triggered, this, &FrameDecodedTableView::resetSelectedToDefault);

    menu.addSeparator();

    auto* batchMenu = menu.addMenu(tr("Batch Set Selected Type to..."));
    batchMenu->setEnabled(count > 0);

    auto addBatchTypeAction = [this, batchMenu](const QString& text, RegisterDataType type) {
        auto* action = batchMenu->addAction(text);
        connect(action, &QAction::triggered, this, [this, type]() {
            batchSetSelectedType(type);
        });
    };

    addBatchTypeAction(tr("UInt16 (1 Reg)"), RegisterDataType::UInt16);
    addBatchTypeAction(tr("Int16 (1 Reg)"), RegisterDataType::Int16);
    addBatchTypeAction(tr("Float32 (Real, 2 Regs)"), RegisterDataType::Float32);
    addBatchTypeAction(tr("Int32 (DInt, 2 Regs)"), RegisterDataType::Int32);
    addBatchTypeAction(tr("UInt32 (UDInt, 2 Regs)"), RegisterDataType::UInt32);
    addBatchTypeAction(tr("Float64 (Double, 4 Regs)"), RegisterDataType::Float64);

    menu.addSeparator();

    auto* resetAllAction = menu.addAction(tr("Reset All Custom Types to Default"));
    resetAllAction->setEnabled(hasCustom);
    connect(resetAllAction, &QAction::triggered, this, &FrameDecodedTableView::resetAllToDefault);

    menu.exec(mapToGlobal(pos));
}

void FrameDecodedTableView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        QWidget* fw = QApplication::focusWidget();
        if (fw && fw != this && fw != viewport()) {
            QTableWidget::keyPressEvent(event);
            return;
        }
        if (selectionModel() && !selectionModel()->selectedRows().isEmpty()) {
            resetSelectedToDefault();
            event->accept();
            return;
        }
    }
    QTableWidget::keyPressEvent(event);
}

void FrameDecodedTableView::exportCsv()
{
    if (!currentResult_.isValid || rowCount() == 0) {
        QMessageBox::information(this, tr("No Data"), tr("There is no data to export."));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export CSV"),
        QStringLiteral("analysis_%1.csv").arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_HHmmss'Z'"))),
        tr("CSV Files (*.csv)"));
    if (filePath.isEmpty()) return;

    QStringList lines;
    QStringList headers;
    for (int c = 0; c < columnCount(); ++c) {
        headers << exporter::escapeCsvValue(horizontalHeaderItem(c)->text());
    }
    lines << headers.join(QLatin1Char(','));

    for (int r = 0; r < rowCount(); ++r) {
        QStringList row;
        for (int c = 0; c < columnCount(); ++c) {
            row << exporter::escapeCsvValue(item(r, c)->text());
        }
        lines << row.join(QLatin1Char(','));
    }

    QString error;
    if (!exporter::writeCsvChunk(filePath, lines, true, &error)) {
        QMessageBox::warning(this, tr("Export Failed"), error);
    }
}

} // namespace ui::widgets
