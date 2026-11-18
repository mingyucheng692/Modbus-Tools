// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusTelemetryWidget.h"
#include "common/FileExporter.h"
#include "modbus/base/ModbusFunctionTraits.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSplitter>
#include <QFileDialog>
#include <QSaveFile>
#include <QMessageBox>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QGuiApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QFontDatabase>

namespace ui::widgets::modbus {

namespace {

QString directionLabel(::ui::common::TrafficDirection dir) {
    switch (dir) {
    case ::ui::common::TrafficDirection::Tx:
        return QStringLiteral("TX");
    case ::ui::common::TrafficDirection::Rx:
        return QStringLiteral("RX");
    case ::ui::common::TrafficDirection::None:
        return QStringLiteral("SYS");
    }
    return QStringLiteral("SYS");
}

QString csvEscapeField(const QString& value) {
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(escaped);
}

} // namespace

ModbusTelemetryWidget::ModbusTelemetryWidget(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void ModbusTelemetryWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    setupToolbar();
    setupMetricsRibbon();

    splitter_ = new QSplitter(Qt::Vertical, this);
    setupTableView();
    setupInspector();

    splitter_->setStretchFactor(0, 7);
    splitter_->setStretchFactor(1, 3);
    mainLayout->addWidget(splitter_, 1);

    batchTimer_ = new QTimer(this);
    batchTimer_->setInterval(kBatchIntervalMs);
    connect(batchTimer_, &QTimer::timeout, this, &ModbusTelemetryWidget::processBatchTimeout);
}

void ModbusTelemetryWidget::setupToolbar() {
    toolbarWidget_ = new QWidget(this);
    toolbarWidget_->setObjectName(QStringLiteral("telemetryToolbar"));
    toolbarLayout_ = new QGridLayout(toolbarWidget_);
    toolbarLayout_->setContentsMargins(0, 0, 0, 0);
    toolbarLayout_->setHorizontalSpacing(6);
    toolbarLayout_->setVerticalSpacing(4);

    filterCombo_ = new QComboBox(toolbarWidget_);
    filterCombo_->setObjectName(QStringLiteral("telemetryFilterCombo"));
    filterCombo_->addItems({tr("All"), tr("TX"), tr("RX"), tr("SYS"), tr("Err & Warn"), tr("Errors"), tr("Warnings")});
    filterCombo_->setToolTip(tr("Filter telemetry log by direction or event severity"));
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTelemetryWidget::onFilterChanged);
    filterLabel_ = new QLabel(tr("Filter:"), toolbarWidget_);
    filterLabel_->setObjectName(QStringLiteral("telemetryFilterLabel"));

    pollFramesCheck_ = new QCheckBox(tr("Poll Frames"), toolbarWidget_);
    pollFramesCheck_->setObjectName(QStringLiteral("telemetryPollFramesCheck"));
    pollFramesCheck_->setToolTip(tr("Sample 1 in 10 polling request/response frame pairs in traffic log; manual requests are always logged in full"));
    connect(pollFramesCheck_, &QCheckBox::toggled, this, &ModbusTelemetryWidget::pollFramesToggled);

    autoScrollBtn_ = new QPushButton(tr("Auto-Scroll"), toolbarWidget_);
    autoScrollBtn_->setObjectName(QStringLiteral("telemetryAutoScrollBtn"));
    autoScrollBtn_->setCheckable(true);
    autoScrollBtn_->setChecked(true);
    autoScrollBtn_->setToolTip(tr("Automatically scroll to latest telemetry event"));
    connect(autoScrollBtn_, &QPushButton::toggled, this, &ModbusTelemetryWidget::setAutoScrollEnabled);

    clearBtn_ = new QPushButton(tr("Clear"), toolbarWidget_);
    clearBtn_->setObjectName(QStringLiteral("telemetryClearBtn"));
    clearBtn_->setToolTip(tr("Clear telemetry events (Ctrl+L)"));
    connect(clearBtn_, &QPushButton::clicked, this, &ModbusTelemetryWidget::clear);

    exportBtn_ = new QPushButton(tr("Export..."), toolbarWidget_);
    exportBtn_->setObjectName(QStringLiteral("telemetryExportBtn"));
    exportBtn_->setToolTip(tr("Export telemetry table to CSV or JSON"));
    connect(exportBtn_, &QPushButton::clicked, this, &ModbusTelemetryWidget::onExportClicked);

    liveAnalyzerBtn_ = new QPushButton(tr("Live → Analyzer"), toolbarWidget_);
    liveAnalyzerBtn_->setObjectName(QStringLiteral("liveAnalyzerBtn"));
    liveAnalyzerBtn_->setCheckable(true);
    liveAnalyzerBtn_->setChecked(true);
    liveAnalyzerBtn_->setToolTip(tr("Stream received responses directly to Frame Analyzer in real time"));
    connect(liveAnalyzerBtn_, &QPushButton::toggled, this, &ModbusTelemetryWidget::linkageToggled);

    overflowLabel_ = new QLabel(toolbarWidget_);
    overflowLabel_->setObjectName(QStringLiteral("telemetryOverflowLabel"));
    overflowLabel_->setStyleSheet(QStringLiteral("color: #DC3545; font-weight: bold;"));
    overflowLabel_->hide();

    applyToolbarLayout(isTwoRowToolbar_);
    qobject_cast<QVBoxLayout*>(layout())->addWidget(toolbarWidget_);
}

