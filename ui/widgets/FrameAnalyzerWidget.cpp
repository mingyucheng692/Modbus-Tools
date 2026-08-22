/**
 * @file FrameAnalyzerWidget.cpp
 * @brief Implementation of FrameAnalyzerWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameAnalyzerWidget.h"
#include "Config.h"
#include "infra/config/ISettingsService.h"
#include "common/SettingsKeys.h"
#include "common/ModbusDataHelper.h"
#include "modbus/base/ModbusProtocolChecks.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "application/analyzer/FrameAnalyzerPresenter.h"
#include "analyzer/AnalyzerCommon.h"
#include "analyzer/AnalyzerExporter.h"
#include "analyzer/ValueFormatter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QPushButton>
#include <QTreeWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QMessageBox>
#include <QApplication>
#include <QEvent>
#include <QSignalBlocker>
#include <QFileDialog>
#include <QListWidget>
#include <QResizeEvent>
#include <QSplitter>

using namespace modbus::parser;
using namespace modbus::analyzer;

namespace ui::widgets {

namespace {

// UI-layer input preprocessing: strips bracketed metadata ([RX]/[TX]/timestamps),
// 0x prefixes and non-hex characters; returns a contiguous lowercase/uppercase
// hex string suitable for QByteArray::fromHex(), or a Latin-1 ASCII frame text
// (":...\r\n") for Modbus ASCII frames. Kept here (P2-27) so the worker stays
// free of UI/input-format concerns.
QString normalizeHexInput(const QString& input)
{
    QString text = input;
    // Remove bracketed metadata segments such as "[12:39:35.668]" / "[RX]".
    text.remove(QRegularExpression(QStringLiteral("\\[[^\\]]*\\]")));

    // ASCII frame fast path: Modbus ASCII frames start with ':' and use
    // hex-encoded content with CRLF terminator. Must be preserved as Latin-1
    // text (not fromHex-decoded) for inspectAsciiAdu to work correctly.
    const QString trimmed = text.trimmed();
    if (trimmed.startsWith(':')) {
        const int crlfPos = trimmed.indexOf(QStringLiteral("\r\n"));
        QString hexBody;
        if (crlfPos > 0) {
            hexBody = trimmed.mid(1, crlfPos - 1);
        } else {
            hexBody = trimmed.mid(1);
        }
        hexBody.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
        if (hexBody.size() % 2 != 0) hexBody.chop(1);
        // Minimum valid frame: slaveId(1) + FC(1) + LRC(1) = 3 bytes = 6 hex chars
        if (hexBody.size() >= 6) {
            return QStringLiteral(":") + hexBody.toUpper() + QStringLiteral("\r\n");
        }
        // Insufficient hex chars — fall through to normal token parsing
    }

    const QStringList rawTokens = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    QString normalized;
    normalized.reserve(rawTokens.size() * 2);

    for (QString token : rawTokens) {
        token = token.trimmed();
        if (token.isEmpty()) continue;

        const QString upper = token.toUpper();
        if (upper == QStringLiteral("RX") || upper == QStringLiteral("TX") ||
            upper == QStringLiteral("FAIL") || upper == QStringLiteral("RTT")) {
            continue;
        }

        // Skip obvious timestamp/date-like tokens.
        if (token.contains(':') || token.contains('.') || token.contains('-')) continue;

        if (token.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
            token = token.mid(2);
        }

        token.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
        if (token.size() < 2) continue;
        if (token.size() % 2 != 0) token.chop(1);

        if (!token.isEmpty()) {
            normalized.append(token);
        }
    }

    // Fallback for plain contiguous hex input.
    if (normalized.isEmpty()) {
        QString plain = input;
        plain.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
        if (plain.size() % 2 != 0) plain.chop(1);
        normalized = plain;
    }
    return normalized;
}

} // namespace

// --- FrameAnalyzerWidget Implementation ---

FrameAnalyzerWidget::FrameAnalyzerWidget(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent)
{
    settingsService_ = settingsService;

    // Background parse thread/worker pair and its teardown are owned by the
    // presenter (via core::common::ThreadGuard); this widget only renders
    // results and forwards parse requests.
    presenter_ = new ui::application::analyzer::FrameAnalyzerPresenter(this);
    connect(presenter_, &ui::application::analyzer::FrameAnalyzerPresenter::parseFinished,
            this, &FrameAnalyzerWidget::onParseFinished);

    setupUi();
    loadSettings();
}

FrameAnalyzerWidget::~FrameAnalyzerWidget()
{
    // Deterministic teardown: destroy the presenter (and with it the parse
    // worker + thread via ThreadGuard) while this widget is still a live
    // QObject, severing worker->widget delivery paths first. No UAF window
    // for an in-flight parse.
    delete presenter_;
    presenter_ = nullptr;
}

void FrameAnalyzerWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    createInputGroup();
    createResultGroup();

    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setHandleWidth(6);
    mainSplitter->addWidget(inputGroup);
    mainSplitter->addWidget(resultGroup);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setSizes({config::Ui::kFrameAnalyzerDefaultInputHeight,
                            config::Ui::kFrameAnalyzerDefaultResultsHeight});
    mainLayout->addWidget(mainSplitter, 1);
    retranslateUi();
}

void FrameAnalyzerWidget::createInputGroup()
{
    inputGroup = new QGroupBox(tr("Frame Input"), this);
    auto groupLayout = new QVBoxLayout(inputGroup);

    // Controls Row
    auto controlsLayout = new QHBoxLayout();

    protocolLabel = new QLabel(tr("Protocol:"), this);
    controlsLayout->addWidget(protocolLabel);
    protocolCombo = new QComboBox(this);
    protocolCombo->addItem(tr("Auto Detect"), QVariant::fromValue(ProtocolType::Unknown));
    protocolCombo->addItem(tr("Modbus TCP"), QVariant::fromValue(ProtocolType::Tcp));
    protocolCombo->addItem(tr("Modbus RTU"), QVariant::fromValue(ProtocolType::Rtu));
    protocolCombo->addItem(tr("Modbus ASCII"), QVariant::fromValue(ProtocolType::Ascii));
    controlsLayout->addWidget(protocolCombo);

    controlsLayout->addSpacing(20);
    startAddrLabel = new QLabel(tr("Start Address (for Response):"), this);
    controlsLayout->addWidget(startAddrLabel);
    startAddrEdit = new QLineEdit(this);
    startAddrEdit->setFixedWidth(88);
    auto* hexValidator = new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-FxXHh]*"), this);
    startAddrEdit->setValidator(hexValidator);
    controlsLayout->addWidget(startAddrEdit);

    controlsLayout->addStretch();
    auto* actionsContainer = new QWidget(this);
    auto* actionsLayout = new QHBoxLayout(actionsContainer);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(6);

    formatBtn = new QPushButton(tr("Format Hex"), this);
    connect(formatBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onFormatClicked);
    formatBtn->setMinimumWidth(86);
    actionsLayout->addWidget(formatBtn);

    parseBtn = new QPushButton(tr("Parse"), this);
    connect(parseBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onParseClicked);
    parseBtn->setMinimumWidth(86);
    actionsLayout->addWidget(parseBtn);

    clearBtn = new QPushButton(tr("Clear"), this);
    connect(clearBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearClicked);
    clearBtn->setMinimumWidth(86);
    actionsLayout->addWidget(clearBtn);

    controlsLayout->addWidget(actionsContainer, 0, Qt::AlignRight);
    groupLayout->addLayout(controlsLayout);

    // Input Editor
    inputEditor = new QPlainTextEdit(this);
    inputEditor->setPlaceholderText(tr("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)"));
    inputEditor->setMinimumHeight(64);
    inputEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    groupLayout->addWidget(inputEditor);

    connect(startAddrEdit, &QLineEdit::textChanged, this, [this]() { saveSettings(); });
    inputGroup->setMinimumHeight(0);
}

void FrameAnalyzerWidget::createResultGroup()
{
    resultGroup = new QGroupBox(tr("Analysis Result"), this);
    auto groupLayout = new QVBoxLayout(resultGroup);

    auto resultToolbarLayout = new QHBoxLayout();
    resultToolbarLayout->setContentsMargins(0, 0, 0, 0);
    resultToolbarLayout->setSpacing(6);

    auto* statusContainer = new QWidget(this);
    auto* statusAreaLayout = new QVBoxLayout(statusContainer);
    statusAreaLayout->setContentsMargins(0, 0, 0, 0);
    statusAreaLayout->setSpacing(2);

    auto* statusLineLayout = new QHBoxLayout();
    statusLineLayout->setContentsMargins(0, 0, 0, 0);
    statusLineLayout->setSpacing(4);

    statusTitleLabel = new QLabel(tr("Status:"), this);
    statusTitleLabel->setStyleSheet("color: gray;");
    statusLineLayout->addWidget(statusTitleLabel);

    statusLabel = new QLabel(tr("Ready"), this);
    statusLabel->setStyleSheet("font-weight: bold; color: gray;");
    statusLineLayout->addWidget(statusLabel);
    statusLineLayout->addStretch();

    statusAreaLayout->addLayout(statusLineLayout);

    linkageTipLabel = new QLabel(this);
    linkageTipLabel->setStyleSheet("color: #10B981; font-size: 11px; font-weight: normal;");
    linkageTipLabel->setText(tr("Tip: \"Pause\" to edit description"));
    linkageTipLabel->setVisible(false);
    statusAreaLayout->addWidget(linkageTipLabel);

    resultToolbarLayout->addWidget(statusContainer);

    // Live Indicators (Left Aligned as per Original UI)
    auto* liveContainer = new QWidget(this);
    auto* liveLayout = new QHBoxLayout(liveContainer);
    liveLayout->setContentsMargins(0, 0, 0, 0);
    liveLayout->setSpacing(4);

    liveLabel = new QLabel(this);
    liveLabel->setStyleSheet("color: #10B981; font-weight: bold; padding: 2px 6px; border: 1px solid #10B981; border-radius: 4px;");
    liveLabel->setVisible(false);
    liveLayout->addWidget(liveLabel);

    linkagePauseBtn = new QPushButton(tr("Pause Refresh"), this);
    linkagePauseBtn->setMinimumHeight(28);
    linkagePauseBtn->setVisible(false);
    connect(linkagePauseBtn, &QPushButton::clicked, this, [this]() {
        emit linkagePauseToggled(!isLivePaused);
    });
    liveLayout->addWidget(linkagePauseBtn);

    linkageStopBtn = new QPushButton(tr("Stop Link"), this);
    linkageStopBtn->setStyleSheet("color: #EF4444; border: 1px solid #EF4444; background-color: white; font-weight: bold; padding: 0 10px; border-radius: 4px;");
    linkageStopBtn->setMinimumHeight(28);
    linkageStopBtn->setVisible(false);
    connect(linkageStopBtn, &QPushButton::clicked, this, [this]() {
        emit linkageStopRequested();
    });
    liveLayout->addWidget(linkageStopBtn);

    resultToolbarLayout->addWidget(liveContainer);
    resultToolbarLayout->addStretch();

    displayModeLabel = new QLabel(tr("Decode Mode:"), this);
    displayModeCombo = new QComboBox(this);
    displayModeCombo->addItem(tr("Unsigned"), static_cast<int>(NumberDisplayMode::Unsigned));
    displayModeCombo->addItem(tr("Signed"), static_cast<int>(NumberDisplayMode::Signed));
    displayModeCombo->setCurrentIndex(0);
    displayModeCombo->setMinimumContentsLength(8);
    displayModeCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    connect(displayModeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        displayMode = static_cast<NumberDisplayMode>(displayModeCombo->currentData().toInt());
        if (!isLiveMode && !inputEditor->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        } else if (isLiveMode && lastLiveResult.isValid) {
            renderResult(lastLiveResult);
        }
    });

    registerOrderLabel = new QLabel(tr("Byte Order:"), this);
    registerOrderCombo = new QComboBox(this);
    registerOrderCombo->addItem(tr("ABCD(default)"), static_cast<int>(modbus::base::RegisterOrder::ABCD));
    registerOrderCombo->addItem("BADC", static_cast<int>(modbus::base::RegisterOrder::BADC));
    registerOrderCombo->addItem("CDAB", static_cast<int>(modbus::base::RegisterOrder::CDAB));
    registerOrderCombo->addItem("DCBA", static_cast<int>(modbus::base::RegisterOrder::DCBA));
    registerOrderCombo->setMinimumWidth(80);
    connect(registerOrderCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        registerOrder = static_cast<modbus::base::RegisterOrder>(registerOrderCombo->itemData(index).toInt());
        if (!isLiveMode && !inputEditor->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        }
    });

    resultToolbarLayout->addWidget(displayModeLabel);
    resultToolbarLayout->addWidget(displayModeCombo);
    resultToolbarLayout->addSpacing(16);
    resultToolbarLayout->addWidget(registerOrderLabel);
    resultToolbarLayout->addWidget(registerOrderCombo);
    resultToolbarLayout->addSpacing(16);

    importJsonBtn = new QPushButton(tr("Import Config"), this);
    connect(importJsonBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onImportJsonClicked);
    exportJsonBtn = new QPushButton(tr("Export Config"), this);
    connect(exportJsonBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onExportJsonClicked);
    exportCsvBtn = new QPushButton(tr("Export CSV"), this);
    connect(exportCsvBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onExportCsvClicked);

    const QList<QPushButton*> actionButtons = { importJsonBtn, exportJsonBtn, exportCsvBtn };
    for (auto* button : actionButtons) {
        button->setMinimumWidth(0);
        button->setMinimumHeight(28);
        resultToolbarLayout->addWidget(button);
    }

    toggleHistoryBtn = new QPushButton(this);
    toggleHistoryBtn->setMinimumWidth(0);
    toggleHistoryBtn->setMinimumHeight(28);
    connect(toggleHistoryBtn, &QPushButton::clicked, this, [this]() { setHistoryCollapsed(!historyCollapsed); });
    updateHistoryToggleText();
    resultToolbarLayout->addWidget(toggleHistoryBtn);

    groupLayout->addLayout(resultToolbarLayout);

    resultTabs = new QTabWidget(this);
    structureTab = new QWidget(this);
    auto structureLayout = new QVBoxLayout(structureTab);
    structureLayout->setContentsMargins(0, 0, 0, 0);
    overviewTree = new QTreeWidget(structureTab);
    overviewTree->setHeaderLabels({tr("Field"), tr("Value"), tr("Description")});
    overviewTree->setColumnWidth(0, 200);
    overviewTree->setColumnWidth(1, 150);
    structureLayout->addWidget(overviewTree);
    resultTabs->addTab(structureTab, tr("Structure"));

    dataTable = new QTableWidget(this);
    dataTable->setColumnCount(7);
    dataTable->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Scale"), tr("Value"), tr("Description")});
    dataTable->horizontalHeader()->setStretchLastSection(true);
    connect(dataTable, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item) {
        if (isUpdatingDataTable || !item) return;
        const int col = item->column();
        if (col != 4 && col != 6) return;
        const uint16_t address = rowAddress(item->row());
        DataMetadata meta = metadataByAddress.value(address);
        if (col == 4) {
            bool ok = false;
            const double parsedScale = item->text().toDouble(&ok);
            if (!ok) {
                QSignalBlocker blocker(dataTable);
                item->setText(QString::number(meta.scale, 'g', 12));
                return;
            }
            meta.scale = parsedScale;
        } else if (col == 6) {
            meta.description = item->text();
        }
        metadataByAddress.insert(address, meta);
        QTableWidgetItem* decItem = dataTable->item(item->row(), 2);
        QVariant value;
        if (decItem && decItem->text() != "-") value = decItem->text().toDouble();
        applyMetadataToRow(item->row(), value, meta);
    });
    resultTabs->addTab(dataTable, tr("Decoded Data"));

    contentSplitter = new QSplitter(Qt::Horizontal, this);
    contentSplitter->addWidget(resultTabs);

    historyGroup = new QGroupBox(tr("History"), this);
    auto historyLayout = new QVBoxLayout(historyGroup);
    historyList = new QListWidget(this);
    historyLayout->addWidget(historyList);
    clearHistoryBtn = new QPushButton(tr("Clear History"), this);
    connect(clearHistoryBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearHistoryClicked);
    historyLayout->addWidget(clearHistoryBtn);
    connect(historyList, &QListWidget::currentRowChanged, this, &FrameAnalyzerWidget::onHistorySelectionChanged);

    contentSplitter->addWidget(historyGroup);
    contentSplitter->setStretchFactor(0, 1);
    contentSplitter->setStretchFactor(1, 0);
    contentSplitter->setSizes({800, config::Ui::kFrameAnalyzerDefaultHistoryWidth});
    groupLayout->addWidget(contentSplitter);

    updateAdaptiveLayout();
}

void FrameAnalyzerWidget::applyMetadataToRow(int row, const QVariant& value, const DataMetadata& meta)
{
    if (!dataTable || row < 0 || row >= dataTable->rowCount()) return;

    QTableWidgetItem* descItem = dataTable->item(row, 6);
    if (descItem) {
        descItem->setToolTip(value_formatter::buildDescriptionTooltip(value, meta, displayMode));
    }

    QTableWidgetItem* scaledItem = dataTable->item(row, 5);
    if (scaledItem) {
        scaledItem->setText(value_formatter::formatScaledValue(value, meta, displayMode));
    }

    QTableWidgetItem* scaleItem = dataTable->item(row, 4);
    if (scaleItem && scaleItem->text().trimmed().isEmpty()) {
        scaleItem->setText(QString::number(meta.scale, 'g', 12));
    }
}

uint16_t FrameAnalyzerWidget::rowAddress(int row) const
{
    if (!dataTable || row < 0 || row >= dataTable->rowCount()) return 0;
    const QTableWidgetItem* addrItem = dataTable->item(row, 0);
    if (!addrItem) return 0;
    const QVariant data = addrItem->data(Qt::UserRole);
    return data.isValid() ? static_cast<uint16_t>(data.toUInt()) : 0;
}

void FrameAnalyzerWidget::setHistoryCollapsed(bool collapsed)
{
    if (!historyGroup || !contentSplitter) return;

    historyCollapsed = collapsed;
    if (collapsed) {
        const QList<int> sizes = contentSplitter->sizes();
        if (sizes.size() > 1 && sizes.at(1) > 0) {
            lastHistoryPanelWidth = sizes.at(1);
        }
        historyGroup->hide();
    } else {
        historyGroup->show();
        QList<int> currentSizes = contentSplitter->sizes();
        int totalWidth = currentSizes.at(0) + currentSizes.at(1);

        // Fallback for first-time render where currentSizes might be {0, 0}
        if (totalWidth <= 0) {
            totalWidth = width();
        }
        if (totalWidth <= 200) {
            totalWidth = 1000; // Ensure a sane default if parent width is also unavailable
        }

        int hWidth = qMax(config::Ui::kFrameAnalyzerMinHistoryWidth, lastHistoryPanelWidth);
        contentSplitter->setSizes({qMax(0, totalWidth - hWidth), hWidth});
    }
    updateHistoryToggleText();
}

void FrameAnalyzerWidget::updateHistoryToggleText()
{
    if (!toggleHistoryBtn) return;
    toggleHistoryBtn->setText(historyCollapsed ? tr("Show History") : tr("Hide History"));
}

void FrameAnalyzerWidget::refreshHistoryList()
{
    if (!historyList) return;
    const QSignalBlocker blocker(historyList);
    historyList->clear();
    for (const modbus::parser::ParseResult& res : historyResults) {
        historyList->addItem(historyItemText(res));
    }
}

QString FrameAnalyzerWidget::historyItemText(const modbus::parser::ParseResult& result) const
{
    const QString status = result.isValid ? tr("OK") : tr("ERR");
    const QString type =
        result.protocol == ProtocolType::Tcp ? QStringLiteral("TCP") :
        result.protocol == ProtocolType::Rtu ? QStringLiteral("RTU") :
        result.protocol == ProtocolType::Ascii ? QStringLiteral("ASCII") :
        QStringLiteral("Unknown");
    return QString("[%1] %2 %3 - %4")
        .arg(tr("Local time %1").arg(result.timestamp.toLocalTime().toString("HH:mm:ss")))
        .arg(type)
        .arg(status)
        .arg(QString::fromLatin1(result.rawFrame.toHex().toUpper().left(16)) + "...");
}

void FrameAnalyzerWidget::addToHistory(const modbus::parser::ParseResult& result)
{
    historyResults.prepend(result);
    while (historyResults.size() > config::Ui::kFrameAnalyzerMaxHistoryItems) {
        historyResults.removeLast();
    }
    refreshHistoryList();
}

void FrameAnalyzerWidget::onFormatClicked()
{
    const QString text = normalizeHexInput(inputEditor->toPlainText());

    QString formatted;
    for (int i = 0; i < text.length(); i += 2) {
        formatted.append(text.mid(i, 2));
        if (i + 2 < text.length()) formatted.append(QLatin1Char(' '));
    }

    inputEditor->setPlainText(formatted.toUpper());
}

void FrameAnalyzerWidget::onClearClicked()
{
    ++latestParseRequestId;
    parseInProgress = false;
    if (parseBtn) parseBtn->setEnabled(true);
    inputEditor->clear();
    clearResult();
}

void FrameAnalyzerWidget::onParseClicked()
{
    clearResult();
    const QString rawInput = inputEditor->toPlainText();
    const QString hexStr = normalizeHexInput(rawInput);

    if (hexStr.isEmpty()) {
        statusLabel->setText(tr("Error: Empty input"));
        statusLabel->setStyleSheet(QStringLiteral("color: red;"));
        return;
    }

    ProtocolType type = protocolCombo->currentData().value<ProtocolType>();
    bool addrOk = false;
    int addrVal = ui::common::data_helper::parseSmartInt(startAddrEdit->text(), &addrOk);
    if (!addrOk || addrVal < 0 || addrVal > 65535) {
        statusLabel->setText(tr("Invalid Address (0-65535): %1").arg(startAddrEdit->text()));
        statusLabel->setStyleSheet(QStringLiteral("color: red;"));
        return;
    }

    ++latestParseRequestId;
    parseInProgress = true;
    statusLabel->setText(tr("Parsing..."));
    statusLabel->setStyleSheet(QStringLiteral("color: gray;"));
    if (parseBtn) parseBtn->setEnabled(false);

    // Pass the pre-normalized hex string so the worker does not need to
    // repeat input-format cleanup (see FrameParseWorker contract). The
    // presenter marshals the request onto the worker thread.
    presenter_->enqueueParse(hexStr, type, static_cast<uint16_t>(addrVal), registerOrder, latestParseRequestId);
}

void FrameAnalyzerWidget::onParseFinished(const ParseResult& result, quint64 requestId)
{
    if (requestId != latestParseRequestId) return;

    parseInProgress = false;
    if (parseBtn) parseBtn->setEnabled(true);

    currentResult = result;
    renderResult(result);
    if (result.isValid) {
        addToHistory(result);
    } else {
        statusLabel->setText(result.error.isEmpty() ? tr("Parse Failed") : result.error);
        statusLabel->setStyleSheet(QStringLiteral("color: red;"));
    }
}

void FrameAnalyzerWidget::onHistorySelectionChanged(int row)
{
    if (row < 0 || row >= historyResults.size()) return;
    currentResult = historyResults.at(row);
    renderResult(currentResult);
}

void FrameAnalyzerWidget::onClearHistoryClicked()
{
    historyResults.clear();
    historyList->clear();
}

void FrameAnalyzerWidget::onExportJsonClicked()
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export Config"), QString(), tr("JSON Files (*.json)"));
    if (filePath.isEmpty()) return;

    QString error;
    bool ok = exporter::saveMetadataJson(filePath,
                                         startAddrEdit->text(),
                                         (displayMode == NumberDisplayMode::Signed ? QStringLiteral("signed") : QStringLiteral("unsigned")),
                                         metadataByAddress,
                                         &error);
    if (!ok) {
        QMessageBox::warning(this, tr("Export Failed"), error);
    }
}

void FrameAnalyzerWidget::onImportJsonClicked()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import Config"), QString(), tr("JSON Files (*.json)"));
    if (filePath.isEmpty()) return;

    ImportResult result = exporter::loadMetadataJson(filePath);
    if (!result.success) {
        QMessageBox::warning(this, tr("Import Failed"), result.error);
        return;
    }

    if (startAddrEdit) startAddrEdit->setText(result.startAddress);
    if (displayModeCombo) {
        int idx = (result.displayMode == QStringLiteral("signed")) ? 1 : 0;
        displayModeCombo->setCurrentIndex(idx);
    }

    metadataByAddress = result.metadata;
    if (!inputEditor->toPlainText().trimmed().isEmpty()) {
        onParseClicked();
    } else {
        clearResult();
    }
}

void FrameAnalyzerWidget::onExportCsvClicked()
{
    if (!currentResult.isValid || !dataTable || dataTable->rowCount() == 0) {
        QMessageBox::information(this, tr("No Data"), tr("There is no data to export."));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export CSV"),
        QStringLiteral("analysis_%1.csv").arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_HHmmss'Z'"))),
        tr("CSV Files (*.csv)"));
    if (filePath.isEmpty()) return;

    QStringList lines;
    QStringList headers;
    for (int c = 0; c < dataTable->columnCount(); ++c) {
        headers << exporter::escapeCsvValue(dataTable->horizontalHeaderItem(c)->text());
    }
    lines << headers.join(QLatin1Char(','));

    for (int r = 0; r < dataTable->rowCount(); ++r) {
        QStringList row;
        for (int c = 0; c < dataTable->columnCount(); ++c) {
            row << exporter::escapeCsvValue(dataTable->item(r, c)->text());
        }
        lines << row.join(QLatin1Char(','));
    }

    QString error;
    if (!exporter::writeCsvChunk(filePath, lines, true, &error)) {
        QMessageBox::warning(this, tr("Export Failed"), error);
    }
}

void FrameAnalyzerWidget::renderResult(const ParseResult& result)
{
    isUpdatingDataTable = true;

    if (overviewTree) {
        overviewTree->clear();
    }
    if (dataTable) {
        dataTable->setRowCount(0);
    }

    if (!result.isValid) {
        isUpdatingDataTable = false;
        if (!isLiveMode) {
            statusLabel->setText(tr("Parse Failed: %1").arg(result.error));
            statusLabel->setStyleSheet(QStringLiteral("color: red; font-weight: bold;"));
        }
        return;
    }

    const QString protocolText =
        result.protocol == ProtocolType::Tcp ? tr("TCP") :
        result.protocol == ProtocolType::Rtu ? tr("RTU") :
        result.protocol == ProtocolType::Ascii ? tr("ASCII") :
        tr("Unknown");
    const QString typeText =
        result.type == FrameType::Request ? tr("Request") :
        result.type == FrameType::Response ? tr("Response") :
        result.type == FrameType::Exception ? tr("Exception") :
        tr("Unknown");

    auto byteHex = [](const QByteArray& data) {
        return data.isEmpty() ? QStringLiteral("(empty)") : QString::fromLatin1(data.toHex(' ').toUpper());
    };

    modbus::base::AsciiAduFields asciiFields;
    const bool hasAsciiFields =
        result.protocol == ProtocolType::Ascii
        && modbus::base::inspectAsciiAdu(result.rawFrame, &asciiFields) == result.rawFrame.size();

    auto buildDescription = [](std::initializer_list<QString> parts) {
        QStringList uniqueParts;
        for (const QString& part : parts) {
            const QString trimmed = part.trimmed();
            if (trimmed.isEmpty() || uniqueParts.contains(trimmed)) {
                continue;
            }
            uniqueParts << trimmed;
        }
        return uniqueParts.join(QStringLiteral(" | "));
    };

    // Helper for adding tree items
    auto addTreeItem = [](QTreeWidgetItem* parent, const QString& field, const QString& value, const QString& desc = QString()) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, field);
        item->setText(1, value);
        item->setText(2, desc);
        return item;
    };

    if (overviewTree) {
        overviewTree->clear();
        if (isLiveMode) {
            auto* root = new QTreeWidgetItem(overviewTree);
            root->setText(0, tr("Structure"));
            root->setText(1, tr("(Unavailable in Live Mode)"));
            root->setText(2, tr("Logical parsing is disabled for high-frequency linkage"));
            root->setExpanded(true);
        } else {
            auto* root = new QTreeWidgetItem(overviewTree);
            root->setText(0, tr("Frame"));
            root->setText(1, tr("%1 bytes").arg(result.rawFrame.size()));
            root->setText(2, buildDescription({protocolText, typeText}));

            addTreeItem(root, tr("Frame Bytes"), byteHex(result.rawFrame), tr("Complete raw frame"));

            if (result.protocol == ProtocolType::Tcp) {
                auto* mbap = addTreeItem(
                    root,
                    tr("MBAP Header"),
                    byteHex(result.rawFrame.left(7)),
                    tr("Transaction + Protocol + Length + Unit ID"));
                addTreeItem(
                    mbap,
                    tr("Transaction ID"),
                    QString("%1 (%2)").arg(result.transactionId).arg(byteHex(result.rawFrame.mid(0, 2))),
                    tr("Request/response correlation ID"));
                addTreeItem(
                    mbap,
                    tr("Protocol ID"),
                    QString("%1 (%2)").arg(result.protocolId).arg(byteHex(result.rawFrame.mid(2, 2))),
                    tr("Modbus TCP protocol identifier"));
                addTreeItem(
                    mbap,
                    tr("Length"),
                    QString("%1 (%2)").arg(result.length).arg(byteHex(result.rawFrame.mid(4, 2))),
                    tr("Remaining bytes after this field"));
                addTreeItem(
                    mbap,
                    tr("Unit ID"),
                    QString("%1 (%2)").arg(result.slaveId).arg(byteHex(result.rawFrame.mid(6, 1))),
                    tr("Target slave / unit address"));
            } else if (result.protocol == ProtocolType::Ascii) {
                addTreeItem(
                    root,
                    tr("Slave ID"),
                    QString("%1 (%2)").arg(result.slaveId).arg(byteHex(asciiFields.binaryAdu.mid(0, 1))),
                    tr("Target slave address"));
                addTreeItem(
                    root,
                    tr("ASCII Start"),
                    byteHex(result.rawFrame.left(1)),
                    tr("Start delimiter ':'"));
                addTreeItem(
                    root,
                    tr("ASCII Payload"),
                    QString::fromLatin1(result.rawFrame.mid(1, result.rawFrame.size() - 3)),
                    tr("ASCII hex payload before CRLF"));
            } else {
                addTreeItem(
                    root,
                    tr("Slave ID"),
                    QString("%1 (%2)").arg(result.slaveId).arg(byteHex(result.rawFrame.mid(0, 1))),
                    tr("Target slave address"));
            }

            const bool isTcp = result.protocol == ProtocolType::Tcp;
            const QByteArray pduBytes =
                isTcp ? result.rawFrame.mid(7, qMax(0, result.rawFrame.size() - 7)) :
                result.protocol == ProtocolType::Ascii && hasAsciiFields
                    ? asciiFields.binaryAdu.mid(1, qMax(0, asciiFields.binaryAdu.size() - 2))
                    : result.rawFrame.mid(1, qMax(0, result.rawFrame.size() - 3));
            const QByteArray payloadBytes =
                pduBytes.size() > 1 ? pduBytes.mid(1) : QByteArray();
            const QByteArray functionCodeBytes =
                isTcp ? result.rawFrame.mid(7, 1) :
                result.protocol == ProtocolType::Ascii && hasAsciiFields
                    ? asciiFields.binaryAdu.mid(1, 1)
                    : result.rawFrame.mid(1, 1);

            auto* pdu = addTreeItem(
                root,
                tr("PDU"),
                byteHex(pduBytes),
                tr("Function code + payload"));
            const QString fcHex = QStringLiteral("0x%1").arg(static_cast<int>(result.functionCode), 2, 16, QChar('0')).toUpper();
            addTreeItem(
                pdu,
                tr("Function Code"),
                    QString("%1 (%2)").arg(fcHex).arg(byteHex(functionCodeBytes)),
                buildDescription({typeText, result.isException ? tr("Exception response") : tr("Normal response")}));
            addTreeItem(
                pdu,
                tr("Payload"),
                byteHex(payloadBytes),
                result.isException ? tr("Exception detail payload") : tr("Application data payload"));

            if (result.isException) {
                addTreeItem(
                    pdu,
                    tr("Exception Code"),
                    QStringLiteral("0x%1").arg(static_cast<int>(result.exceptionCode), 2, 16, QChar('0')).toUpper(),
                    buildDescription({result.error, tr("Exception detail payload")}));
            }

            if (result.protocol == ProtocolType::Rtu) {
                QString crcDescription = result.checksumValid ? tr("CRC valid") : tr("CRC invalid");
                if (!result.checksumValid) {
                    crcDescription = buildDescription({
                        crcDescription,
                        tr("Expected 0x%1").arg(QString::number(result.calculatedChecksum, 16).toUpper())
                    });
                }
                addTreeItem(
                    root,
                    tr("CRC16"),
                    QString("%1 (%2)")
                        .arg(QStringLiteral("0x%1").arg(result.checksum, 4, 16, QChar('0')).toUpper())
                        .arg(byteHex(result.rawFrame.right(2))),
                    crcDescription);
            }
            if (result.protocol == ProtocolType::Ascii) {
                addTreeItem(
                    root,
                    tr("LRC"),
                    QString("%1 (%2)")
                        .arg(QStringLiteral("0x%1").arg(result.checksum, 2, 16, QChar('0')).toUpper())
                        .arg(hasAsciiFields ? byteHex(asciiFields.binaryAdu.right(1)) : QStringLiteral("(n/a)")),
                    result.checksumValid ? tr("LRC valid") : tr("LRC invalid"));
                addTreeItem(
                    root,
                    tr("CRLF"),
                    byteHex(result.rawFrame.right(2)),
                    tr("ASCII frame terminator"));
            }

            overviewTree->expandAll();
        }
    }

    if (dataTable) {
        dataTable->setRowCount(result.dataItems.size());
        for (int i = 0; i < result.dataItems.size(); ++i) {
            const auto& item = result.dataItems[i];
            DataMetadata meta = metadataByAddress.value(item.address);

            auto* addrItem = new QTableWidgetItem(QStringLiteral("%1 (0x%2)")
                .arg(item.address)
                .arg(QString::number(item.address, 16).toUpper().rightJustified(4, QLatin1Char('0'))));
            addrItem->setData(Qt::UserRole, item.address);
            addrItem->setFlags(addrItem->flags() & ~Qt::ItemIsEditable);
            dataTable->setItem(i, 0, addrItem);

            auto* hexItem = new QTableWidgetItem(value_formatter::formatHexValue(item.rawBytes, item.hexString));
            hexItem->setFlags(addrItem->flags());
            dataTable->setItem(i, 1, hexItem);

            auto* decItem = new QTableWidgetItem(value_formatter::formatDecimalValue(item.value, displayMode));
            decItem->setFlags(addrItem->flags());
            dataTable->setItem(i, 2, decItem);

            auto* binItem = new QTableWidgetItem(value_formatter::formatBinaryValue(item.rawBytes, item.binaryString));
            binItem->setFlags(addrItem->flags());
            dataTable->setItem(i, 3, binItem);

            dataTable->setItem(i, 4, new QTableWidgetItem(QString::number(meta.scale, 'g', 12)));

            auto* scaledItem = new QTableWidgetItem(value_formatter::formatScaledValue(item.value, meta, displayMode));
            scaledItem->setFlags(addrItem->flags());
            dataTable->setItem(i, 5, scaledItem);

            dataTable->setItem(i, 6, new QTableWidgetItem(meta.description));

            applyMetadataToRow(i, item.value, meta);
        }
    }

    isUpdatingDataTable = false;
    if (!isLiveMode) {
        QString statusText = tr("Success (%1)").arg(protocolText);
        if (result.isForced) {
            statusText += QStringLiteral(" [") + tr("Forced Parsing") + QStringLiteral("]");
        }
        if (!result.warnings.isEmpty()) {
            statusText += QStringLiteral(" (") + tr("Warnings") + QStringLiteral(")");
        }
        statusLabel->setText(statusText);
        statusLabel->setStyleSheet(result.warnings.isEmpty() ?
            QStringLiteral("color: #10B981; font-weight: bold;") :
            QStringLiteral("color: #F59E0B; font-weight: bold;"));
    }
}

void FrameAnalyzerWidget::clearResult()
{
    statusLabel->setText(tr("Ready"));
    statusLabel->setStyleSheet(QStringLiteral("color: gray;"));

    const bool wasLive = isLiveMode;
    isLiveMode = false;
    isLivePaused = false;

    liveLabel->setVisible(false);
    linkageStopBtn->setVisible(false);
    if (linkageTipLabel) linkageTipLabel->setVisible(false);
    if (linkagePauseBtn) {
        linkagePauseBtn->setVisible(false);
        linkagePauseBtn->setText(tr("Pause Refresh"));
        linkagePauseBtn->setStyleSheet(QString());
    }

    if (registerOrderCombo) registerOrderCombo->setEnabled(true);

    if (wasLive && lastLiveResult.isValid) {
        renderResult(lastLiveResult);
    }
    if (resultTabs) resultTabs->setTabText(0, tr("Structure"));
}

void FrameAnalyzerWidget::processLivePdu(const modbus::base::Pdu& pdu, modbus::parser::ProtocolType protocol, uint16_t addr)
{
    isLiveMode = true;
    if (resultTabs) resultTabs->setTabText(0, tr("Structure (Unavailable in Live Mode)"));
    if (parseInProgress) return;

    ParseResult result;
    result.isValid = true;
    result.protocol = protocol;
    result.timestamp = QDateTime::currentDateTimeUtc();
    result.functionCode = pdu.functionCode();
    result.isException = pdu.isException();
    result.pduData = pdu.toByteArray();
    result.type = result.isException ? FrameType::Exception : FrameType::Response;

    modbus::parser::parsePdu(result, result.pduData, addr, 0);

    QString protocolStr =
        protocol == ProtocolType::Tcp ? QStringLiteral("TCP") :
        protocol == ProtocolType::Rtu ? QStringLiteral("RTU") :
        protocol == ProtocolType::Ascii ? QStringLiteral("ASCII") :
        QStringLiteral("Unknown");
    if (liveLabel) {
        liveLabel->setText(tr("LIVE: %1").arg(protocolStr));
        liveLabel->setVisible(true);
    }
    if (statusLabel) {
        statusLabel->setText(tr("Live Data Received at %1").arg(result.timestamp.toLocalTime().toString("HH:mm:ss.zzz")));
        statusLabel->setStyleSheet(QStringLiteral("color: #10B981; font-weight: bold;"));
    }
    if (linkageTipLabel) linkageTipLabel->setVisible(true);
    if (linkagePauseBtn) linkagePauseBtn->setVisible(true);
    if (linkageStopBtn) linkageStopBtn->setVisible(true);

    if (registerOrderCombo) registerOrderCombo->setEnabled(false); // 联动模式通常由会话层控制字节序

    lastLiveResult = result;
    if (!isLivePaused) renderResult(result);
}

void FrameAnalyzerWidget::exitLiveMode()
{
    clearResult();
}

void FrameAnalyzerWidget::setLivePaused(bool paused)
{
    isLivePaused = paused;
    if (linkagePauseBtn) {
        linkagePauseBtn->setText(paused ? tr("Resume Refresh") : tr("Pause Refresh"));
        if (paused) {
            linkagePauseBtn->setStyleSheet("background-color: #F59E0B; color: white; border: 1px solid #D97706; font-weight: bold; border-radius: 4px;");
        } else {
            linkagePauseBtn->setStyleSheet(QString());
        }
    }
}

void FrameAnalyzerWidget::loadSettings()
{
    if (!settingsService_) return;

    QSignalBlocker blocker(startAddrEdit);
    const QString startAddr = settingsService_->value(core::common::settings_keys::kFrameAnalyzerStartAddr).toString();
    if (!startAddr.isEmpty()) {
        startAddrEdit->setText(startAddr);
    } else {
        startAddrEdit->setText(QString::number(config::Modbus::kDefaultStandardStartAddress));
    }

    const int mode = settingsService_->value(core::common::settings_keys::kFrameAnalyzerDecodeMode).toInt();
    if (displayModeCombo) displayModeCombo->setCurrentIndex(mode);
}

void FrameAnalyzerWidget::saveSettings()
{
    if (!settingsService_) return;
    settingsService_->setValue(core::common::settings_keys::kFrameAnalyzerStartAddr, startAddrEdit->text());
    settingsService_->setValue(core::common::settings_keys::kFrameAnalyzerDecodeMode, displayModeCombo->currentIndex());
}

void FrameAnalyzerWidget::updateAdaptiveLayout()
{
    if (!contentSplitter || !toggleHistoryBtn) return;
    const bool shouldCollapse = width() < 1000;
    if (shouldCollapse != historyAutoCollapsed) {
        historyAutoCollapsed = shouldCollapse;
        setHistoryCollapsed(shouldCollapse);
    }
}

void FrameAnalyzerWidget::resizeEvent(QResizeEvent* event)
{
    updateAdaptiveLayout();
    QWidget::resizeEvent(event);
}

void FrameAnalyzerWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void FrameAnalyzerWidget::retranslateUi()
{
    if (inputGroup) inputGroup->setTitle(tr("Frame Input"));
    if (protocolLabel) protocolLabel->setText(tr("Protocol:"));
    if (protocolCombo) {
        protocolCombo->setItemText(0, tr("Auto Detect"));
        protocolCombo->setItemText(1, tr("Modbus TCP"));
        protocolCombo->setItemText(2, tr("Modbus RTU"));
        protocolCombo->setItemText(3, tr("Modbus ASCII"));
    }
    if (startAddrLabel) startAddrLabel->setText(tr("Start Address (for Response):"));
    if (startAddrEdit) {
        startAddrEdit->setToolTip(tr("Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16)."));
    }
    if (displayModeLabel) displayModeLabel->setText(tr("Decode Mode:"));
    if (displayModeCombo) {
        displayModeCombo->setItemText(0, tr("Unsigned"));
        displayModeCombo->setItemText(1, tr("Signed"));
    }
    if (registerOrderLabel) registerOrderLabel->setText(tr("Byte Order:"));
    if (registerOrderCombo) {
        registerOrderCombo->setItemText(0, tr("ABCD(default)"));
        registerOrderCombo->setItemText(1, "BADC");
        registerOrderCombo->setItemText(2, "CDAB");
        registerOrderCombo->setItemText(3, "DCBA");
    }
    if (formatBtn) formatBtn->setText(tr("Format Hex"));
    if (importJsonBtn) importJsonBtn->setText(tr("Import Config"));
    if (exportJsonBtn) exportJsonBtn->setText(tr("Export Config"));
    if (exportCsvBtn) exportCsvBtn->setText(tr("Export CSV"));

    updateHistoryToggleText();

    if (linkageStopBtn) linkageStopBtn->setText(tr("Stop Link"));
    if (linkagePauseBtn) {
        linkagePauseBtn->setText(isLivePaused ? tr("Resume Refresh") : tr("Pause Refresh"));
    }
    if (statusTitleLabel) statusTitleLabel->setText(tr("Status:"));
    if (isLiveMode) {
        QString protocolStr =
            lastLiveResult.protocol == ProtocolType::Tcp ? tr("TCP") :
            lastLiveResult.protocol == ProtocolType::Rtu ? tr("RTU") :
            lastLiveResult.protocol == ProtocolType::Ascii ? tr("ASCII") :
            tr("Unknown");
        if (liveLabel) liveLabel->setText(tr("LIVE: %1").arg(protocolStr));
        if (statusLabel && lastLiveResult.isValid) {
            statusLabel->setText(tr("Live Data Received at %1").arg(lastLiveResult.timestamp.toLocalTime().toString("HH:mm:ss.zzz")));
        }
    } else {
        if (liveLabel) liveLabel->setText(QString());
        if (statusLabel) statusLabel->setText(tr("Ready"));
    }

    if (parseBtn) parseBtn->setText(tr("Parse"));
    if (clearBtn) clearBtn->setText(tr("Clear"));
    if (inputEditor) {
        inputEditor->setPlaceholderText(
            tr("Enter Hex string (e.g., RTU: 01 03 00 00 00 01 84 0A, ASCII bytes: 3A 30 31 30 33 ... 0D 0A)"));
    }

    if (resultGroup) resultGroup->setTitle(tr("Analysis Result"));
    if (historyGroup) historyGroup->setTitle(tr("History"));
    if (resultTabs) {
        resultTabs->setTabText(0, isLiveMode ? tr("Structure (Unavailable in Live Mode)") : tr("Structure"));
        resultTabs->setTabText(1, tr("Decoded Data"));
    }
    if (overviewTree) {
        QTreeWidgetItem* header = overviewTree->headerItem();
        header->setText(0, tr("Field"));
        header->setText(1, tr("Value"));
        header->setText(2, tr("Description"));
    }
    if (dataTable) {
        dataTable->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Scale"), tr("Value"), tr("Description")});
    }
    if (clearHistoryBtn) clearHistoryBtn->setText(tr("Clear History"));

    refreshHistoryList();
}

} // namespace ui::widgets
