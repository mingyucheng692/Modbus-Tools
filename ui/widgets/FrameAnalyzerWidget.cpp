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
#include "widgets/FrameDecodedTableView.h"
#include "modbus/base/ModbusAddressMapping.h"
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
#include <QHeaderView>
#include <QTabWidget>
#include <QMessageBox>
#include <QApplication>
#include <QEvent>
#include <QSignalBlocker>
#include <QFileDialog>
#include <QListWidget>
#include <QResizeEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <QSplitter>

using namespace modbus::parser;
using namespace modbus::analyzer;

namespace ui::widgets {

namespace {

// UI-layer input preprocessing: strips bracketed metadata ([RX]/[TX]/timestamps),
// 0x prefixes and non-hex characters; returns a contiguous lowercase/uppercase
// hex string suitable for QByteArray::fromHex(), or a Latin-1 ASCII frame text
// (":...\r\n") for Modbus ASCII frames. Kept here so the worker stays
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

    clearBtn = new QPushButton(tr("Clear"), this);
    connect(clearBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearClicked);
    clearBtn->setMinimumWidth(86);
    actionsLayout->addWidget(clearBtn);

    parseBtn = new QPushButton(tr("Parse"), this);
    connect(parseBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onParseClicked);
    parseBtn->setMinimumWidth(86);
    parseBtn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    actionsLayout->addWidget(parseBtn);

    pasteAndParseBtn = new QPushButton(tr("Paste & Parse"), this);
    connect(pasteAndParseBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onPasteAndParseClicked);
    pasteAndParseBtn->setMinimumWidth(100);
    pasteAndParseBtn->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    pasteAndParseBtn->setDefault(true);
    actionsLayout->addWidget(pasteAndParseBtn);

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
    displayModeCombo->addItem(tr("UInt16 (Unsigned)"), static_cast<int>(RegisterDataType::UInt16));
    displayModeCombo->addItem(tr("Int16 (Signed)"), static_cast<int>(RegisterDataType::Int16));
    displayModeCombo->addItem(tr("Float32 (Real)"), static_cast<int>(RegisterDataType::Float32));
    displayModeCombo->addItem(tr("Int32 (DInt)"), static_cast<int>(RegisterDataType::Int32));
    displayModeCombo->addItem(tr("UInt32 (UDInt)"), static_cast<int>(RegisterDataType::UInt32));
    displayModeCombo->addItem(tr("Float64 (Double)"), static_cast<int>(RegisterDataType::Float64));
    displayModeCombo->setCurrentIndex(0);
    displayModeCombo->setMinimumContentsLength(10);
    displayModeCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    connect(displayModeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        globalDataType = static_cast<RegisterDataType>(displayModeCombo->currentData().toInt());
        displayMode = (globalDataType == RegisterDataType::Int16) ? NumberDisplayMode::Signed : NumberDisplayMode::Unsigned;
        if (!isLiveMode && !inputEditor->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        } else if (currentResult.isValid) {
            renderResult(currentResult);
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
        } else if (currentResult.isValid) {
            renderResult(currentResult);
        }
    });

    resultToolbarLayout->addWidget(displayModeLabel);
    resultToolbarLayout->addWidget(displayModeCombo);

    resetTypesBtn = new QPushButton(tr("Reset All"), this);
    resetTypesBtn->setMinimumHeight(28);
    resetTypesBtn->setEnabled(false);
    resetTypesBtn->setToolTip(tr("Reset all custom register types to Default"));
    connect(resetTypesBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onResetTypesClicked);
    resultToolbarLayout->addWidget(resetTypesBtn);

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