void ModbusTelemetryWidget::applyToolbarLayout(bool twoRow) {
    isTwoRowToolbar_ = twoRow;
    while (QLayoutItem* item = toolbarLayout_->takeAt(0)) {
        delete item;
    }

    if (twoRow) {
        toolbarLayout_->addWidget(filterLabel_, 0, 0);
        toolbarLayout_->addWidget(filterCombo_, 0, 1);
        toolbarLayout_->addWidget(autoScrollBtn_, 0, 2);
        toolbarLayout_->addWidget(clearBtn_, 0, 3);
        toolbarLayout_->addWidget(overflowLabel_, 0, 4);
        toolbarLayout_->setColumnStretch(5, 1);

        toolbarLayout_->addWidget(pollFramesCheck_, 1, 0, 1, 2);
        toolbarLayout_->addWidget(exportBtn_, 1, 2);
        toolbarLayout_->addWidget(liveAnalyzerBtn_, 1, 3);
        toolbarLayout_->setColumnStretch(5, 1);
    } else {
        toolbarLayout_->addWidget(filterLabel_, 0, 0);
        toolbarLayout_->addWidget(filterCombo_, 0, 1);
        toolbarLayout_->addWidget(pollFramesCheck_, 0, 2);
        toolbarLayout_->addWidget(autoScrollBtn_, 0, 3);
        toolbarLayout_->addWidget(clearBtn_, 0, 4);
        toolbarLayout_->addWidget(exportBtn_, 0, 5);
        toolbarLayout_->addWidget(liveAnalyzerBtn_, 0, 6);
        toolbarLayout_->addWidget(overflowLabel_, 0, 7);
        toolbarLayout_->setColumnStretch(8, 1);
    }
}

int ModbusTelemetryWidget::singleRowToolbarRequiredWidth() const {
    if (!toolbarLayout_) return 560;
    const QWidget* widgets[] = {
        filterLabel_, filterCombo_, pollFramesCheck_, autoScrollBtn_,
        clearBtn_, exportBtn_, liveAnalyzerBtn_
    };
    int totalWidth = 0;
    int count = 0;
    for (const auto* w : widgets) {
        if (w) {
            totalWidth += w->sizeHint().width();
            count++;
        }
    }
    if (overflowLabel_ && overflowLabel_->isVisible()) {
        totalWidth += overflowLabel_->sizeHint().width();
        count++;
    }
    const int spacing = toolbarLayout_->horizontalSpacing();
    if (count > 1) {
        totalWidth += (count - 1) * (spacing > 0 ? spacing : 6);
    }
    return totalWidth + 20;
}

void ModbusTelemetryWidget::updateResponsiveToolbar(int availableWidth) {
    if (availableWidth <= 0) return;
    const int collapseThreshold = singleRowToolbarRequiredWidth();
    const int expandThreshold = collapseThreshold + 30;

    if (isTwoRowToolbar_ && availableWidth >= expandThreshold) {
        applyToolbarLayout(false);
    } else if (!isTwoRowToolbar_ && availableWidth < collapseThreshold) {
        applyToolbarLayout(true);
    }
}

void ModbusTelemetryWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (event) {
        updateResponsiveToolbar(event->size().width());
        updateResponsiveInspectorHeader(event->size().width());
    }
}

void ModbusTelemetryWidget::setupMetricsRibbon() {
    metricsRibbon_ = new QWidget(this);
    metricsRibbon_->setObjectName(QStringLiteral("telemetryMetricsRibbon"));
    auto* ribbonLayout = new QHBoxLayout(metricsRibbon_);
    ribbonLayout->setContentsMargins(0, 2, 0, 2);
    ribbonLayout->setSpacing(12);

    txStatsLabel_ = new QLabel(metricsRibbon_);
    txStatsLabel_->setObjectName(QStringLiteral("txStatsLabel"));
    txStatsLabel_->setStyleSheet(QStringLiteral("color: #1976D2; font-weight: bold; font-family: monospace;"));
    ribbonLayout->addWidget(txStatsLabel_);

    rxStatsLabel_ = new QLabel(metricsRibbon_);
    rxStatsLabel_->setObjectName(QStringLiteral("rxStatsLabel"));
    rxStatsLabel_->setStyleSheet(QStringLiteral("color: #388E3C; font-weight: bold; font-family: monospace;"));
    ribbonLayout->addWidget(rxStatsLabel_);

    errStatsLabel_ = new QLabel(metricsRibbon_);
    errStatsLabel_->setObjectName(QStringLiteral("errStatsLabel"));
    ribbonLayout->addWidget(errStatsLabel_);

    rttStatsLabel_ = new QLabel(metricsRibbon_);
    rttStatsLabel_->setObjectName(QStringLiteral("rttStatsLabel"));
    rttStatsLabel_->setStyleSheet(QStringLiteral("color: #616161; font-family: monospace;"));
    ribbonLayout->addWidget(rttStatsLabel_);

    ribbonLayout->addStretch(1);

    resetStatsBtn_ = new QPushButton(tr("Reset Stats"), metricsRibbon_);
    resetStatsBtn_->setObjectName(QStringLiteral("resetStatsBtn"));
    resetStatsBtn_->setToolTip(tr("Reset communication statistics (TX, RX, Error, RTT)"));
    connect(resetStatsBtn_, &QPushButton::clicked, this, &ModbusTelemetryWidget::resetMetricsRequested);
    ribbonLayout->addWidget(resetStatsBtn_);

    updateMetricsRibbon();
    qobject_cast<QVBoxLayout*>(layout())->addWidget(metricsRibbon_);
}

