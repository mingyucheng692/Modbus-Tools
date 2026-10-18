// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusTelemetryWidget.h"
#include "modbus/base/ModbusFunctionTraits.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSplitter>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QGuiApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QFontDatabase>

namespace ui::widgets::modbus {

ModbusTelemetryWidget::ModbusTelemetryWidget(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void ModbusTelemetryWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    setupToolbar();

    splitter_ = new QSplitter(Qt::Vertical, this);
    setupTableView();
    setupInspector();

    splitter_->setStretchFactor(0, 7);
    splitter_->setStretchFactor(1, 3);
    mainLayout->addWidget(splitter_, 1);

    setupFooter();

    batchTimer_ = new QTimer(this);
    batchTimer_->setInterval(kBatchIntervalMs);
    connect(batchTimer_, &QTimer::timeout, this, &ModbusTelemetryWidget::processBatchTimeout);
    batchTimer_->start();
}

void ModbusTelemetryWidget::setupToolbar() {
    auto* toolLayout = new QHBoxLayout();
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->setSpacing(6);

    filterCombo_ = new QComboBox(this);
    filterCombo_->addItems({tr("All"), tr("TX"), tr("RX"), tr("SYS"), tr("Errors")});
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTelemetryWidget::onFilterChanged);
    toolLayout->addWidget(new QLabel(tr("Filter:"), this));
    toolLayout->addWidget(filterCombo_);

    rawFramesCheck_ = new QCheckBox(tr("Raw Frames"), this);
    connect(rawFramesCheck_, &QCheckBox::toggled, this, &ModbusTelemetryWidget::rawFramesToggled);
    toolLayout->addWidget(rawFramesCheck_);

    autoScrollBtn_ = new QPushButton(tr("Auto-Scroll: ON"), this);
    autoScrollBtn_->setCheckable(true);
    autoScrollBtn_->setChecked(true);
    connect(autoScrollBtn_, &QPushButton::toggled, this, &ModbusTelemetryWidget::setAutoScrollEnabled);
    toolLayout->addWidget(autoScrollBtn_);

    clearBtn_ = new QPushButton(tr("Clear (Ctrl+L)"), this);
    connect(clearBtn_, &QPushButton::clicked, this, &ModbusTelemetryWidget::clear);
    toolLayout->addWidget(clearBtn_);

    exportBtn_ = new QPushButton(tr("Export..."), this);
    connect(exportBtn_, &QPushButton::clicked, this, &ModbusTelemetryWidget::onExportClicked);
    toolLayout->addWidget(exportBtn_);

    overflowLabel_ = new QLabel(this);
    overflowLabel_->setStyleSheet(QStringLiteral("color: #DC3545; font-weight: bold;"));
    overflowLabel_->hide();
    toolLayout->addWidget(overflowLabel_);

    toolLayout->addStretch(1);
    qobject_cast<QVBoxLayout*>(layout())->addLayout(toolLayout);
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
                autoScrollBtn_->setChecked(false);
                autoScrollBtn_->setText(tr("Auto-Scroll: OFF"));
            }
        }
    });

    splitter_->addWidget(tableView_);
}

void ModbusTelemetryWidget::setupInspector() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    inspectorHeaderLabel_ = new QLabel(tr("Frame Details"), container);
    inspectorHeaderLabel_->setStyleSheet(QStringLiteral("font-weight: bold; background-color: palette(alternate-base); padding: 2px;"));
    layout->addWidget(inspectorHeaderLabel_);

    inspectorDetailsText_ = new QTextEdit(container);
    inspectorDetailsText_->setReadOnly(true);
    inspectorDetailsText_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    inspectorDetailsText_->setPlaceholderText(tr("Select a frame to inspect details"));
    layout->addWidget(inspectorDetailsText_, 1);

    splitter_->addWidget(container);
}

