// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QList>
#include <QMap>
#include <QTimer>
#include <cstdint>
#include "TelemetryTableModel.h"
#include "TelemetryRowView.h"
#include "common/TrafficEvent.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/base/ModbusByteOrder.h"
#include "modbus/base/RegisterDataType.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "analyzer/AnalyzerCommon.h"
#include "analyzer/ValueFormatter.h"

class QTableView;
class QSortFilterProxyModel;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QTextEdit;
class QSplitter;
class QGridLayout;
class QResizeEvent;

namespace ui::widgets::modbus {

/// Role: High-performance telemetry panel featuring 50ms batching, table view, micro-inspector, and metrics.
/// Thread-safety: Thread-affine (GUI main thread event loop).
/// Ownership: Owned by views::modbus::ModbusPage via Qt hierarchical ownership.
class ModbusTelemetryWidget : public QWidget {
    Q_OBJECT

public:
    static constexpr int kMaxPendingEvents = 1000;
    static constexpr int kMaxBatchSize = 200;
    static constexpr int kBatchIntervalMs = 50;

    explicit ModbusTelemetryWidget(QWidget* parent = nullptr);
    ~ModbusTelemetryWidget() override = default;

    void appendTrafficEvent(const ::ui::common::TrafficEvent& event);
    void setProtocolType(::modbus::parser::ProtocolType protocol) noexcept;
    void updateMetrics(int tx, int rx, int err, int rttMs);
    void setLinkageState(bool active);
    [[nodiscard]] bool isLinkageActive() const noexcept;
    void clear();

    [[nodiscard]] TelemetryTableModel* tableModel() const noexcept { return tableModel_; }
    [[nodiscard]] QTableView* tableView() const noexcept { return tableView_; }
    [[nodiscard]] bool isAutoScrollEnabled() const noexcept { return autoScroll_; }
    [[nodiscard]] bool isBatchTimerActive() const noexcept { return batchTimer_ && batchTimer_->isActive(); }
    void setAutoScrollEnabled(bool enabled);
    [[nodiscard]] QString inspectorDetailsText() const noexcept;
    [[nodiscard]] bool exportToFile(const QString& filePath, QString* error = nullptr) const;
    [[nodiscard]] bool isTwoRowToolbar() const noexcept { return isTwoRowToolbar_; }
    void updateResponsiveToolbar(int availableWidth);
    [[nodiscard]] QPushButton* autoScrollButton() const noexcept { return autoScrollBtn_; }
    [[nodiscard]] QPushButton* clearButton() const noexcept { return clearBtn_; }
    [[nodiscard]] QPushButton* exportButton() const noexcept { return exportBtn_; }
    [[nodiscard]] QPushButton* liveAnalyzerButton() const noexcept { return liveAnalyzerBtn_; }
    [[nodiscard]] QPushButton* resetStatsButton() const noexcept { return resetStatsBtn_; }
    [[nodiscard]] QPushButton* inspectInAnalyzerButton() const noexcept { return inspectInAnalyzerBtn_; }
    [[nodiscard]] QComboBox* filterComboBox() const noexcept { return filterCombo_; }
    [[nodiscard]] QCheckBox* pollFramesCheckBox() const noexcept { return pollFramesCheck_; }
    [[nodiscard]] QLabel* txStatsLabel() const noexcept { return txStatsLabel_; }
    [[nodiscard]] QLabel* rxStatsLabel() const noexcept { return rxStatsLabel_; }
    [[nodiscard]] QLabel* errStatsLabel() const noexcept { return errStatsLabel_; }
    [[nodiscard]] QLabel* rttStatsLabel() const noexcept { return rttStatsLabel_; }

    void setMetadataMap(const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& meta);
    [[nodiscard]] const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& metadataMap() const noexcept { return metadataByAddress_; }
    [[nodiscard]] QComboBox* decodeTypeCombo() const noexcept { return decodeTypeCombo_; }
    [[nodiscard]] QComboBox* orderCombo() const noexcept { return orderCombo_; }
    [[nodiscard]] ::modbus::analyzer::RegisterDataType currentDecodeType() const noexcept { return currentDataType_; }
    [[nodiscard]] ::modbus::base::RegisterOrder currentRegisterOrder() const noexcept { return currentOrder_; }
    void setDecodeType(::modbus::analyzer::RegisterDataType type);
    void setRegisterOrder(::modbus::base::RegisterOrder order);
    void applyDecodeCapability(const ::modbus::analyzer::value_formatter::FrameDecodeEvaluation& eval);

