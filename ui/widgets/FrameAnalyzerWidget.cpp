/**
 * @file FrameAnalyzerWidget.cpp
 * @brief Implementation of FrameAnalyzerWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameAnalyzerWidget.h"
#include "AppConstants.h"
#include "../common/ISettingsService.h"
#include "../common/SettingsKeys.h"
#include "modbus/base/ModbusDataHelper.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "modbus/parser/FrameParseWorker.h"
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
#include <QFutureWatcher>
#include <QSplitter>
#include <QThread>
#include <QPointer>

using namespace modbus::core::parser;
using namespace modbus::analyzer;

namespace ui::widgets {

class FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate {
public:
    explicit FrameAnalyzerWidgetPrivate(FrameAnalyzerWidget* q)
        : q_ptr(q)
    {}

    // --- Helpers ---
    [[nodiscard]] uint16_t rowAddress(int row) const;
    [[nodiscard]] QString historyItemText(const modbus::core::parser::ParseResult& result) const;
    
    void applyMetadataToRow(int row, const QVariant& value, const DataMetadata& meta);
    void addToHistory(const modbus::core::parser::ParseResult& result);
    void refreshHistoryList();
    void setHistoryCollapsed(bool collapsed);
    void updateHistoryToggleText();
    void updateAdaptiveLayout();
    void loadSettings();
    void saveSettings();

    // --- UI Logic ---
    void createInputGroup();
    void createResultGroup();

    // --- Members ---
    FrameAnalyzerWidget* q_ptr;

    // Input Controls
    QGroupBox* inputGroup = nullptr;
    QSplitter* mainSplitter = nullptr;
    QLabel* protocolLabel = nullptr;
    QLabel* startAddrLabel = nullptr;
    QLabel* displayModeLabel = nullptr;
    QPlainTextEdit* inputEditor = nullptr;
    QComboBox* protocolCombo = nullptr;
    QComboBox* displayModeCombo = nullptr;
    QComboBox* registerOrderCombo = nullptr;
    QPushButton* parseBtn = nullptr;
    QPushButton* formatBtn = nullptr;
    QPushButton* importJsonBtn = nullptr;
    QPushButton* exportJsonBtn = nullptr;
    QPushButton* exportCsvBtn = nullptr;
    QPushButton* toggleHistoryBtn = nullptr;
    QPushButton* clearBtn = nullptr;
    QLabel* registerOrderLabel = nullptr;
    QLineEdit* startAddrEdit = nullptr;

    // Result Controls
    QGroupBox* resultGroup = nullptr;
    QLabel* statusTitleLabel = nullptr;
    QLabel* statusLabel = nullptr;
    QWidget* structureTab = nullptr;
    QTreeWidget* overviewTree = nullptr;
    QTableWidget* dataTable = nullptr;
    QTabWidget* resultTabs = nullptr;
    QSplitter* contentSplitter = nullptr;
    QGroupBox* historyGroup = nullptr;
    QListWidget* historyList = nullptr;
    QPushButton* clearHistoryBtn = nullptr;

    // State
    bool historyCollapsed = false;
    bool historyAutoCollapsed = false;
    int lastHistoryPanelWidth = app::constants::Values::Ui::kFrameAnalyzerDefaultHistoryWidth;
    NumberDisplayMode displayMode = NumberDisplayMode::Unsigned;
    QMap<uint16_t, DataMetadata> metadataByAddress;
    QList<modbus::core::parser::ParseResult> historyResults;
    modbus::core::parser::ParseResult currentResult;
    
    // Threading
    QThread* parseThread = nullptr;
    QPointer<FrameParseWorker> parseWorker;
    quint64 latestParseRequestId = 0;
    bool parseInProgress = false;
    bool isUpdatingDataTable = false;

    // Live Link State
    bool isLiveMode = false;
    bool isLivePaused = false;
    modbus::core::parser::ParseResult lastLiveResult;
    modbus::base::RegisterOrder registerOrder = modbus::base::RegisterOrder::ABCD;

    // Service
    ui::common::ISettingsService* settingsService = nullptr;
    
    // Live Link UI
    QLabel* liveLabel = nullptr;
    QLabel* linkageTipLabel = nullptr;
    QPushButton* linkagePauseBtn = nullptr;
    QPushButton* linkageStopBtn = nullptr;
};

// --- FrameAnalyzerWidget Implementation ---

/// --- FrameAnalyzerWidgetPrivate Implementations ---

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::applyMetadataToRow(int row, const QVariant& value, const DataMetadata& meta)
{
    if (!dataTable || row < 0 || row >= dataTable->rowCount()) return;

    QTableWidgetItem* descItem = dataTable->item(row, 6);
    if (descItem) {
        descItem->setToolTip(ValueFormatter::buildDescriptionTooltip(value, meta, displayMode));
    }
    
    QTableWidgetItem* scaledItem = dataTable->item(row, 5);
    if (scaledItem) {
        scaledItem->setText(ValueFormatter::formatScaledValue(value, meta, displayMode));
    }
    
    QTableWidgetItem* scaleItem = dataTable->item(row, 4);
    if (scaleItem && scaleItem->text().trimmed().isEmpty()) {
        scaleItem->setText(QString::number(meta.scale, 'g', 12));
    }
}

uint16_t FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::rowAddress(int row) const
{
    if (!dataTable || row < 0 || row >= dataTable->rowCount()) return 0;
    const QTableWidgetItem* addrItem = dataTable->item(row, 0);
    if (!addrItem) return 0;
    const QVariant data = addrItem->data(Qt::UserRole);
    return data.isValid() ? static_cast<uint16_t>(data.toUInt()) : 0;
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::setHistoryCollapsed(bool collapsed)
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
            totalWidth = q_ptr->width();
        }
        if (totalWidth <= 200) {
            totalWidth = 1000; // Ensure a sane default if parent width is also unavailable
        }

        int hWidth = qMax(app::constants::Values::Ui::kFrameAnalyzerMinHistoryWidth, lastHistoryPanelWidth);
        contentSplitter->setSizes({qMax(0, totalWidth - hWidth), hWidth});
    }
    updateHistoryToggleText();
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::updateHistoryToggleText()
{
    if (!toggleHistoryBtn) return;
    toggleHistoryBtn->setText(historyCollapsed ? tr("Show History") : tr("Hide History"));
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::refreshHistoryList()
{
    if (!historyList) return;
    const QSignalBlocker blocker(historyList);
    historyList->clear();
    for (const modbus::core::parser::ParseResult& res : historyResults) {
        historyList->addItem(historyItemText(res));
    }
}

QString FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::historyItemText(const modbus::core::parser::ParseResult& result) const
{
    const QString status = result.isValid ? tr("OK") : tr("ERR");
    const QString type = (result.protocol == ProtocolType::Tcp) ? "TCP" : "RTU";
    return QString("[%1] %2 %3 - %4")
        .arg(result.timestamp.toString("HH:mm:ss"))
        .arg(type)
        .arg(status)
        .arg(QString::fromLatin1(result.rawFrame.toHex().toUpper().left(16)) + "...");
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::addToHistory(const modbus::core::parser::ParseResult& result)
{
    historyResults.prepend(result);
    while (historyResults.size() > app::constants::Values::Ui::kFrameAnalyzerMaxHistoryItems) {
        historyResults.removeLast();
    }
    refreshHistoryList();
}

// --- FrameAnalyzerWidget Implementation ---

FrameAnalyzerWidget::FrameAnalyzerWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new FrameAnalyzerWidgetPrivate(this))
{
    Q_D(FrameAnalyzerWidget);
    d->settingsService = settingsService;

    qRegisterMetaType<modbus::core::parser::ProtocolType>();
    qRegisterMetaType<modbus::core::parser::ParseResult>();
    qRegisterMetaType<modbus::base::RegisterOrder>();

    d->parseThread = new QThread(this);
    d->parseWorker = new FrameParseWorker();
    d->parseWorker->moveToThread(d->parseThread);
    
    connect(d->parseWorker.data(), &FrameParseWorker::parseFinished, this, &FrameAnalyzerWidget::onParseFinished);
    
    // Lifecycle management according to CodingRole.md
    connect(d->parseThread, &QThread::finished, d->parseWorker.data(), &QObject::deleteLater);
    
    d->parseThread->start();

    setupUi();
    d->loadSettings();
}

FrameAnalyzerWidget::~FrameAnalyzerWidget()
{
    Q_D(FrameAnalyzerWidget);
    if (d->parseThread) {
        d->parseThread->quit();
        d->parseThread->wait();
    }
}

void FrameAnalyzerWidget::setupUi()
{
    Q_D(FrameAnalyzerWidget);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    d->createInputGroup();
    d->createResultGroup();

    d->mainSplitter = new QSplitter(Qt::Vertical, this);
    d->mainSplitter->setChildrenCollapsible(false);
    d->mainSplitter->setHandleWidth(6);
    d->mainSplitter->addWidget(d->inputGroup);
    d->mainSplitter->addWidget(d->resultGroup);
    d->mainSplitter->setStretchFactor(0, 0);
    d->mainSplitter->setStretchFactor(1, 1);
    d->mainSplitter->setSizes({app::constants::Values::Ui::kFrameAnalyzerDefaultInputHeight, 
                               app::constants::Values::Ui::kFrameAnalyzerDefaultResultsHeight});
    mainLayout->addWidget(d->mainSplitter, 1);
    retranslateUi();
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::createInputGroup()
{
    inputGroup = new QGroupBox(tr("Frame Input"), q_ptr);
    auto groupLayout = new QVBoxLayout(inputGroup);

    // Controls Row
    auto controlsLayout = new QHBoxLayout();
    
    protocolLabel = new QLabel(tr("Protocol:"), q_ptr);
    controlsLayout->addWidget(protocolLabel);
    protocolCombo = new QComboBox(q_ptr);
    protocolCombo->addItem(tr("Auto Detect"), QVariant::fromValue(ProtocolType::Unknown));
    protocolCombo->addItem(tr("Modbus TCP"), QVariant::fromValue(ProtocolType::Tcp));
    protocolCombo->addItem(tr("Modbus RTU"), QVariant::fromValue(ProtocolType::Rtu));
    controlsLayout->addWidget(protocolCombo);

    controlsLayout->addSpacing(20);
    startAddrLabel = new QLabel(tr("Start Address (for Response):"), q_ptr);
    controlsLayout->addWidget(startAddrLabel);
    startAddrEdit = new QLineEdit(q_ptr);
    startAddrEdit->setFixedWidth(88);
    auto* hexValidator = new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-FxXHh]*"), q_ptr);
    startAddrEdit->setValidator(hexValidator);
    controlsLayout->addWidget(startAddrEdit);

    controlsLayout->addStretch();
    auto* actionsContainer = new QWidget(q_ptr);
    auto* actionsLayout = new QHBoxLayout(actionsContainer);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(6);

    formatBtn = new QPushButton(tr("Format Hex"), q_ptr);
    connect(formatBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onFormatClicked);
    formatBtn->setMinimumWidth(86);
    actionsLayout->addWidget(formatBtn);

    parseBtn = new QPushButton(tr("Parse"), q_ptr);
    connect(parseBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onParseClicked);
    parseBtn->setMinimumWidth(86);
    actionsLayout->addWidget(parseBtn);

    clearBtn = new QPushButton(tr("Clear"), q_ptr);
    connect(clearBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onClearClicked);
    clearBtn->setMinimumWidth(86);
    actionsLayout->addWidget(clearBtn);

    controlsLayout->addWidget(actionsContainer, 0, Qt::AlignRight);
    groupLayout->addLayout(controlsLayout);

    // Input Editor
    inputEditor = new QPlainTextEdit(q_ptr);
    inputEditor->setPlaceholderText(tr("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)"));
    inputEditor->setMinimumHeight(64);
    inputEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    groupLayout->addWidget(inputEditor);

    connect(startAddrEdit, &QLineEdit::textChanged, q_ptr, [this]() { saveSettings(); });
    inputGroup->setMinimumHeight(0);
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::createResultGroup()
{
    resultGroup = new QGroupBox(tr("Analysis Result"), q_ptr);
    auto groupLayout = new QVBoxLayout(resultGroup);

    auto resultToolbarLayout = new QHBoxLayout();
    resultToolbarLayout->setContentsMargins(0, 0, 0, 0);
    resultToolbarLayout->setSpacing(6);
    
    auto* statusContainer = new QWidget(q_ptr);
    auto* statusAreaLayout = new QVBoxLayout(statusContainer);
    statusAreaLayout->setContentsMargins(0, 0, 0, 0);
    statusAreaLayout->setSpacing(2);
    
    auto* statusLineLayout = new QHBoxLayout();
    statusLineLayout->setContentsMargins(0, 0, 0, 0);
    statusLineLayout->setSpacing(4);

    statusTitleLabel = new QLabel(tr("Status:"), q_ptr);
    statusTitleLabel->setStyleSheet("color: gray;");
    statusLineLayout->addWidget(statusTitleLabel);

    statusLabel = new QLabel(tr("Ready"), q_ptr);
    statusLabel->setStyleSheet("font-weight: bold; color: gray;");
    statusLineLayout->addWidget(statusLabel);
    statusLineLayout->addStretch();
    
    statusAreaLayout->addLayout(statusLineLayout);
    
    linkageTipLabel = new QLabel(q_ptr);
    linkageTipLabel->setStyleSheet("color: #10B981; font-size: 11px; font-weight: normal;");
    linkageTipLabel->setText(tr("Tip: \"Pause\" to edit description"));
    linkageTipLabel->setVisible(false);
    statusAreaLayout->addWidget(linkageTipLabel);
    
    resultToolbarLayout->addWidget(statusContainer);
    
    // Live Indicators (Left Aligned as per Original UI)
    auto* liveContainer = new QWidget(q_ptr);
    auto* liveLayout = new QHBoxLayout(liveContainer);
    liveLayout->setContentsMargins(0, 0, 0, 0);
    liveLayout->setSpacing(4);
    
    liveLabel = new QLabel(q_ptr);
    liveLabel->setStyleSheet("color: #10B981; font-weight: bold; padding: 2px 6px; border: 1px solid #10B981; border-radius: 4px;");
    liveLabel->setVisible(false);
    liveLayout->addWidget(liveLabel);

    linkagePauseBtn = new QPushButton(tr("Pause Refresh"), q_ptr);
    linkagePauseBtn->setMinimumHeight(28);
    linkagePauseBtn->setVisible(false);
    connect(linkagePauseBtn, &QPushButton::clicked, q_ptr, [this]() {
        isLivePaused = !isLivePaused;
        linkagePauseBtn->setText(isLivePaused ? tr("Resume Refresh") : tr("Pause Refresh"));
        if (isLivePaused) {
            linkagePauseBtn->setStyleSheet("background-color: #F59E0B; color: white; border: 1px solid #D97706; font-weight: bold; border-radius: 4px;");
        } else {
            linkagePauseBtn->setStyleSheet("");
        }
    });
    liveLayout->addWidget(linkagePauseBtn);

    linkageStopBtn = new QPushButton(tr("Stop Link"), q_ptr);
    linkageStopBtn->setStyleSheet("color: #EF4444; border: 1px solid #EF4444; background-color: white; font-weight: bold; padding: 0 10px; border-radius: 4px;");
    linkageStopBtn->setMinimumHeight(28);
    linkageStopBtn->setVisible(false);
    connect(linkageStopBtn, &QPushButton::clicked, q_ptr, [this]() {
        q_ptr->clearResult(); // Clear UI state immediately
        emit q_ptr->linkageStopRequested();
    });
    liveLayout->addWidget(linkageStopBtn);
    
    resultToolbarLayout->addWidget(liveContainer);
    resultToolbarLayout->addStretch();
    
    displayModeLabel = new QLabel(tr("Decode Mode:"), q_ptr);
    displayModeCombo = new QComboBox(q_ptr);
    displayModeCombo->addItem(tr("Unsigned"), static_cast<int>(NumberDisplayMode::Unsigned));
    displayModeCombo->addItem(tr("Signed"), static_cast<int>(NumberDisplayMode::Signed));
    displayModeCombo->setCurrentIndex(0);
    displayModeCombo->setMinimumContentsLength(8);
    displayModeCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    connect(displayModeCombo, qOverload<int>(&QComboBox::currentIndexChanged), q_ptr, [this]() {
        displayMode = static_cast<NumberDisplayMode>(displayModeCombo->currentData().toInt());
        if (!isLiveMode && !inputEditor->toPlainText().trimmed().isEmpty()) {
            q_ptr->onParseClicked();
        } else if (isLiveMode && lastLiveResult.isValid) {
            q_ptr->renderResult(lastLiveResult);
        }
    });

    registerOrderLabel = new QLabel(tr("Byte Order:"), q_ptr);
    registerOrderCombo = new QComboBox(q_ptr);
    registerOrderCombo->addItem(tr("ABCD(default)"), static_cast<int>(modbus::base::RegisterOrder::ABCD));
    registerOrderCombo->addItem("BADC", static_cast<int>(modbus::base::RegisterOrder::BADC));
    registerOrderCombo->addItem("CDAB", static_cast<int>(modbus::base::RegisterOrder::CDAB));
    registerOrderCombo->addItem("DCBA", static_cast<int>(modbus::base::RegisterOrder::DCBA));
    registerOrderCombo->setMinimumWidth(80);
    connect(registerOrderCombo, qOverload<int>(&QComboBox::currentIndexChanged), q_ptr, [this](int index) {
        registerOrder = static_cast<modbus::base::RegisterOrder>(registerOrderCombo->itemData(index).toInt());
        if (!isLiveMode && !inputEditor->toPlainText().trimmed().isEmpty()) {
            q_ptr->onParseClicked();
        }
    });

    resultToolbarLayout->addWidget(displayModeLabel);
    resultToolbarLayout->addWidget(displayModeCombo);
    resultToolbarLayout->addSpacing(16);
    resultToolbarLayout->addWidget(registerOrderLabel);
    resultToolbarLayout->addWidget(registerOrderCombo);
    resultToolbarLayout->addSpacing(16);
    
    importJsonBtn = new QPushButton(tr("Import Config"), q_ptr);
    connect(importJsonBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onImportJsonClicked);
    exportJsonBtn = new QPushButton(tr("Export Config"), q_ptr);
    connect(exportJsonBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onExportJsonClicked);
    exportCsvBtn = new QPushButton(tr("Export CSV"), q_ptr);
    connect(exportCsvBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onExportCsvClicked);
    
    const QList<QPushButton*> actionButtons = { importJsonBtn, exportJsonBtn, exportCsvBtn };
    for (auto* button : actionButtons) {
        button->setMinimumWidth(0);
        button->setMinimumHeight(28);
        resultToolbarLayout->addWidget(button);
    }

    toggleHistoryBtn = new QPushButton(q_ptr);
    toggleHistoryBtn->setMinimumWidth(0);
    toggleHistoryBtn->setMinimumHeight(28);
    connect(toggleHistoryBtn, &QPushButton::clicked, q_ptr, [this]() { setHistoryCollapsed(!historyCollapsed); });
    updateHistoryToggleText();
    resultToolbarLayout->addWidget(toggleHistoryBtn);
    
    groupLayout->addLayout(resultToolbarLayout);

    resultTabs = new QTabWidget(q_ptr);
    structureTab = new QWidget(q_ptr);
    auto structureLayout = new QVBoxLayout(structureTab);
    structureLayout->setContentsMargins(0, 0, 0, 0);
    overviewTree = new QTreeWidget(structureTab);
    overviewTree->setHeaderLabels({tr("Field"), tr("Value"), tr("Description")});
    overviewTree->setColumnWidth(0, 200);
    overviewTree->setColumnWidth(1, 150);
    structureLayout->addWidget(overviewTree);
    resultTabs->addTab(structureTab, tr("Structure"));

    dataTable = new QTableWidget(q_ptr);
    dataTable->setColumnCount(7);
    dataTable->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Scale"), tr("Value"), tr("Description")});
    dataTable->horizontalHeader()->setStretchLastSection(true);
    connect(dataTable, &QTableWidget::itemChanged, q_ptr, [this](QTableWidgetItem* item) {
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

    contentSplitter = new QSplitter(Qt::Horizontal, q_ptr);
    contentSplitter->addWidget(resultTabs);

    historyGroup = new QGroupBox(tr("History"), q_ptr);
    auto historyLayout = new QVBoxLayout(historyGroup);
    historyList = new QListWidget(q_ptr);
    historyLayout->addWidget(historyList);
    clearHistoryBtn = new QPushButton(tr("Clear History"), q_ptr);
    connect(clearHistoryBtn, &QPushButton::clicked, q_ptr, &FrameAnalyzerWidget::onClearHistoryClicked);
    historyLayout->addWidget(clearHistoryBtn);
    connect(historyList, &QListWidget::currentRowChanged, q_ptr, &FrameAnalyzerWidget::onHistorySelectionChanged);

    contentSplitter->addWidget(historyGroup);
    contentSplitter->setStretchFactor(0, 1);
    contentSplitter->setStretchFactor(1, 0);
    contentSplitter->setSizes({800, app::constants::Values::Ui::kFrameAnalyzerDefaultHistoryWidth}); 
    groupLayout->addWidget(contentSplitter);

    updateAdaptiveLayout();
}

void FrameAnalyzerWidget::onFormatClicked()
{
    Q_D(FrameAnalyzerWidget);
    const QString text = FrameParseWorker::normalizeHexInput(d->inputEditor->toPlainText());
    
    QString formatted;
    for (int i = 0; i < text.length(); i += 2) {
        formatted.append(text.mid(i, 2));
        if (i + 2 < text.length()) formatted.append(QLatin1Char(' '));
    }
    
    d->inputEditor->setPlainText(formatted.toUpper());
}

void FrameAnalyzerWidget::onClearClicked()
{
    Q_D(FrameAnalyzerWidget);
    ++d->latestParseRequestId;
    d->parseInProgress = false;
    if (d->parseBtn) d->parseBtn->setEnabled(true);
    d->inputEditor->clear();
    clearResult();
}

void FrameAnalyzerWidget::onParseClicked()
{
    Q_D(FrameAnalyzerWidget);
    clearResult();
    const QString rawInput = d->inputEditor->toPlainText();
    const QString hexStr = FrameParseWorker::normalizeHexInput(rawInput);
    
    if (hexStr.isEmpty()) {
        d->statusLabel->setText(tr("Error: Empty input"));
        d->statusLabel->setStyleSheet(QStringLiteral("color: red;"));
        return;
    }
    
    ProtocolType type = d->protocolCombo->currentData().value<ProtocolType>();
    bool addrOk = false;
    int addrVal = modbus::base::ModbusDataHelper::parseSmartInt(d->startAddrEdit->text(), &addrOk);
    if (!addrOk || addrVal < 0 || addrVal > 65535) {
        d->statusLabel->setText(tr("Invalid Address (0-65535): %1").arg(d->startAddrEdit->text()));
        d->statusLabel->setStyleSheet(QStringLiteral("color: red;"));
        return;
    }

    ++d->latestParseRequestId;
    d->parseInProgress = true;
    d->statusLabel->setText(tr("Parsing..."));
    d->statusLabel->setStyleSheet(QStringLiteral("color: gray;"));
    if (d->parseBtn) d->parseBtn->setEnabled(false);

    if (d->parseWorker) {
        d->parseWorker->enqueueParse(rawInput, type, static_cast<uint16_t>(addrVal), d->registerOrder, d->latestParseRequestId);
    }
}

void FrameAnalyzerWidget::onParseFinished(const ParseResult& result, quint64 requestId)
{
    Q_D(FrameAnalyzerWidget);
    if (requestId != d->latestParseRequestId) return;

    d->parseInProgress = false;
    if (d->parseBtn) d->parseBtn->setEnabled(true);

    d->currentResult = result;
    renderResult(result);
    if (result.isValid) {
        d->addToHistory(result);
    } else {
        d->statusLabel->setText(result.error.isEmpty() ? tr("Parse Failed") : result.error);
        d->statusLabel->setStyleSheet(QStringLiteral("color: red;"));
    }
}

void FrameAnalyzerWidget::onHistorySelectionChanged(int row)
{
    Q_D(FrameAnalyzerWidget);
    if (row < 0 || row >= d->historyResults.size()) return;
    d->currentResult = d->historyResults.at(row);
    renderResult(d->currentResult);
}

void FrameAnalyzerWidget::onClearHistoryClicked()
{
    Q_D(FrameAnalyzerWidget);
    d->historyResults.clear();
    d->historyList->clear();
}

void FrameAnalyzerWidget::onExportJsonClicked()
{
    Q_D(FrameAnalyzerWidget);
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export Config"), QString(), tr("JSON Files (*.json)"));
    if (filePath.isEmpty()) return;

    QString error;
    bool ok = AnalyzerExporter::saveMetadataJson(filePath, 
                                               d->startAddrEdit->text(),
                                               (d->displayMode == NumberDisplayMode::Signed ? QStringLiteral("signed") : QStringLiteral("unsigned")),
                                               d->metadataByAddress,
                                               &error);
    if (!ok) {
        QMessageBox::warning(this, tr("Export Failed"), error);
    }
}

void FrameAnalyzerWidget::onImportJsonClicked()
{
    Q_D(FrameAnalyzerWidget);
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import Config"), QString(), tr("JSON Files (*.json)"));
    if (filePath.isEmpty()) return;

    ImportResult result = AnalyzerExporter::loadMetadataJson(filePath);
    if (!result.success) {
        QMessageBox::warning(this, tr("Import Failed"), result.error);
        return;
    }

    if (d->startAddrEdit) d->startAddrEdit->setText(result.startAddress);
    if (d->displayModeCombo) {
        int idx = (result.displayMode == QStringLiteral("signed")) ? 1 : 0;
        d->displayModeCombo->setCurrentIndex(idx);
    }
    
    d->metadataByAddress = result.metadata;
    if (!d->inputEditor->toPlainText().trimmed().isEmpty()) {
        onParseClicked();
    } else {
        clearResult();
    }
}

void FrameAnalyzerWidget::onExportCsvClicked()
{
    Q_D(FrameAnalyzerWidget);
    if (!d->currentResult.isValid || !d->dataTable || d->dataTable->rowCount() == 0) {
        QMessageBox::information(this, tr("No Data"), tr("There is no data to export."));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export CSV"), 
        QStringLiteral("analysis_%1.csv").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))),
        tr("CSV Files (*.csv)"));
    if (filePath.isEmpty()) return;

    QStringList lines;
    QStringList headers;
    for (int c = 0; c < d->dataTable->columnCount(); ++c) {
        headers << AnalyzerExporter::escapeCsvValue(d->dataTable->horizontalHeaderItem(c)->text());
    }
    lines << headers.join(QLatin1Char(','));
    
    for (int r = 0; r < d->dataTable->rowCount(); ++r) {
        QStringList row;
        for (int c = 0; c < d->dataTable->columnCount(); ++c) {
            row << AnalyzerExporter::escapeCsvValue(d->dataTable->item(r, c)->text());
        }
        lines << row.join(QLatin1Char(','));
    }

    QString error;
    if (!AnalyzerExporter::writeCsvChunk(filePath, lines, true, &error)) {
        QMessageBox::warning(this, tr("Export Failed"), error);
    }
}

void FrameAnalyzerWidget::renderResult(const ParseResult& result)
{
    Q_D(FrameAnalyzerWidget);
    d->isUpdatingDataTable = true;
    
    if (d->overviewTree) {
        d->overviewTree->clear();
    }
    if (d->dataTable) {
        d->dataTable->setRowCount(0);
    }

    if (!result.isValid) {
        d->isUpdatingDataTable = false;
        if (!d->isLiveMode) {
            d->statusLabel->setText(tr("Parse Failed: %1").arg(result.error));
            d->statusLabel->setStyleSheet(QStringLiteral("color: red; font-weight: bold;"));
        }
        return;
    }

    // Helper for adding tree items
    auto addTreeItem = [](QTreeWidgetItem* parent, const QString& field, const QString& value, const QString& desc = QString()) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, field);
        item->setText(1, value);
        item->setText(2, desc);
        return item;
    };

    if (d->overviewTree) {
        d->overviewTree->clear();
        if (!d->isLiveMode) {
            auto* root = new QTreeWidgetItem(d->overviewTree);
            root->setText(0, tr("Frame"));
            root->setText(1, tr("%1 bytes").arg(result.rawFrame.size()));
            root->setText(2, result.protocol == ProtocolType::Tcp ? tr("Modbus TCP") : tr("Modbus RTU"));

            addTreeItem(root, tr("Raw Hex"), QStringLiteral("0x") + result.rawFrame.toHex().toUpper());
            
            if (result.protocol == ProtocolType::Tcp) {
                auto* mbap = addTreeItem(root, tr("MBAP Header"), QStringLiteral("-"));
                addTreeItem(mbap, tr("Transaction ID"), QString::number(result.transactionId));
                addTreeItem(mbap, tr("Protocol ID"), QString::number(result.protocolId));
                addTreeItem(mbap, tr("Length"), QString::number(result.length));
                addTreeItem(mbap, tr("Unit ID"), QString::number(result.slaveId));
            } else {
                addTreeItem(root, tr("Slave ID"), QString::number(result.slaveId));
            }

            auto* pdu = addTreeItem(root, tr("PDU"), QStringLiteral("-"));
            const QString fcHex = QStringLiteral("0x%1").arg(static_cast<int>(result.functionCode), 2, 16, QChar('0')).toUpper();
            addTreeItem(pdu, tr("Function Code"), fcHex);
            
            if (result.isException) {
                addTreeItem(pdu, tr("Exception Code"), QStringLiteral("0x%1").arg(static_cast<int>(result.exceptionCode), 2, 16, QChar('0')).toUpper());
            }

            if (result.protocol == ProtocolType::Rtu) {
                QString crcStatus = result.checksumValid ? tr("Valid") : tr("Invalid (Expected 0x%1)").arg(QString::number(result.calculatedChecksum, 16).toUpper());
                addTreeItem(root, tr("CRC16"), QStringLiteral("0x%1").arg(result.checksum, 4, 16, QChar('0')).toUpper(), crcStatus);
            }
            
            d->overviewTree->expandAll();
        }
    }

    if (d->dataTable) {
        d->dataTable->setRowCount(result.dataItems.size());
        for (int i = 0; i < result.dataItems.size(); ++i) {
            const auto& item = result.dataItems[i];
            DataMetadata meta = d->metadataByAddress.value(item.address);
            
            auto* addrItem = new QTableWidgetItem(QStringLiteral("%1 (0x%2)")
                .arg(item.address)
                .arg(QString::number(item.address, 16).toUpper().rightJustified(4, QLatin1Char('0'))));
            addrItem->setData(Qt::UserRole, item.address);
            addrItem->setFlags(addrItem->flags() & ~Qt::ItemIsEditable);
            d->dataTable->setItem(i, 0, addrItem);

            auto* hexItem = new QTableWidgetItem(ValueFormatter::formatHexValue(item.rawBytes, item.hexString));
            hexItem->setFlags(addrItem->flags());
            d->dataTable->setItem(i, 1, hexItem);

            auto* decItem = new QTableWidgetItem(ValueFormatter::formatDecimalValue(item.value, d->displayMode));
            decItem->setFlags(addrItem->flags());
            d->dataTable->setItem(i, 2, decItem);

            auto* binItem = new QTableWidgetItem(ValueFormatter::formatBinaryValue(item.rawBytes, item.binaryString));
            binItem->setFlags(addrItem->flags());
            d->dataTable->setItem(i, 3, binItem);

            d->dataTable->setItem(i, 4, new QTableWidgetItem(QString::number(meta.scale, 'g', 12)));
            
            auto* scaledItem = new QTableWidgetItem(ValueFormatter::formatScaledValue(item.value, meta, d->displayMode));
            scaledItem->setFlags(addrItem->flags());
            d->dataTable->setItem(i, 5, scaledItem);

            d->dataTable->setItem(i, 6, new QTableWidgetItem(meta.description));

            d->applyMetadataToRow(i, item.value, meta);
        }
    }

    d->isUpdatingDataTable = false;
    if (!d->isLiveMode) {
        d->statusLabel->setText(result.warnings.isEmpty() ? tr("Success") : tr("Success with warnings"));
        d->statusLabel->setStyleSheet(result.warnings.isEmpty() ? 
            QStringLiteral("color: #10B981; font-weight: bold;") : 
            QStringLiteral("color: #F59E0B; font-weight: bold;"));
    }
}

void FrameAnalyzerWidget::clearResult()
{
    Q_D(FrameAnalyzerWidget);
    d->statusLabel->setText(tr("Ready"));
    d->statusLabel->setStyleSheet(QStringLiteral("color: gray;"));
    
    const bool wasLive = d->isLiveMode;
    d->isLiveMode = false;
    d->isLivePaused = false;

    d->liveLabel->setVisible(false);
    d->linkageStopBtn->setVisible(false);
    if (d->linkageTipLabel) d->linkageTipLabel->setVisible(false);
    if (d->linkagePauseBtn) {
        d->linkagePauseBtn->setVisible(false);
        d->linkagePauseBtn->setText(tr("Pause Refresh"));
        d->linkagePauseBtn->setStyleSheet(QString());
    }
    
    if (d->registerOrderCombo) d->registerOrderCombo->setEnabled(true);

    if (wasLive && d->lastLiveResult.isValid) {
        renderResult(d->lastLiveResult);
    }
    if (d->resultTabs) d->resultTabs->setTabText(0, tr("Structure"));
}

void FrameAnalyzerWidget::processLivePdu(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr)
{
    Q_D(FrameAnalyzerWidget);
    d->isLiveMode = true;
    if (d->resultTabs) d->resultTabs->setTabText(0, tr("Structure (Live Mode)"));
    if (d->parseInProgress) return;
    
    ParseResult result;
    result.isValid = true;
    result.protocol = protocol;
    result.timestamp = QDateTime::currentDateTime();
    result.functionCode = pdu.functionCode();
    result.isException = pdu.isException();
    result.pduData = pdu.toByteArray();
    result.type = result.isException ? FrameType::Exception : FrameType::Response;
    
    modbus::core::parser::ModbusFrameParser::parsePdu(result, result.pduData, addr, 0);
    
    QString protocolStr = (protocol == ProtocolType::Tcp) ? QStringLiteral("TCP") : QStringLiteral("RTU");
    if (d->liveLabel) {
        d->liveLabel->setText(tr("LIVE: %1").arg(protocolStr));
        d->liveLabel->setVisible(true);
    }
    if (d->statusLabel) {
        d->statusLabel->setText(tr("Live Data Received at %1").arg(result.timestamp.toString("HH:mm:ss.zzz")));
        d->statusLabel->setStyleSheet(QStringLiteral("color: #10B981; font-weight: bold;"));
    }
    if (d->linkageTipLabel) d->linkageTipLabel->setVisible(true);
    if (d->linkagePauseBtn) d->linkagePauseBtn->setVisible(true);
    if (d->linkageStopBtn) d->linkageStopBtn->setVisible(true);
    
    if (d->registerOrderCombo) d->registerOrderCombo->setEnabled(false); // 联动模式通常由会话层控制字节序

    d->lastLiveResult = result;
    if (!d->isLivePaused) renderResult(result);
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::loadSettings()
{
    if (!settingsService) return;
    
    QSignalBlocker blocker(startAddrEdit);
    const QString startAddr = settingsService->value(ui::common::settings_keys::kFrameAnalyzerStartAddr).toString();
    if (!startAddr.isEmpty()) {
        startAddrEdit->setText(startAddr);
    } else {
        startAddrEdit->setText(QString::number(app::constants::Values::Modbus::kDefaultStandardStartAddress));
    }

    const int mode = settingsService->value(ui::common::settings_keys::kFrameAnalyzerDecodeMode).toInt();
    if (displayModeCombo) displayModeCombo->setCurrentIndex(mode);
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::saveSettings()
{
    if (!settingsService) return;
    settingsService->setValue(ui::common::settings_keys::kFrameAnalyzerStartAddr, startAddrEdit->text());
    settingsService->setValue(ui::common::settings_keys::kFrameAnalyzerDecodeMode, displayModeCombo->currentIndex());
}

void FrameAnalyzerWidget::FrameAnalyzerWidgetPrivate::updateAdaptiveLayout()
{
    if (!contentSplitter || !toggleHistoryBtn) return;
    const bool shouldCollapse = q_ptr->width() < 1000;
    if (shouldCollapse != historyAutoCollapsed) {
        historyAutoCollapsed = shouldCollapse;
        setHistoryCollapsed(shouldCollapse);
    }
}

void FrameAnalyzerWidget::resizeEvent(QResizeEvent* event)
{
    Q_D(FrameAnalyzerWidget);
    d->updateAdaptiveLayout();
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
    Q_D(FrameAnalyzerWidget);
    if (d->inputGroup) d->inputGroup->setTitle(tr("Frame Input"));
    if (d->protocolLabel) d->protocolLabel->setText(tr("Protocol:"));
    if (d->protocolCombo) {
        d->protocolCombo->setItemText(0, tr("Auto Detect"));
        d->protocolCombo->setItemText(1, tr("Modbus TCP"));
        d->protocolCombo->setItemText(2, tr("Modbus RTU"));
    }
    if (d->startAddrLabel) d->startAddrLabel->setText(tr("Start Address (for Response):"));
    if (d->startAddrEdit) {
        d->startAddrEdit->setToolTip(tr("Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16)."));
    }
    if (d->displayModeLabel) d->displayModeLabel->setText(tr("Decode Mode:"));
    if (d->displayModeCombo) {
        d->displayModeCombo->setItemText(0, tr("Unsigned"));
        d->displayModeCombo->setItemText(1, tr("Signed"));
    }
    if (d->registerOrderLabel) d->registerOrderLabel->setText(tr("Byte Order:"));
    if (d->registerOrderCombo) {
        d->registerOrderCombo->setItemText(0, tr("ABCD(default)"));
        d->registerOrderCombo->setItemText(1, "BADC");
        d->registerOrderCombo->setItemText(2, "CDAB");
        d->registerOrderCombo->setItemText(3, "DCBA");
    }
    if (d->formatBtn) d->formatBtn->setText(tr("Format Hex"));
    if (d->importJsonBtn) d->importJsonBtn->setText(tr("Import Config"));
    if (d->exportJsonBtn) d->exportJsonBtn->setText(tr("Export Config"));
    if (d->exportCsvBtn) d->exportCsvBtn->setText(tr("Export CSV"));
    
    d->updateHistoryToggleText();
    
    if (d->linkageStopBtn) d->linkageStopBtn->setText(tr("Stop Link"));
    if (d->linkagePauseBtn) {
        d->linkagePauseBtn->setText(d->isLivePaused ? tr("Resume Refresh") : tr("Pause Refresh"));
    }
    if (d->statusTitleLabel) d->statusTitleLabel->setText(tr("Status:"));
    if (d->isLiveMode) {
        QString protocolStr = (d->lastLiveResult.protocol == ProtocolType::Tcp) ? tr("TCP") : tr("RTU");
        if (d->liveLabel) d->liveLabel->setText(tr("LIVE: %1").arg(protocolStr));
        if (d->statusLabel && d->lastLiveResult.isValid) {
            d->statusLabel->setText(tr("Live Data Received at %1").arg(d->lastLiveResult.timestamp.toString("HH:mm:ss.zzz")));
        }
    } else {
        if (d->liveLabel) d->liveLabel->setText(QString());
        if (d->statusLabel) d->statusLabel->setText(tr("Ready"));
    }
    
    if (d->parseBtn) d->parseBtn->setText(tr("Parse"));
    if (d->clearBtn) d->clearBtn->setText(tr("Clear"));
    if (d->inputEditor) {
        d->inputEditor->setPlaceholderText(tr("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)"));
    }

    if (d->resultGroup) d->resultGroup->setTitle(tr("Analysis Result"));
    if (d->historyGroup) d->historyGroup->setTitle(tr("History"));
    if (d->resultTabs) {
        d->resultTabs->setTabText(0, d->isLiveMode ? tr("Structure (Live Mode)") : tr("Structure"));
        d->resultTabs->setTabText(1, tr("Decoded Data"));
    }
    if (d->overviewTree) {
        QTreeWidgetItem* header = d->overviewTree->headerItem();
        header->setText(0, tr("Field"));
        header->setText(1, tr("Value"));
        header->setText(2, tr("Description"));
    }
    if (d->dataTable) {
        d->dataTable->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Scale"), tr("Value"), tr("Description")});
    }
    if (d->clearHistoryBtn) d->clearHistoryBtn->setText(tr("Clear History"));
    
    d->refreshHistoryList();
}

} // namespace ui::widgets