void ModbusTelemetryWidget::setupTableView() {
    tableModel_ = new TelemetryTableModel(this);
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(tableModel_);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);

    tableView_ = new QTableView(this);
    tableView_->setModel(proxyModel_);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setAlternatingRowColors(true);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->horizontalHeader()->setStretchLastSection(false);

    auto* hHeader = tableView_->horizontalHeader();
    hHeader->setSectionResizeMode(TelemetryTableModel::ColTimestamp, QHeaderView::ResizeToContents);
    hHeader->setSectionResizeMode(TelemetryTableModel::ColDirection, QHeaderView::ResizeToContents);
    hHeader->setSectionResizeMode(TelemetryTableModel::ColLength, QHeaderView::ResizeToContents);
    hHeader->setSectionResizeMode(TelemetryTableModel::ColPayload, QHeaderView::Stretch);
    hHeader->setSectionResizeMode(TelemetryTableModel::ColStatus, QHeaderView::ResizeToContents);

    tableView_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ModbusTelemetryWidget::onSelectionChanged);

    auto* vBar = tableView_->verticalScrollBar();
    connect(vBar, &QScrollBar::valueChanged, this, [this, vBar](int value) {
        if (value < vBar->maximum() - 2) {
            if (autoScroll_) {
                autoScroll_ = false;
                if (autoScrollBtn_) {
                    autoScrollBtn_->setChecked(false);
                }
            }
        }
    });

    connect(tableView_, &QTableView::doubleClicked, this, [this](const QModelIndex&) {
        onInspectInAnalyzerClicked();
    });

    splitter_->addWidget(tableView_);
}

void ModbusTelemetryWidget::setupInspector() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    inspectorHeaderWidget_ = new QWidget(container);
    inspectorHeaderWidget_->setObjectName(QStringLiteral("inspectorHeaderWidget"));
    inspectorHeaderLayout_ = new QGridLayout(inspectorHeaderWidget_);
    inspectorHeaderLayout_->setContentsMargins(2, 2, 2, 2);
    inspectorHeaderLayout_->setSpacing(4);

    inspectorHeaderLabel_ = new QLabel(tr("Frame Details"), inspectorHeaderWidget_);
    inspectorHeaderLabel_->setStyleSheet(QStringLiteral("font-weight: bold; background-color: palette(alternate-base); padding: 2px;"));

    decodeLabel_ = new QLabel(tr("Decode:"), inspectorHeaderWidget_);
    decodeTypeCombo_ = new QComboBox(inspectorHeaderWidget_);
    decodeTypeCombo_->setObjectName(QStringLiteral("inspectorDecodeTypeCombo"));
    decodeTypeCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    decodeTypeCombo_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    decodeTypeCombo_->addItem(QStringLiteral("UInt16"), static_cast<int>(::modbus::analyzer::RegisterDataType::UInt16));
    decodeTypeCombo_->addItem(QStringLiteral("Int16"), static_cast<int>(::modbus::analyzer::RegisterDataType::Int16));
    decodeTypeCombo_->addItem(QStringLiteral("UInt32"), static_cast<int>(::modbus::analyzer::RegisterDataType::UInt32));
    decodeTypeCombo_->addItem(QStringLiteral("Int32"), static_cast<int>(::modbus::analyzer::RegisterDataType::Int32));
    decodeTypeCombo_->addItem(QStringLiteral("Float32"), static_cast<int>(::modbus::analyzer::RegisterDataType::Float32));
    decodeTypeCombo_->addItem(QStringLiteral("Float64"), static_cast<int>(::modbus::analyzer::RegisterDataType::Float64));

    orderLabel_ = new QLabel(tr("Order:"), inspectorHeaderWidget_);
    orderCombo_ = new QComboBox(inspectorHeaderWidget_);
    orderCombo_->setObjectName(QStringLiteral("inspectorOrderCombo"));
    orderCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    orderCombo_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    orderCombo_->addItem(QStringLiteral("ABCD"), static_cast<int>(::modbus::base::RegisterOrder::ABCD));
    orderCombo_->addItem(QStringLiteral("CDAB"), static_cast<int>(::modbus::base::RegisterOrder::CDAB));
    orderCombo_->addItem(QStringLiteral("BADC"), static_cast<int>(::modbus::base::RegisterOrder::BADC));
    orderCombo_->addItem(QStringLiteral("DCBA"), static_cast<int>(::modbus::base::RegisterOrder::DCBA));

    connect(decodeTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTelemetryWidget::onDecodeConfigChanged);
    connect(orderCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTelemetryWidget::onDecodeConfigChanged);

    inspectInAnalyzerBtn_ = new QPushButton(tr("Inspect in Analyzer ↗"), inspectorHeaderWidget_);
    inspectInAnalyzerBtn_->setObjectName(QStringLiteral("inspectInAnalyzerBtn"));
    inspectInAnalyzerBtn_->setEnabled(false);
    inspectInAnalyzerBtn_->setToolTip(tr("Send the selected frame payload to Frame Analyzer for deep parsing"));
    connect(inspectInAnalyzerBtn_, &QPushButton::clicked, this, &ModbusTelemetryWidget::onInspectInAnalyzerClicked);

    applyInspectorHeaderLayout(false);
    layout->addWidget(inspectorHeaderWidget_);

    inspectorDetailsText_ = new QTextEdit(container);
    inspectorDetailsText_->setReadOnly(true);
    inspectorDetailsText_->setLineWrapMode(QTextEdit::NoWrap);
    inspectorDetailsText_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    inspectorDetailsText_->setPlaceholderText(tr("Select a frame to inspect details"));
    layout->addWidget(inspectorDetailsText_, 1);

    splitter_->addWidget(container);
}

