/**
 * @file FrameDecodedTableView.h
 * @brief Specialized table view component for decoded Modbus frame data items.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QTableWidget>
#include <QMap>
#include <QList>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/base/ModbusAddressMapping.h"

namespace ui::widgets {

class RegisterTypeDelegate;

/**
 * @brief Specialized QTableWidget encapsulating decoded register rendering,
 *        stride-aware batch type assignment, row multi-selection, keyboard
 *        shortcuts, and CSV export.
 */
class FrameDecodedTableView : public QTableWidget {
    Q_OBJECT

public:
    explicit FrameDecodedTableView(QWidget* parent = nullptr);
    ~FrameDecodedTableView() override = default;

    /**
     * @brief Renders the decoded data items from the parse result.
     */
    void renderData(const modbus::parser::ParseResult& result,
                    modbus::analyzer::RegisterDataType globalType,
                    modbus::base::RegisterOrder order,
                    modbus::address::AddressBase addressBase = modbus::address::AddressBase::Offset0Based);

    /**
     * @brief Clears all table content and cached result.
     */
    void clearTable();

    /**
     * @brief Exports current table content to CSV via file dialog.
     */
    void exportCsv();

    /**
     * @brief Resets custom types of selected rows to Default.
     */
    void resetSelectedToDefault();

    /**
     * @brief Resets all custom types in the table to Default.
     */
    void resetAllToDefault();

    /**
     * @brief Batch sets custom type for selected rows with stride awareness.
     */
    void batchSetSelectedType(modbus::analyzer::RegisterDataType targetType);

    /**
     * @brief Re-translates header labels.
     */
    void retranslateUi();

    /**
     * @brief Sets the address-to-metadata mapping.
     */
    void setMetadataMap(const QMap<uint16_t, modbus::analyzer::DataMetadata>& meta);

    /**
     * @brief Retrieves the address-to-metadata mapping.
     */
    [[nodiscard]] const QMap<uint16_t, modbus::analyzer::DataMetadata>& metadataMap() const noexcept {
        return metadataByAddress_;
    }

    /**
     * @brief Queries if any custom type is currently set.
     */
    [[nodiscard]] bool hasCustomTypes() const;

    /**
     * @brief Queries count of currently selected rows.
     */
    [[nodiscard]] int selectedRowCount() const;

signals:
    /**
     * @brief Emitted when selection count or custom type presence changes.
     * @param selectedCount Number of selected rows.
     * @param hasCustom True if at least one register has a customType override.
     */
    void selectionOrCustomTypeStateChanged(int selectedCount, bool hasCustom);

    /**
     * @brief Emitted when metadataByAddress is modified (type, scale, description).
     */
    void metadataChanged();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onSelectionChanged();
    void onTableContextMenuRequested(const QPoint& pos);
    void onItemChanged(QTableWidgetItem* item);

private:
    void setupTable();
    void notifyStateChanged();
    [[nodiscard]] uint16_t rowAddress(int row) const;
    [[nodiscard]] QList<int> getSelectedPrimaryRowsSorted() const;

    QMap<uint16_t, modbus::analyzer::DataMetadata> metadataByAddress_;
    modbus::parser::ParseResult currentResult_;
    modbus::analyzer::RegisterDataType globalDataType_ = modbus::analyzer::RegisterDataType::UInt16;
    modbus::base::RegisterOrder registerOrder_ = modbus::base::RegisterOrder::ABCD;
    modbus::address::AddressBase addressBase_ = modbus::address::AddressBase::Offset0Based;
    bool isUpdatingTable_ = false;
};

} // namespace ui::widgets
