// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

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

// Role: Encapsulates decoded register rendering, batch type assignment, and CSV export.
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent hierarchy.
class FrameDecodedTableView : public QTableWidget {
    Q_OBJECT

public:
    explicit FrameDecodedTableView(QWidget* parent = nullptr);
    ~FrameDecodedTableView() override = default;
    void renderData(const modbus::parser::ParseResult& result,
                    modbus::analyzer::RegisterDataType globalType,
                    modbus::base::RegisterOrder order,
                    modbus::address::AddressBase addressBase = modbus::address::AddressBase::Offset0Based);
    void clearTable();
    void exportCsv();
    void resetSelectedToDefault();
    void resetAllToDefault();
    void batchSetSelectedType(modbus::analyzer::RegisterDataType targetType);
    void retranslateUi();
    void setMetadataMap(const QMap<uint16_t, modbus::analyzer::DataMetadata>& meta);
    [[nodiscard]] const QMap<uint16_t, modbus::analyzer::DataMetadata>& metadataMap() const noexcept {
        return metadataByAddress_;
    }
    [[nodiscard]] bool hasCustomTypes() const;
    [[nodiscard]] int selectedRowCount() const;

signals:
    void selectionOrCustomTypeStateChanged(int selectedCount, bool hasCustom);
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