void ModbusTelemetryWidget::applyInspectorHeaderLayout(bool twoRow) {
    if (!inspectorHeaderLayout_) return;

    isTwoRowInspectorHeader_ = twoRow;

    inspectorHeaderLayout_->removeWidget(inspectorHeaderLabel_);
    inspectorHeaderLayout_->removeWidget(decodeLabel_);
    inspectorHeaderLayout_->removeWidget(decodeTypeCombo_);
    inspectorHeaderLayout_->removeWidget(orderLabel_);
    inspectorHeaderLayout_->removeWidget(orderCombo_);
    inspectorHeaderLayout_->removeWidget(inspectInAnalyzerBtn_);

    if (twoRow) {
        inspectorHeaderLayout_->addWidget(inspectorHeaderLabel_, 0, 0, 1, 4);
        inspectorHeaderLayout_->addWidget(inspectInAnalyzerBtn_, 0, 5, 1, 1, Qt::AlignRight);
        inspectorHeaderLayout_->addWidget(decodeLabel_, 1, 0);
        inspectorHeaderLayout_->addWidget(decodeTypeCombo_, 1, 1);
        inspectorHeaderLayout_->addWidget(orderLabel_, 1, 2);
        inspectorHeaderLayout_->addWidget(orderCombo_, 1, 3);
        inspectorHeaderLayout_->setColumnStretch(0, 0);
        inspectorHeaderLayout_->setColumnStretch(1, 0);
        inspectorHeaderLayout_->setColumnStretch(2, 0);
        inspectorHeaderLayout_->setColumnStretch(3, 0);
        inspectorHeaderLayout_->setColumnStretch(4, 1);
        inspectorHeaderLayout_->setColumnStretch(5, 0);
        inspectorHeaderLayout_->setColumnStretch(6, 0);
    } else {
        inspectorHeaderLayout_->addWidget(inspectorHeaderLabel_, 0, 0);
        inspectorHeaderLayout_->addWidget(decodeLabel_, 0, 1);
        inspectorHeaderLayout_->addWidget(decodeTypeCombo_, 0, 2);
        inspectorHeaderLayout_->addWidget(orderLabel_, 0, 3);
        inspectorHeaderLayout_->addWidget(orderCombo_, 0, 4);
        inspectorHeaderLayout_->addWidget(inspectInAnalyzerBtn_, 0, 6, Qt::AlignRight);
        inspectorHeaderLayout_->setColumnStretch(0, 0);
        inspectorHeaderLayout_->setColumnStretch(1, 0);
        inspectorHeaderLayout_->setColumnStretch(2, 0);
        inspectorHeaderLayout_->setColumnStretch(3, 0);
        inspectorHeaderLayout_->setColumnStretch(4, 0);
        inspectorHeaderLayout_->setColumnStretch(5, 1);
        inspectorHeaderLayout_->setColumnStretch(6, 0);
    }
}

int ModbusTelemetryWidget::singleRowInspectorHeaderRequiredWidth() const {
    if (!inspectorHeaderLayout_) return 480;
    const QWidget* widgets[] = {
        inspectorHeaderLabel_, decodeLabel_, decodeTypeCombo_,
        orderLabel_, orderCombo_, inspectInAnalyzerBtn_
    };
    int totalWidth = 0;
    int count = 0;
    for (const auto* w : widgets) {
        if (w) {
            totalWidth += w->sizeHint().width();
            count++;
        }
    }
    const int spacing = inspectorHeaderLayout_->horizontalSpacing();
    if (count > 1) {
        totalWidth += (count - 1) * (spacing > 0 ? spacing : 4);
    }
    return totalWidth + 20;
}

void ModbusTelemetryWidget::updateResponsiveInspectorHeader(int availableWidth) {
    if (availableWidth <= 0) return;
    const int collapseThreshold = singleRowInspectorHeaderRequiredWidth();
    const int expandThreshold = collapseThreshold + 30;

    if (isTwoRowInspectorHeader_ && availableWidth >= expandThreshold) {
        applyInspectorHeaderLayout(false);
    } else if (!isTwoRowInspectorHeader_ && availableWidth < collapseThreshold) {
        applyInspectorHeaderLayout(true);
    }
}

void ModbusTelemetryWidget::setMetadataMap(const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& meta) {
    metadataByAddress_ = meta;
    const auto* selModel = tableView_ ? tableView_->selectionModel() : nullptr;
    if (selModel && selModel->hasSelection() && proxyModel_ && tableModel_) {
        const auto indexes = selModel->selectedRows();
        if (!indexes.isEmpty()) {
            const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
            updateInspectorContent(tableModel_->rowAt(sourceRow));
        }
    }
}

void ModbusTelemetryWidget::setDecodeType(::modbus::analyzer::RegisterDataType type) {
    currentDataType_ = type;
    if (decodeTypeCombo_) {
        const int idx = decodeTypeCombo_->findData(static_cast<int>(type));
        if (idx >= 0 && decodeTypeCombo_->currentIndex() != idx) {
            decodeTypeCombo_->setCurrentIndex(idx);
        }
    }
}

void ModbusTelemetryWidget::setRegisterOrder(::modbus::base::RegisterOrder order) {
    currentOrder_ = order;
    if (orderCombo_) {
        const int idx = orderCombo_->findData(static_cast<int>(order));
        if (idx >= 0 && orderCombo_->currentIndex() != idx) {
            orderCombo_->setCurrentIndex(idx);
        }
    }
}

