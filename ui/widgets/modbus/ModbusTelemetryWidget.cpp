// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusTelemetryWidget.h"
#include "common/FileExporter.h"
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
}

void ModbusTelemetryWidget::setupToolbar() {
    auto* toolLayout = new QHBoxLayout();
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->setSpacing(6);

    filterCombo_ = new QComboBox(this);
    filterCombo_->addItems({tr("All"), tr("TX"), tr("RX"), tr("SYS"), tr("Errors")});
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTelemetryWidget::onFilterChanged);
    filterLabel_ = new QLabel(tr("Filter:"), this);
    toolLayout->addWidget(filterLabel_);
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
    inspectorDetailsText_->setLineWrapMode(QTextEdit::NoWrap);
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
    if (batchTimer_ && batchTimer_->isActive()) {
        batchTimer_->stop();
    }
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

QString ModbusTelemetryWidget::inspectorDetailsText() const noexcept {
    return inspectorDetailsText_ ? inspectorDetailsText_->toPlainText() : QString();
}

QString ModbusTelemetryWidget::formatInspectorDetails(const ::modbus::parser::ParseResult& pr, int aduLength) {
    QString details;
    QTextStream out(&details);
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

    const auto dirStr = (row->event.direction == ::ui::common::TrafficDirection::Tx) ? QStringLiteral("TX") : QStringLiteral("RX");
    inspectorHeaderLabel_->setText(tr("[#%1] %2 | %3 | %4 bytes | %5")
                         .arg(row->rowId)
                         .arg(dirStr)
                         .arg(row->event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz")))
                         .arg(row->event.payload.size())
                         .arg(row->statusText));

    inspectorDetailsText_->setPlainText(formatInspectorDetails(row->parseResult, row->event.payload.size()));
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

void ModbusTelemetryWidget::retranslateUi() {
    if (filterLabel_) {
        filterLabel_->setText(tr("Filter:"));
    }
    if (filterCombo_) {
        const int cur = filterCombo_->currentIndex();
        filterCombo_->setItemText(0, tr("All"));
        filterCombo_->setItemText(1, tr("TX"));
        filterCombo_->setItemText(2, tr("RX"));
        filterCombo_->setItemText(3, tr("SYS"));
        filterCombo_->setItemText(4, tr("Errors"));
        filterCombo_->setCurrentIndex(cur);
    }
    if (rawFramesCheck_) {
        rawFramesCheck_->setText(tr("Raw Frames"));
    }
    if (autoScrollBtn_) {
        autoScrollBtn_->setText(autoScroll_ ? tr("Auto-Scroll: ON") : tr("Auto-Scroll: OFF"));
    }
    if (clearBtn_) {
        clearBtn_->setText(tr("Clear (Ctrl+L)"));
    }
    if (exportBtn_) {
        exportBtn_->setText(tr("Export..."));
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
    if (linkageCheck_) {
        linkageCheck_->setText(tr("Link to Frame Analyzer"));
    }
    updateMetricsLabel();
    if (tableModel_) {
        tableModel_->retranslate();
    }
}

} // namespace ui::widgets::modbus