void ModbusTelemetryWidget::setupFooter() {
    auto* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(10);

    metricsLabel_ = new QLabel(this);
    updateMetricsLabel();
    footerLayout->addWidget(metricsLabel_);

    footerLayout->addStretch(1);

    linkageCheck_ = new QCheckBox(tr("Link to Frame Analyzer"), this);
    linkageCheck_->setChecked(true);
    connect(linkageCheck_, &QCheckBox::toggled, this, &ModbusTelemetryWidget::linkageToggled);
    footerLayout->addWidget(linkageCheck_);

    qobject_cast<QVBoxLayout*>(layout())->addLayout(footerLayout);
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
}

void ModbusTelemetryWidget::processBatchTimeout() {
    if (pendingEvents_.isEmpty()) {
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
}

void ModbusTelemetryWidget::setProtocolType(::modbus::parser::ProtocolType protocol) noexcept {
    currentProtocol_ = protocol;
}

void ModbusTelemetryWidget::updateMetrics(int tx, int rx, int err, int rttMs) {
    txCount_ = tx;
    rxCount_ = rx;
    errCount_ = err;
    rttMs_ = rttMs;
    updateMetricsLabel();
}

void ModbusTelemetryWidget::updateMetricsLabel() {
    metricsLabel_->setText(tr("TX: %1 | RX: %2 | Error: %3 | RTT: %4 ms")
                               .arg(txCount_)
                               .arg(rxCount_)
                               .arg(errCount_)
                               .arg(rttMs_));
}

void ModbusTelemetryWidget::setLinkageState(bool active) {
    if (linkageCheck_->isChecked() != active) {
        linkageCheck_->setChecked(active);
    }
}

bool ModbusTelemetryWidget::isLinkageActive() const noexcept {
    return linkageCheck_->isChecked();
}

void ModbusTelemetryWidget::setAutoScrollEnabled(bool enabled) {
    autoScroll_ = enabled;
    autoScrollBtn_->setChecked(enabled);
    autoScrollBtn_->setText(enabled ? tr("Auto-Scroll: ON") : tr("Auto-Scroll: OFF"));
    if (enabled) {
        tableView_->scrollToBottom();
    }
}

void ModbusTelemetryWidget::clear() {
    pendingEvents_.clear();
    droppedEventsCount_ = 0;
    overflowLabel_->hide();
    tableModel_->clear();
    inspectorDetailsText_->clear();
    inspectorHeaderLabel_->setText(tr("Frame Details"));
}

void ModbusTelemetryWidget::onSelectionChanged() {
    const auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        updateInspectorContent(nullptr);
        return;
    }
    const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const TelemetryRowView* row = tableModel_->rowAt(sourceRow);
    updateInspectorContent(row);

    if (row && !row->event.payload.isEmpty() && linkageCheck_->isChecked()) {
        emit frameSelectedForAnalysis(row->event.payload);
    }
}