void ModbusTelemetryWidget::applyDecodeCapability(const ::modbus::analyzer::value_formatter::FrameDecodeEvaluation& eval) {
    if (!decodeTypeCombo_ || !orderCombo_) return;

    decodeTypeCombo_->setEnabled(eval.enableDecodeCombo);
    orderCombo_->setEnabled(eval.enableOrderCombo);

    if (eval.capability == ::modbus::analyzer::value_formatter::FrameDecodeCapability::SingleRegister) {
        if (currentDataType_ != ::modbus::analyzer::RegisterDataType::UInt16 &&
            currentDataType_ != ::modbus::analyzer::RegisterDataType::Int16) {
            setDecodeType(::modbus::analyzer::RegisterDataType::UInt16);
        }
        if (auto* model = qobject_cast<QStandardItemModel*>(decodeTypeCombo_->model())) {
            for (int i = 0; i < decodeTypeCombo_->count(); ++i) {
                const auto t = static_cast<::modbus::analyzer::RegisterDataType>(decodeTypeCombo_->itemData(i).toInt());
                const bool is16 = (t == ::modbus::analyzer::RegisterDataType::UInt16 || t == ::modbus::analyzer::RegisterDataType::Int16);
                if (auto* item = model->item(i)) {
                    item->setEnabled(is16);
                }
            }
        }
        decodeTypeCombo_->setToolTip(tr("16-bit scalar register"));
        orderCombo_->setToolTip(eval.capabilityHint);
    } else if (eval.capability == ::modbus::analyzer::value_formatter::FrameDecodeCapability::FullRegisters) {
        if (auto* model = qobject_cast<QStandardItemModel*>(decodeTypeCombo_->model())) {
            for (int i = 0; i < decodeTypeCombo_->count(); ++i) {
                if (auto* item = model->item(i)) {
                    item->setEnabled(true);
                }
            }
        }
        decodeTypeCombo_->setToolTip(tr("Target register data representation type"));
        orderCombo_->setToolTip(tr("Multi-word byte/word ordering convention"));
    } else {
        decodeTypeCombo_->setToolTip(eval.capabilityHint);
        orderCombo_->setToolTip(eval.capabilityHint);
    }
}

void ModbusTelemetryWidget::onDecodeConfigChanged() {
    if (decodeTypeCombo_) {
        currentDataType_ = static_cast<::modbus::analyzer::RegisterDataType>(
            decodeTypeCombo_->currentData().toInt());
    }
    if (orderCombo_) {
        currentOrder_ = static_cast<::modbus::base::RegisterOrder>(
            orderCombo_->currentData().toInt());
    }
    const auto* selModel = tableView_ ? tableView_->selectionModel() : nullptr;
    if (selModel && selModel->hasSelection() && proxyModel_ && tableModel_) {
        const auto indexes = selModel->selectedRows();
        if (!indexes.isEmpty()) {
            const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
            updateInspectorContent(tableModel_->rowAt(sourceRow));
        }
    }
}

void ModbusTelemetryWidget::onInspectInAnalyzerClicked() {
    if (!tableView_ || !proxyModel_ || !tableModel_) {
        return;
    }
    const auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }
    const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const TelemetryRowView* row = tableModel_->rowAt(sourceRow);
    if (!row || row->event.payload.isEmpty()) {
        return;
    }
    const QString hexStr = row->event.payload.toHex(' ').toUpper();
    emit inspectInAnalyzerRequested(hexStr);
}

void ModbusTelemetryWidget::appendTrafficEvent(const ::ui::common::TrafficEvent& event) {
    if (pendingEvents_.size() >= kMaxPendingEvents) {
        const int overflowDrop = pendingEvents_.size() - kMaxPendingEvents + 1;
        for (int i = 0; i < overflowDrop; ++i) {
            pendingEvents_.removeFirst();
        }
        droppedEventsCount_ += overflowDrop;
        overflowLabel_->setText(tr("Queue overflow: %1 dropped").arg(droppedEventsCount_));
        overflowLabel_->show();
    }
    pendingEvents_.append(event);
    if (batchTimer_ && !batchTimer_->isActive()) {
        batchTimer_->start();
    }
}

void ModbusTelemetryWidget::processBatchTimeout() {
    if (pendingEvents_.isEmpty()) {
        if (batchTimer_ && batchTimer_->isActive()) {
            batchTimer_->stop();
        }
        return;
    }

    const int takeCount = std::min(static_cast<int>(pendingEvents_.size()), kMaxBatchSize);
    QList<TelemetryRowView> rowsToAdd;
    rowsToAdd.reserve(takeCount);

    for (int i = 0; i < takeCount; ++i) {
        const auto event = pendingEvents_.takeFirst();
        TelemetryRowView row;
        row.rowId = tableModel_->nextRowId();
        row.event = event;
        row.protocol = currentProtocol_;
        if (!event.payload.isEmpty()) {
            row.parseResult = ::modbus::parser::parse(event.payload, currentProtocol_);
        }
        deriveRowStatus(event, row.parseResult, row.statusText, row.statusColor);
        rowsToAdd.append(row);
    }

    tableModel_->appendRows(rowsToAdd);
    if (autoScroll_) {
        tableView_->scrollToBottom();
    }
    if (pendingEvents_.isEmpty() && batchTimer_ && batchTimer_->isActive()) {
        batchTimer_->stop();
    }
}

void ModbusTelemetryWidget::setProtocolType(::modbus::parser::ProtocolType protocol) noexcept {
    currentProtocol_ = protocol;
}

void ModbusTelemetryWidget::updateMetrics(int tx, int rx, int err, int rttMs) {
    txCount_ = tx;
    rxCount_ = rx;
    errCount_ = err;
    rttMs_ = rttMs;
    updateMetricsRibbon();
}

void ModbusTelemetryWidget::updateMetricsRibbon() {
    if (txStatsLabel_) {
        txStatsLabel_->setText(tr("TX: %1").arg(txCount_));
    }
    if (rxStatsLabel_) {
        rxStatsLabel_->setText(tr("RX: %1").arg(rxCount_));
    }
    if (errStatsLabel_) {
        errStatsLabel_->setText(tr("ERR: %1").arg(errCount_));
        errStatsLabel_->setStyleSheet(errCount_ > 0
            ? QStringLiteral("color: #D32F2F; font-weight: bold; font-family: monospace;")
            : QStringLiteral("color: #757575; font-family: monospace;"));
    }
    if (rttStatsLabel_) {
        rttStatsLabel_->setText(rttMs_ > 0 ? tr("RTT: %1 ms").arg(rttMs_) : tr("RTT: --"));
    }
}

