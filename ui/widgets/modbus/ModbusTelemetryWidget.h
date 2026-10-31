// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QList>
#include <QTimer>
#include <cstdint>
#include "TelemetryTableModel.h"
#include "TelemetryRowView.h"
#include "common/TrafficEvent.h"
#include "modbus/parser/ModbusFrameParser.h"

class QTableView;
class QSortFilterProxyModel;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QTextEdit;
class QSplitter;

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
    void setAutoScrollEnabled(bool enabled);
    [[nodiscard]] QString inspectorDetailsText() const noexcept;

    [[nodiscard]] static QString formatInspectorDetails(const ::modbus::parser::ParseResult& pr, int aduLength);

signals:
    void linkageToggled(bool active);
    void rawFramesToggled(bool enabled);
    void frameSelectedForAnalysis(const QByteArray& payload);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void processBatchTimeout();
    void onSelectionChanged();
    void onFilterChanged(int index);
    void onExportClicked();
    void onCopySelectedRow();

private:
    void setupUi();
    void setupToolbar();
    void setupTableView();
    void setupInspector();
    void setupFooter();
    void updateInspectorContent(const TelemetryRowView* row);
    void updateMetricsLabel();
    void retranslateUi();

    TelemetryTableModel* tableModel_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    QTableView* tableView_ = nullptr;
    QSplitter* splitter_ = nullptr;

    // Toolbar controls
    QLabel* filterLabel_ = nullptr;
    QComboBox* filterCombo_ = nullptr;
    QCheckBox* rawFramesCheck_ = nullptr;
    QPushButton* autoScrollBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* exportBtn_ = nullptr;
    QLabel* overflowLabel_ = nullptr;

    // Inspector controls
    QLabel* inspectorHeaderLabel_ = nullptr;
    QTextEdit* inspectorDetailsText_ = nullptr;

    // Footer controls
    QLabel* metricsLabel_ = nullptr;
    QCheckBox* linkageCheck_ = nullptr;

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