void ModbusTelemetryWidget::updateInspectorContent(const TelemetryRowView* row) {
    if (!row) {
        inspectorHeaderLabel_->setText(tr("Frame Details"));
        inspectorDetailsText_->clear();
        return;
    }

    if (row->event.direction == ::ui::common::TrafficDirection::None) {
        inspectorHeaderLabel_->setText(tr("[SYS] %1").arg(row->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz"))));
        inspectorDetailsText_->setText(row->event.summary);
        return;
    }

    const auto& pr = row->parseResult;
    QString header = tr("[#%1] %2 | %3 | %4 bytes | %5")
                         .arg(row->rowId)
                         .arg(row->event.direction == ::ui::common::TrafficDirection::Tx ? QStringLiteral("TX") : QStringLiteral("RX"))
                         .arg(row->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")))
                         .arg(row->event.payload.size())
                         .arg(row->statusText);
    inspectorHeaderLabel_->setText(header);

    QString details;
    QTextStream out(&details);
    if (pr.protocol == ::modbus::parser::ProtocolType::Tcp) {
        out << tr("MBAP Header: TID=%1, PID=%2, Len=%3, UnitID=%4\n")
                   .arg(pr.transactionId)
                   .arg(pr.protocolId)
                   .arg(pr.length)
                   .arg(pr.slaveId);
    } else {
        out << tr("Serial ADU: Slave ID=%1\n").arg(pr.slaveId);
    }

    out << QString(tr("Function Code: 0x%1 (%2)\n")
               .arg(static_cast<int>(pr.functionCode), 2, 16, QChar('0')).toUpper()
               .arg(QString::fromLatin1(::modbus::base::function_traits::functionCodeName(pr.functionCode))));

    if (pr.isException) {
        out << QString(tr("Exception Code: 0x%1 (%2)\n")
                   .arg(static_cast<int>(pr.exceptionCode), 2, 16, QChar('0')).toUpper()
                   .arg(QString::fromLatin1(::modbus::base::function_traits::exceptionCodeName(pr.exceptionCode))));
    } else if (!pr.dataItems.isEmpty()) {
        out << tr("Decoded Items (%1):\n").arg(pr.dataItems.size());
        for (int i = 0; i < pr.dataItems.size(); ++i) {
            out << QStringLiteral("  +%-3d: %1\n").arg(i).arg(pr.dataItems[i].value.toString());
        }
    }
    inspectorDetailsText_->setText(details);
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
    } else if (index == 4) { // Errors
        proxyModel_->setFilterKeyColumn(TelemetryTableModel::ColStatus);
        proxyModel_->setFilterRegularExpression(QStringLiteral("Error|Exception|Malformed"));
    }
}

void ModbusTelemetryWidget::onExportClicked() {
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Export Telemetry"), QString(),
        tr("CSV Files (*.csv);;JSON Files (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    if (filePath.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
        QJsonArray arr;
        for (int i = 0; i < tableModel_->rowCount(); ++i) {
            const auto* r = tableModel_->rowAt(i);
            if (!r) continue;
            QJsonObject obj;
            obj[QStringLiteral("id")] = static_cast<qint64>(r->rowId);
            obj[QStringLiteral("timestamp")] = r->event.timestamp.toString(Qt::ISODateWithMs);
            obj[QStringLiteral("direction")] = r->event.direction == ::ui::common::TrafficDirection::Tx ? QStringLiteral("TX") : (r->event.direction == ::ui::common::TrafficDirection::Rx ? QStringLiteral("RX") : QStringLiteral("SYS"));
            obj[QStringLiteral("payloadHex")] = QString::fromLatin1(r->event.payload.toHex());
            obj[QStringLiteral("status")] = r->statusText;
            arr.append(obj);
        }
        file.write(QJsonDocument(arr).toJson());
    } else {
        QTextStream out(&file);
        out << "RowId,Timestamp,Direction,Length,PayloadHex,Status\n";
        for (int i = 0; i < tableModel_->rowCount(); ++i) {
            const auto* r = tableModel_->rowAt(i);
            if (!r) continue;
            out << r->rowId << ","
                << r->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")) << ","
                << (r->event.direction == ::ui::common::TrafficDirection::Tx ? "TX" : (r->event.direction == ::ui::common::TrafficDirection::Rx ? "RX" : "SYS")) << ","
                << r->event.payload.size() << ",\""
                << QString::fromLatin1(r->event.payload.toHex(' ').toUpper()) << "\",\""
                << r->statusText << "\"\n";
        }
    }
}

void ModbusTelemetryWidget::onCopySelectedRow() {
    const auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }
    const int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const TelemetryRowView* row = tableModel_->rowAt(sourceRow);
    if (!row) {
        return;
    }

    QString text = QStringLiteral("[%1] %2 | %3 | %4 bytes\n%5")
                       .arg(row->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")))
                       .arg(row->event.direction == ::ui::common::TrafficDirection::Tx ? QStringLiteral("TX") : QStringLiteral("RX"))
                       .arg(row->statusText)
                       .arg(row->event.payload.size())
                       .arg(QString::fromLatin1(row->event.payload.toHex(' ').toUpper()));
    QGuiApplication::clipboard()->setText(text);
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

} // namespace ui::widgets::modbus