void ModbusTelemetryWidget::setLinkageState(bool active) {
    if (liveAnalyzerBtn_ && liveAnalyzerBtn_->isChecked() != active) {
        liveAnalyzerBtn_->setChecked(active);
    }
}

bool ModbusTelemetryWidget::isLinkageActive() const noexcept {
    return liveAnalyzerBtn_ && liveAnalyzerBtn_->isChecked();
}

void ModbusTelemetryWidget::setAutoScrollEnabled(bool enabled) {
    autoScroll_ = enabled;
    if (autoScrollBtn_ && autoScrollBtn_->isChecked() != enabled) {
        autoScrollBtn_->setChecked(enabled);
    }
    if (enabled && tableView_) {
        tableView_->scrollToBottom();
    }
}

void ModbusTelemetryWidget::clear() {
    pendingEvents_.clear();
    if (batchTimer_ && batchTimer_->isActive()) {
        batchTimer_->stop();
    }
    droppedEventsCount_ = 0;
    overflowLabel_->hide();
    tableModel_->clear();
    inspectorDetailsText_->clear();
    inspectorHeaderLabel_->setText(tr("Frame Details"));
    if (inspectInAnalyzerBtn_) {
        inspectInAnalyzerBtn_->setEnabled(false);
    }
    ::modbus::analyzer::value_formatter::FrameDecodeEvaluation noEval;
    noEval.capabilityHint = tr("Select a frame to inspect details");
    applyDecodeCapability(noEval);
}

void ModbusTelemetryWidget::onSelectionChanged() {
    const auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        ::modbus::analyzer::value_formatter::FrameDecodeEvaluation noEval;
        noEval.capabilityHint = tr("Select a frame to inspect details");
        applyDecodeCapability(noEval);
        updateInspectorContent(nullptr);
        if (inspectInAnalyzerBtn_) {
            inspectInAnalyzerBtn_->setEnabled(false);
        }
        return;
    }
    const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const TelemetryRowView* row = tableModel_->rowAt(sourceRow);
    if (!row) {
        updateInspectorContent(nullptr);
        return;
    }

    const bool isTx = (row->event.direction == ::ui::common::TrafficDirection::Tx);
    const auto eval = ::modbus::analyzer::value_formatter::evaluateFrameDecodeCapability(row->parseResult, isTx);
    applyDecodeCapability(eval);
    updateInspectorContent(row);

    const bool hasPayload = (!row->event.payload.isEmpty());
    if (inspectInAnalyzerBtn_) {
        inspectInAnalyzerBtn_->setEnabled(hasPayload);
    }
    if (hasPayload && isLinkageActive()) {
        emit frameSelectedForAnalysis(row->event.payload);
    }
}

QString ModbusTelemetryWidget::inspectorDetailsText() const noexcept {
    return inspectorDetailsText_ ? inspectorDetailsText_->toPlainText() : QString();
}

QString ModbusTelemetryWidget::formatInspectorDetails(const ::modbus::parser::ParseResult& pr, int aduLength) {
    ModbusTelemetryWidget dummy;
    return dummy.formatInspectorDetails(pr, aduLength, false, ::modbus::analyzer::RegisterDataType::UInt16, ::modbus::base::RegisterOrder::ABCD, {});
}

QString ModbusTelemetryWidget::formatInspectorDetails(
    const ::modbus::parser::ParseResult& pr,
    int aduLength,
    bool isTx,
    ::modbus::analyzer::RegisterDataType type,
    ::modbus::base::RegisterOrder order,
    const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& metadata) const {

    QString details;
    QTextStream out(&details);

    // 1. In-place decoded values / semantic summary at top
    const auto eval = ::modbus::analyzer::value_formatter::evaluateFrameDecodeCapability(pr, isTx);
    const QString decodedSummary = ::modbus::analyzer::value_formatter::formatDecodedSummary(
        pr, eval, type, order, metadata);
    if (!decodedSummary.isEmpty()) {
        out << tr("--- Decoded Values (%1 / %2) ---")
                   .arg(::modbus::analyzer::registerDataTypeToString(type),
                        QString::fromLatin1(::modbus::base::registerOrderToString(order)))
            << "\n" << decodedSummary << "\n\n";
    }

    // 2. Frame structure details
    out << tr("--- Frame Structure ---\n");
    const auto fcHex = QString::number(static_cast<int>(pr.functionCode), 16).rightJustified(2, QLatin1Char('0')).toUpper();
    const auto fcName = QString::fromLatin1(::modbus::base::function_traits::functionCodeName(pr.functionCode));

    if (pr.protocol == ::modbus::parser::ProtocolType::Tcp) {
        const auto tidHex = QString::number(pr.transactionId, 16).rightJustified(4, QLatin1Char('0')).toUpper();
        const auto pidHex = QString::number(pr.protocolId, 16).rightJustified(4, QLatin1Char('0')).toUpper();
        out << tr("[TID: 0x%1]  [PID: 0x%2]  [MBAP Len: %3 B]  [UID: %4]  [FC: 0x%5 (%6)]\n")
                   .arg(tidHex, pidHex).arg(pr.length).arg(pr.slaveId).arg(fcHex, fcName);
    } else {
        out << tr("[Slave: %1]  [ADU Len: %2 B]  [FC: 0x%3 (%4)]\n")
                   .arg(pr.slaveId).arg(aduLength).arg(fcHex, fcName);
    }

    if (pr.isException) {
        const auto exHex = QString::number(static_cast<int>(pr.exceptionCode), 16).rightJustified(2, QLatin1Char('0')).toUpper();
        const auto exName = QString::fromLatin1(::modbus::base::function_traits::exceptionCodeName(pr.exceptionCode));
        out << tr("[Exception Code: 0x%1 (%2)]\n").arg(exHex, exName);
    } else if (!pr.dataItems.isEmpty()) {
        for (int i = 0; i < pr.dataItems.size(); ++i) {
            const QString item = QStringLiteral("[+%1] %2")
                .arg(i, 2, 10, QLatin1Char('0'))
                .arg(pr.dataItems[i].value.toString())
                .leftJustified(18, QLatin1Char(' '));
            out << item;
            if ((i + 1) % 4 == 0 || i + 1 == pr.dataItems.size()) {
                out << QLatin1Char('\n');
            }
        }
    } else if (!pr.isValid && !pr.error.isEmpty()) {
        out << tr("[Error: %1]\n").arg(pr.error);
    }
    return details;
}