    [[nodiscard]] static QString formatInspectorDetails(const ::modbus::parser::ParseResult& pr, int aduLength);
    [[nodiscard]] QString formatInspectorDetails(
        const ::modbus::parser::ParseResult& pr,
        int aduLength,
        bool isTx,
        ::modbus::analyzer::RegisterDataType type,
        ::modbus::base::RegisterOrder order,
        const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& metadata = {}) const;
    [[nodiscard]] static QString formatRowTextForCopy(const TelemetryRowView& row);

signals:
    void linkageToggled(bool active);
    void pollFramesToggled(bool enabled);
    void frameSelectedForAnalysis(const QByteArray& payload);
    void inspectInAnalyzerRequested(const QString& hexString);
    void resetMetricsRequested();
    void rowCopied(const QString& text);
    void exportFailed(const QString& error);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void processBatchTimeout();
    void onSelectionChanged();
    void onFilterChanged(int index);
    void onExportClicked();
    void onCopySelectedRow();
    void onInspectInAnalyzerClicked();
    void onDecodeConfigChanged();

private:
    void setupUi();
    void setupToolbar();
    void applyToolbarLayout(bool twoRow);
    void setupTableView();
    void setupInspector();
    void applyInspectorHeaderLayout(bool twoRow);
    void updateResponsiveInspectorHeader(int availableWidth);
    void setupMetricsRibbon();
    void updateInspectorContent(const TelemetryRowView* row);
    void updateMetricsLabel();
    void retranslateUi();
    void retranslateToolbarUi();
    void retranslateInspectorUi();
    [[nodiscard]] QStringList buildCsvLines() const;
    [[nodiscard]] bool exportJsonToFile(const QString& filePath, QString* error) const;

    TelemetryTableModel* tableModel_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    QTableView* tableView_ = nullptr;
    QSplitter* splitter_ = nullptr;

    // Toolbar controls
    QWidget* toolbarWidget_ = nullptr;
    QGridLayout* toolbarLayout_ = nullptr;
    bool isTwoRowToolbar_ = false;
    QLabel* filterLabel_ = nullptr;
    QComboBox* filterCombo_ = nullptr;
    QCheckBox* pollFramesCheck_ = nullptr;
    QPushButton* autoScrollBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* exportBtn_ = nullptr;
    QPushButton* liveAnalyzerBtn_ = nullptr;
    QLabel* overflowLabel_ = nullptr;

    // KPI Metrics Ribbon controls
    QWidget* metricsRibbon_ = nullptr;
    QLabel* txStatsLabel_ = nullptr;
    QLabel* rxStatsLabel_ = nullptr;
    QLabel* errStatsLabel_ = nullptr;
    QLabel* rttStatsLabel_ = nullptr;
    QPushButton* resetStatsBtn_ = nullptr;
    QLabel* metricsLabel_ = nullptr;

    // Inspector controls
    QWidget* inspectorHeaderWidget_ = nullptr;
    QGridLayout* inspectorHeaderLayout_ = nullptr;
    bool isTwoRowInspectorHeader_ = false;
    QLabel* inspectorHeaderLabel_ = nullptr;
    QLabel* decodeLabel_ = nullptr;
    QComboBox* decodeTypeCombo_ = nullptr;
    QLabel* orderLabel_ = nullptr;
    QComboBox* orderCombo_ = nullptr;
    QPushButton* inspectInAnalyzerBtn_ = nullptr;
    QTextEdit* inspectorDetailsText_ = nullptr;
    QMap<uint16_t, ::modbus::analyzer::DataMetadata> metadataByAddress_;
    ::modbus::analyzer::RegisterDataType currentDataType_ = ::modbus::analyzer::RegisterDataType::UInt16;
    ::modbus::base::RegisterOrder currentOrder_ = ::modbus::base::RegisterOrder::ABCD;

    // Backpressure & flow control state
    QTimer* batchTimer_ = nullptr;
    QList<::ui::common::TrafficEvent> pendingEvents_;
    ::modbus::parser::ProtocolType currentProtocol_ = ::modbus::parser::ProtocolType::Tcp;
    bool autoScroll_ = true;
    int droppedEventsCount_ = 0;

    // Metrics counters
    int txCount_ = 0;
    int rxCount_ = 0;
    int errCount_ = 0;
    int rttMs_ = 0;
};

} // namespace ui::widgets::modbus