    dataTable = new FrameDecodedTableView(this);
    connect(dataTable, &FrameDecodedTableView::selectionOrCustomTypeStateChanged,
            this, [this](int count, bool hasCustom) {
        updateResetButtonState(count, hasCustom);
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

void FrameAnalyzerWidget::onPasteAndParseClicked()
{
    auto* clipboard = QGuiApplication::clipboard();
    if (!clipboard) return;

    const QString text = clipboard->text().trimmed();
    if (text.isEmpty()) {
        statusLabel->setText(tr("Clipboard is empty"));
        statusLabel->setStyleSheet(QStringLiteral("color: red;"));
        return;
    }

    inputEditor->setPlainText(text);
    onParseClicked();
}

void FrameAnalyzerWidget::loadAndParseHex(const QString& hex)
{
    exitLiveMode();
    if (inputEditor) {
        inputEditor->setPlainText(hex);
    }
    onParseClicked();
}

void FrameAnalyzerWidget::onClearClicked()
{
    ++latestParseRequestId;
    parseInProgress = false;
    if (parseBtn) parseBtn->setEnabled(true);
    if (pasteAndParseBtn) pasteAndParseBtn->setEnabled(true);
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
    if (pasteAndParseBtn) pasteAndParseBtn->setEnabled(false);

    // Pass the pre-normalized hex string so the worker does not need to
    // repeat input-format cleanup (see FrameParseWorker contract). The
    // presenter marshals the request onto the worker thread.
    presenter_->enqueueParse(hexStr, type, static_cast<uint16_t>(addrVal), modbus::base::RegisterOrder::ABCD, latestParseRequestId);
}

void FrameAnalyzerWidget::onParseFinished(const ParseResult& result, quint64 requestId)
{
    if (requestId != latestParseRequestId) return;

    parseInProgress = false;
    if (parseBtn) parseBtn->setEnabled(true);
    if (pasteAndParseBtn) pasteAndParseBtn->setEnabled(true);

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
    const auto meta = dataTable ? dataTable->metadataMap() : QMap<uint16_t, DataMetadata>{};
    bool ok = exporter::saveMetadataJson(filePath,
                                         startAddrEdit->text(),
                                         registerDataTypeToString(globalDataType),
                                         meta,
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
        auto importedType = stringToRegisterDataType(result.displayMode);
        if (importedType.has_value()) {
            int idx = displayModeCombo->findData(static_cast<int>(*importedType));
            if (idx >= 0) displayModeCombo->setCurrentIndex(idx);
        } else if (result.displayMode == QStringLiteral("signed")) {
            int idx = displayModeCombo->findData(static_cast<int>(RegisterDataType::Int16));
            if (idx >= 0) displayModeCombo->setCurrentIndex(idx);
        } else {
            int idx = displayModeCombo->findData(static_cast<int>(RegisterDataType::UInt16));
            if (idx >= 0) displayModeCombo->setCurrentIndex(idx);
        }
    }

    if (dataTable) {
        dataTable->setMetadataMap(result.metadata);
    }
    updateResetButtonState();
    if (!inputEditor->toPlainText().trimmed().isEmpty()) {
        onParseClicked();
    } else {
        clearResult();
    }
}

void FrameAnalyzerWidget::onExportCsvClicked()
{
    if (dataTable) {
        dataTable->exportCsv();
    }
}

void FrameAnalyzerWidget::updateResetButtonState(int selectedCount, bool hasCustom)
{
    if (!resetTypesBtn) return;

    if (selectedCount > 0) {
        resetTypesBtn->setText(tr("Reset Selected (%1)").arg(selectedCount));
        resetTypesBtn->setToolTip(tr("Reset selected %1 register type(s) to Default (Delete)").arg(selectedCount));
        resetTypesBtn->setEnabled(true);
    } else {
        resetTypesBtn->setText(tr("Reset All"));
        resetTypesBtn->setToolTip(tr("Reset all custom register types to Default"));
        resetTypesBtn->setEnabled(hasCustom);
    }
}

void FrameAnalyzerWidget::updateResetButtonState()
{
    if (!dataTable) {
        updateResetButtonState(0, false);
        return;
    }
    updateResetButtonState(dataTable->selectedRowCount(), dataTable->hasCustomTypes());
}

void FrameAnalyzerWidget::onResetTypesClicked()
{
    if (!dataTable) return;
    if (dataTable->selectedRowCount() > 0) {
        dataTable->resetSelectedToDefault();
    } else {
        dataTable->resetAllToDefault();
    }
}

void FrameAnalyzerWidget::renderResult(const ParseResult& result)
{
    if (overviewTree) {
        overviewTree->clear();
    }
    if (dataTable) {
        dataTable->clearTable();
    }

    if (!result.isValid) {
        updateResetButtonState();
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
        modbus::address::AddressBase addressBase = modbus::address::AddressBase::Offset0Based;
        if (settingsService_) {
            const QVariant val = settingsService_->value(core::common::settings_keys::kModbusAddressBase);
            if (val.isValid()) {
                addressBase = static_cast<modbus::address::AddressBase>(val.toInt());
            }
        }
        dataTable->renderData(result, globalDataType, registerOrder, addressBase);
    }

    updateResetButtonState();
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
    } else {
        if (dataTable) dataTable->clearTable();
        if (overviewTree) overviewTree->clear();
    }
    if (resultTabs) resultTabs->setTabText(0, tr("Structure"));
    updateResetButtonState();
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
    if (displayModeCombo && mode >= 0 && mode < displayModeCombo->count()) {
        displayModeCombo->setCurrentIndex(mode);
        globalDataType = static_cast<RegisterDataType>(displayModeCombo->currentData().toInt());
        displayMode = (globalDataType == RegisterDataType::Int16) ? NumberDisplayMode::Signed : NumberDisplayMode::Unsigned;
    }
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
        displayModeCombo->setItemText(0, tr("UInt16 (Unsigned)"));
        displayModeCombo->setItemText(1, tr("Int16 (Signed)"));
        displayModeCombo->setItemText(2, tr("Float32 (Real)"));
        displayModeCombo->setItemText(3, tr("Int32 (DInt)"));
        displayModeCombo->setItemText(4, tr("UInt32 (UDInt)"));
        displayModeCombo->setItemText(5, tr("Float64 (Double)"));
    }
    if (registerOrderLabel) registerOrderLabel->setText(tr("Byte Order:"));
    if (registerOrderCombo) {
        registerOrderCombo->setItemText(0, tr("ABCD(default)"));
        registerOrderCombo->setItemText(1, "BADC");
        registerOrderCombo->setItemText(2, "CDAB");
        registerOrderCombo->setItemText(3, "DCBA");
    }
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

    if (formatBtn) formatBtn->setText(tr("Format Hex"));
    if (clearBtn) clearBtn->setText(tr("Clear"));
    if (parseBtn) {
        parseBtn->setText(tr("Parse"));
        parseBtn->setToolTip(tr("Parse current input (Ctrl+Enter)"));
    }
    if (pasteAndParseBtn) {
        pasteAndParseBtn->setText(tr("Paste & Parse"));
        pasteAndParseBtn->setToolTip(tr("Paste clipboard content and parse immediately (Ctrl+Shift+V)"));
    }
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
        dataTable->retranslateUi();
    }
    if (clearHistoryBtn) clearHistoryBtn->setText(tr("Clear History"));

    updateResetButtonState();
    refreshHistoryList();
}

} // namespace ui::widgets