void ModbusTelemetryWidget::updateInspectorContent(const TelemetryRowView* row) {
    if (!row) {
        inspectorHeaderLabel_->setText(tr("Frame Details"));
        inspectorDetailsText_->clear();
        return;
    }

    if (row->event.direction == ::ui::common::TrafficDirection::None) {
        inspectorHeaderLabel_->setText(tr("[SYS] %1").arg(row->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz"))));
        inspectorDetailsText_->setPlainText(row->event.summary);
        return;
    }

    const bool isTx = (row->event.direction == ::ui::common::TrafficDirection::Tx);
    const auto dirStr = isTx ? QStringLiteral("TX") : QStringLiteral("RX");
    inspectorHeaderLabel_->setText(tr("[#%1] %2 | %3 | %4 bytes | %5")
                         .arg(row->rowId)
                         .arg(dirStr)
                         .arg(row->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")))
                         .arg(row->event.payload.size())
                         .arg(row->statusText));

    inspectorDetailsText_->setPlainText(formatInspectorDetails(
        row->parseResult, row->event.payload.size(), isTx, currentDataType_, currentOrder_, metadataByAddress_));
}

void ModbusTelemetryWidget::onFilterChanged(int index) {
    if (index == 0) { // All
        proxyModel_->setFilterKeyColumn(-1);
        proxyModel_->setFilterFixedString(QString());
    } else if (index == 1) { // TX
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColDirection);
        proxyModel_->setFilterFixedString(QStringLiteral("TX"));
    } else if (index == 2) { // RX
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColDirection);
        proxyModel_->setFilterFixedString(QStringLiteral("RX"));
    } else if (index == 3) { // SYS
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColDirection);
        proxyModel_->setFilterFixedString(QStringLiteral("SYS"));
    } else if (index == 4) { // Err & Warn
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColStatus);
        proxyModel_->setFilterRegularExpression(QStringLiteral("Error|Warning|Exception|Malformed"));
    } else if (index == 5) { // Errors
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColStatus);
        proxyModel_->setFilterRegularExpression(QStringLiteral("Error|Exception|Malformed"));
    } else if (index == 6) { // Warnings
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColStatus);
        proxyModel_->setFilterRegularExpression(QStringLiteral("Warning"));
    }
}

QStringList ModbusTelemetryWidget::buildCsvLines() const {
    QStringList lines;
    lines.reserve(tableModel_->rowCount() + 1);
    lines.append(QStringLiteral("RowId,Timestamp,Direction,Length,PayloadHex,Status"));
    for (int i = 0; i < tableModel_->rowCount(); ++i) {
        const auto* r = tableModel_->rowAt(i);
        if (!r) continue;
        const QString payloadOrSummary =
            (r->event.direction == ::ui::common::TrafficDirection::None || r->event.payload.isEmpty())
                ? r->event.summary
                : QString::fromLatin1(r->event.payload.toHex(' ').toUpper());
        lines.append(QStringLiteral("%1,%2,%3,%4,%5,%6")
                         .arg(r->rowId)
                         .arg(csvEscapeField(r->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz"))))
                         .arg(directionLabel(r->event.direction))
                         .arg(r->event.payload.size())
                         .arg(csvEscapeField(payloadOrSummary))
                         .arg(csvEscapeField(r->statusText)));
    }
    return lines;
}

bool ModbusTelemetryWidget::exportJsonToFile(const QString& filePath, QString* error) const {
    QJsonArray arr;
    for (int i = 0; i < tableModel_->rowCount(); ++i) {
        const auto* r = tableModel_->rowAt(i);
        if (!r) continue;
        QJsonObject obj;
        obj[QStringLiteral("id")] = static_cast<qint64>(r->rowId);
        obj[QStringLiteral("timestamp")] = r->event.timestamp.toString(Qt::ISODateWithMs);
        obj[QStringLiteral("direction")] = directionLabel(r->event.direction);
        obj[QStringLiteral("payloadHex")] = QString::fromLatin1(r->event.payload.toHex());
        obj[QStringLiteral("status")] = r->statusText;
        arr.append(obj);
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error = file.errorString();
        return false;
    }
    const QByteArray payload = QJsonDocument(arr).toJson();
    if (file.write(payload) != payload.size() || file.error() != QFileDevice::NoError) {
        if (error) *error = file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}

bool ModbusTelemetryWidget::exportToFile(const QString& filePath, QString* error) const {
    if (filePath.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
        return exportJsonToFile(filePath, error);
    }
    return ::ui::common::exportLinesToFile(filePath, buildCsvLines(), false, error);
}

void ModbusTelemetryWidget::onExportClicked() {
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Export Telemetry"), QString(),
        tr("CSV Files (*.csv);;JSON Files (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }
    QString errorMsg;
    if (!exportToFile(filePath, &errorMsg)) {
        emit exportFailed(errorMsg);
        QMessageBox::warning(this, tr("Export Failed"),
                             tr("Failed to export telemetry data: %1").arg(errorMsg));
    }
}

QString ModbusTelemetryWidget::formatRowTextForCopy(const TelemetryRowView& row) {
    if (row.event.direction == ::ui::common::TrafficDirection::None) {
        return QStringLiteral("[%1] %2 | %3 | %4")
            .arg(row.event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")))
            .arg(directionLabel(row.event.direction))
            .arg(row.statusText)
            .arg(row.event.summary);
    }
    return QStringLiteral("[%1] %2 | %3 | %4 bytes\n%5")
        .arg(row.event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")))
        .arg(directionLabel(row.event.direction))
        .arg(row.statusText)
        .arg(row.event.payload.size())
        .arg(QString::fromLatin1(row.event.payload.toHex(' ').toUpper()));
}

void ModbusTelemetryWidget::onCopySelectedRow() {
    auto* selModel = tableView_ ? tableView_->selectionModel() : nullptr;
    if (!selModel) {
        return;
    }
    const auto indexes = selModel->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }
    const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const TelemetryRowView* row = tableModel_->rowAt(sourceRow);
    if (!row) {
        return;
    }

    const QString text = formatRowTextForCopy(*row);
    QGuiApplication::clipboard()->setText(text);
    emit rowCopied(text);
}

void ModbusTelemetryWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_L && (event->modifiers() & Qt::ControlModifier)) {
        clear();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Space) {
        setAutoScrollEnabled(!autoScroll_);
        event->accept();
        return;
    }
    if (event->matches(QKeySequence::Copy)) {
        onCopySelectedRow();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ModbusTelemetryWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void ModbusTelemetryWidget::retranslateToolbarUi() {
    if (filterLabel_) {
        filterLabel_->setText(tr("Filter:"));
    }
    if (filterCombo_) {
        const int cur = filterCombo_->currentIndex();
        filterCombo_->setItemText(0, tr("All"));
        filterCombo_->setItemText(1, tr("TX"));
        filterCombo_->setItemText(2, tr("RX"));
        filterCombo_->setItemText(3, tr("SYS"));
        filterCombo_->setItemText(4, tr("Err & Warn"));
        filterCombo_->setItemText(5, tr("Errors"));
        filterCombo_->setItemText(6, tr("Warnings"));
        filterCombo_->setCurrentIndex(cur);
        filterCombo_->setToolTip(tr("Filter telemetry log by direction or event severity"));
    }
    if (pollFramesCheck_) {
        pollFramesCheck_->setText(tr("Poll Frames"));
        pollFramesCheck_->setToolTip(tr("Sample 1 in 10 polling request/response frame pairs in traffic log; manual requests are always logged in full"));
    }
    if (autoScrollBtn_) {
        autoScrollBtn_->setText(tr("Auto-Scroll"));
        autoScrollBtn_->setToolTip(tr("Automatically scroll to latest telemetry event"));
    }
    if (clearBtn_) {
        clearBtn_->setText(tr("Clear"));
        clearBtn_->setToolTip(tr("Clear telemetry events (Ctrl+L)"));
    }
    if (exportBtn_) {
        exportBtn_->setText(tr("Export..."));
        exportBtn_->setToolTip(tr("Export telemetry table to CSV or JSON"));
    }
    if (liveAnalyzerBtn_) {
        liveAnalyzerBtn_->setText(tr("Live → Analyzer"));
        liveAnalyzerBtn_->setToolTip(tr("Stream received responses directly to Frame Analyzer in real time"));
    }
    if (resetStatsBtn_) {
        resetStatsBtn_->setText(tr("Reset Stats"));
        resetStatsBtn_->setToolTip(tr("Reset communication statistics (TX, RX, Error, RTT)"));
    }
}

void ModbusTelemetryWidget::retranslateInspectorUi() {
    if (inspectInAnalyzerBtn_) {
        inspectInAnalyzerBtn_->setText(tr("Inspect in Analyzer ↗"));
        inspectInAnalyzerBtn_->setToolTip(tr("Send the selected frame payload to Frame Analyzer for deep parsing"));
    }
    if (decodeLabel_) {
        decodeLabel_->setText(tr("Decode:"));
    }
    if (orderLabel_) {
        orderLabel_->setText(tr("Order:"));
    }
    if (inspectorHeaderLabel_) {
        const auto* selModel = tableView_ ? tableView_->selectionModel() : nullptr;
        if (!selModel || !selModel->hasSelection()) {
            inspectorHeaderLabel_->setText(tr("Frame Details"));
        } else {
            const auto indexes = selModel->selectedRows();
            if (!indexes.isEmpty() && proxyModel_ && tableModel_) {
                const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
                updateInspectorContent(tableModel_->rowAt(sourceRow));
            }
        }
    }
    if (inspectorDetailsText_) {
        inspectorDetailsText_->setPlaceholderText(tr("Select a frame to inspect details"));
    }
}

void ModbusTelemetryWidget::retranslateUi() {
    retranslateToolbarUi();
    retranslateInspectorUi();
    updateMetricsRibbon();
    if (tableModel_) {
        tableModel_->retranslate();
    }
    updateResponsiveToolbar(width());
    updateResponsiveInspectorHeader(width());
}

} // namespace ui::widgets::modbus

