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
#include "modbus/parser/ModbusFrameParser.h"
#include "modbus/base/ModbusDataHelper.h"
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
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidget>
#include <QStringList>
#include <QResizeEvent>
#include <QTextStream>
#include <QFutureWatcher>
#include <QObject>
#include <QSplitter>
#include <QtConcurrent/QtConcurrent>
#include <memory>

namespace ui::widgets {

using namespace modbus::core::parser;

namespace {

QString byteHex(const QByteArray& bytes)
{
    QStringList parts;
    parts.reserve(bytes.size());
    for (unsigned char byte : bytes) {
        parts << QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
    }
    return parts.join(' ');
}

QString groupBits(const QString& bits)
{
    QString grouped = bits;
    for (int k = grouped.size() - 4; k > 0; k -= 4) {
        grouped.insert(k, ' ');
    }
    return grouped;
}

QString normalizeHexInput(const QString& input)
{
    QString text = input;
    // Remove bracketed metadata segments such as "[12:39:35.668]" / "[RX]".
    text.remove(QRegularExpression("\\[[^\\]]*\\]"));

    const QStringList rawTokens = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QString normalized;
    normalized.reserve(rawTokens.size() * 2);

    for (QString token : rawTokens) {
        token = token.trimmed();
        if (token.isEmpty()) {
            continue;
        }

        const QString upper = token.toUpper();
        if (upper == "RX" || upper == "TX" || upper == "FAIL" || upper == "RTT") {
            continue;
        }

        // Skip obvious timestamp/date-like tokens.
        if (token.contains(':') || token.contains('.') || token.contains('-')) {
            continue;
        }

        if (token.startsWith("0x", Qt::CaseInsensitive)) {
            token = token.mid(2);
        }

        token.remove(QRegularExpression("[^0-9A-Fa-f]"));
        if (token.size() < 2) {
            continue;
        }
        if (token.size() % 2 != 0) {
            token.chop(1);
        }
        if (!token.isEmpty()) {
            normalized.append(token);
        }
    }

    // Fallback for plain contiguous hex input.
    if (normalized.isEmpty()) {
        QString plain = input;
        plain.remove(QRegularExpression("[^0-9A-Fa-f]"));
        if (plain.size() % 2 != 0) {
            plain.chop(1);
        }
        normalized = plain;
    }
    return normalized;
}

uint32_t maskForBits(int bitWidth)
{
    if (bitWidth >= 32) {
        return 0xFFFFFFFFu;
    }
    return (1u << bitWidth) - 1u;
}

class FrameParseWorker : public QObject {
    Q_OBJECT

public slots:
    void enqueueParse(const QString& input,
                      ProtocolType type,
                      uint16_t startAddress,
                      modbus::base::RegisterOrder order,
                      quint64 requestId)
    {
        pendingInput_ = input;
        pendingType_ = type;
        pendingStartAddress_ = startAddress;
        pendingOrder_ = order;
        pendingRequestId_ = requestId;
        hasPendingRequest_ = true;

        if (processing_) {
            return;
        }

        processing_ = true;
        QMetaObject::invokeMethod(this, &FrameParseWorker::processPending, Qt::QueuedConnection);
    }

signals:
    void parseFinished(const modbus::core::parser::ParseResult& result, quint64 requestId);

private slots:
    void processPending()
    {
        while (hasPendingRequest_) {
            const QString input = pendingInput_;
            const ProtocolType type = pendingType_;
            const uint16_t startAddress = pendingStartAddress_;
            const quint64 requestId = pendingRequestId_;
            hasPendingRequest_ = false;

            const modbus::base::RegisterOrder order = pendingOrder_;
            emit parseFinished(buildResult(input, type, startAddress, order), requestId);
        }

        processing_ = false;
    }

private:
    ParseResult buildResult(const QString& input,
                            ProtocolType type,
                            uint16_t startAddress,
                            modbus::base::RegisterOrder order) const
    {
        const QString hexStr = normalizeHexInput(input);

        ParseResult result;
        result.timestamp = QDateTime::currentDateTime();
        if (hexStr.isEmpty()) {
            result.isValid = false;
            result.error = QCoreApplication::translate("FrameAnalyzerWidget", "Error: Empty input");
            return result;
        }

        const QByteArray frame = QByteArray::fromHex(hexStr.toLatin1());
        const bool force = (type != ProtocolType::Unknown);
        return ModbusFrameParser::parse(frame, type, startAddress, 0, force, order);
    }

    QString pendingInput_;
    ProtocolType pendingType_ = ProtocolType::Unknown;
    uint16_t pendingStartAddress_ = 0;
    modbus::base::RegisterOrder pendingOrder_ = modbus::base::RegisterOrder::ABCD;
    quint64 pendingRequestId_ = 0;
    bool hasPendingRequest_ = false;
    bool processing_ = false;
};
}

FrameAnalyzerWidget::FrameAnalyzerWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService)
{
    qRegisterMetaType<ProtocolType>("modbus::core::parser::ProtocolType");
    qRegisterMetaType<ParseResult>("modbus::core::parser::ParseResult");
    qRegisterMetaType<modbus::base::RegisterOrder>("modbus::base::RegisterOrder");

    parseThread_ = new QThread();
    auto* parseWorker = new FrameParseWorker();
    parseWorker_ = parseWorker;
    parseWorker->moveToThread(parseThread_);
    connect(this, &FrameAnalyzerWidget::parseRequested, parseWorker, &FrameParseWorker::enqueueParse, Qt::QueuedConnection);
    connect(parseWorker, &FrameParseWorker::parseFinished, this, &FrameAnalyzerWidget::onParseFinished, Qt::QueuedConnection);
    connect(parseThread_, &QThread::finished, parseThread_, &QObject::deleteLater);
    parseThread_->start();

    setupUi();
}

FrameAnalyzerWidget::~FrameAnalyzerWidget()
{
    if (parseThread_) {
        disconnect(this, nullptr, parseWorker_, nullptr);
        QObject* worker = parseWorker_;
        QThread* thread = parseThread_;
        parseWorker_ = nullptr;
        parseThread_ = nullptr;
        if (thread->isRunning() && worker) {
            QMetaObject::invokeMethod(worker, [worker, thread]() {
                QObject::connect(worker, &QObject::destroyed, thread, &QThread::quit, Qt::UniqueConnection);
                worker->deleteLater();
            }, Qt::QueuedConnection);
        } else {
            delete worker;
            delete thread;
        }
    }
}

QString FrameAnalyzerWidget::formatDecimalValue(const QVariant& value) const
{
    if (!value.isValid()) {
        return "-";
    }
    if (value.typeId() == QMetaType::Bool) {
        return value.toBool() ? "1" : "0";
    }
    if (value.typeId() == QMetaType::UShort || value.typeId() == QMetaType::UInt || value.typeId() == QMetaType::Int) {
        const uint16_t uVal = static_cast<uint16_t>(value.toUInt());
        if (displayMode_ == NumberDisplayMode::Signed) {
            return QString::number(static_cast<int16_t>(uVal));
        }
        return QString::number(uVal);
    }
    return value.toString();
}

double FrameAnalyzerWidget::numericValueForDisplay(const QVariant& value, bool* ok) const
{
    if (ok) {
        *ok = false;
    }
    if (!value.isValid() || value.typeId() == QMetaType::Bool) {
        return 0.0;
    }
    if (value.typeId() == QMetaType::UShort || value.typeId() == QMetaType::UInt || value.typeId() == QMetaType::Int) {
        const uint16_t uVal = static_cast<uint16_t>(value.toUInt());
        if (ok) {
            *ok = true;
        }
        if (displayMode_ == NumberDisplayMode::Signed) {
            return static_cast<double>(static_cast<int16_t>(uVal));
        }
        return static_cast<double>(uVal);
    }
    bool localOk = false;
    const double parsed = value.toDouble(&localOk);
    if (ok) {
        *ok = localOk;
    }
    return localOk ? parsed : 0.0;
}

QString FrameAnalyzerWidget::formatScaledValue(const QVariant& value, const DataMetadata& meta) const
{
    bool ok = false;
    const double raw = numericValueForDisplay(value, &ok);
    if (!ok) {
        return "-";
    }
    return QString::number(raw * meta.scale, 'g', 12);
}

QString FrameAnalyzerWidget::buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta) const
{
    QStringList lines;
    if (!meta.description.trimmed().isEmpty()) {
        lines << tr("Description: %1").arg(meta.description.trimmed());
    }
    bool ok = false;
    const double raw = numericValueForDisplay(value, &ok);
    if (ok) {
        const double scaled = raw * meta.scale;
        lines << tr("Raw: %1").arg(QString::number(raw, 'g', 12));
        lines << tr("Scale: %1").arg(QString::number(meta.scale, 'g', 12));
        lines << tr("Scaled: %1").arg(QString::number(scaled, 'g', 12));
    }
    return lines.join('\n');
}

void FrameAnalyzerWidget::applyMetadataToRow(int row, const QVariant& value, const DataMetadata& meta)
{
    if (!dataTable_ || row < 0 || row >= dataTable_->rowCount()) {
        return;
    }
    QTableWidgetItem* descItem = dataTable_->item(row, 6);
    if (!descItem) {
        return;
    }
    const QString tooltip = buildDescriptionTooltip(value, meta);
    descItem->setToolTip(tooltip);
    QTableWidgetItem* scaledItem = dataTable_->item(row, 5);
    if (scaledItem) {
        scaledItem->setText(formatScaledValue(value, meta));
    }
    QTableWidgetItem* scaleItem = dataTable_->item(row, 4);
    if (scaleItem && scaleItem->text().trimmed().isEmpty()) {
        scaleItem->setText(QString::number(meta.scale, 'g', 12));
    }
}

QString FrameAnalyzerWidget::formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex) const
{
    QString normalized = QString(rawBytes.toHex()).toUpper();
    if (normalized.isEmpty()) {
        normalized = fallbackHex;
    }
    normalized.remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (normalized.isEmpty()) {
        return fallbackHex;
    }
    const int digits = normalized.size();
    bool ok = false;
    const uint32_t rawValue = normalized.toUInt(&ok, 16);
    if (!ok) {
        return fallbackHex;
    }
    const int bitWidth = qMax(1, digits * 4);
    const uint32_t masked = rawValue & maskForBits(bitWidth);
    const QString rawHex = QString("%1").arg(masked, digits, 16, QChar('0')).toUpper();
    return QString("0x%1").arg(rawHex);
}

QString FrameAnalyzerWidget::formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary) const
{
    bool ok = false;
    uint32_t rawValue = 0;
    int bitWidth = 0;
    if (!rawBytes.isEmpty()) {
        QString normalized = QString(rawBytes.toHex()).toUpper();
        normalized.remove(QRegularExpression("[^0-9A-Fa-f]"));
        if (normalized.isEmpty()) {
            return fallbackBinary;
        }
        rawValue = normalized.toUInt(&ok, 16);
        bitWidth = qMax(1, normalized.size() * 4);
    } else {
        QString bitText = fallbackBinary;
        bitText.remove(QRegularExpression("[^01]"));
        if (bitText.isEmpty()) {
            return fallbackBinary;
        }
        rawValue = bitText.toUInt(&ok, 2);
        bitWidth = qMax(1, bitText.size());
    }
    if (!ok) {
        return fallbackBinary;
    }
    const uint32_t masked = rawValue & maskForBits(bitWidth);
    const QString bits = groupBits(QString::number(masked, 2).rightJustified(bitWidth, '0'));
    return bits;
}

void FrameAnalyzerWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    createInputGroup();
    createResultGroup();
    mainSplitter_ = new QSplitter(Qt::Vertical, this);
    mainSplitter_->setChildrenCollapsible(false);
    mainSplitter_->setHandleWidth(6);
    mainSplitter_->addWidget(inputGroup_);
    mainSplitter_->addWidget(resultGroup_);
    mainSplitter_->setStretchFactor(0, 0);
    mainSplitter_->setStretchFactor(1, 1);
    mainSplitter_->setSizes({140, 520});
    mainLayout->addWidget(mainSplitter_, 1);
}

void FrameAnalyzerWidget::createInputGroup()
{
    inputGroup_ = new QGroupBox(tr("Frame Input"), this);
    auto groupLayout = new QVBoxLayout(inputGroup_);

    // Controls Row
    auto controlsLayout = new QHBoxLayout();
    
    protocolLabel_ = new QLabel(tr("Protocol:"), this);
    controlsLayout->addWidget(protocolLabel_);
    protocolCombo_ = new QComboBox(this);
    protocolCombo_->addItem(tr("Auto Detect"), QVariant::fromValue(ProtocolType::Unknown));
    protocolCombo_->addItem(tr("Modbus TCP"), QVariant::fromValue(ProtocolType::Tcp));
    protocolCombo_->addItem(tr("Modbus RTU"), QVariant::fromValue(ProtocolType::Rtu));
    controlsLayout->addWidget(protocolCombo_);

    controlsLayout->addSpacing(20);
    startAddrLabel_ = new QLabel(tr("Start Address (for Response):"), this);
    controlsLayout->addWidget(startAddrLabel_);
    startAddrEdit_ = new QLineEdit(this);
    startAddrEdit_->setFixedWidth(88); // Wide enough for 0xFFFF/65535
    auto* hexValidator = new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-FxXHh]*"), this);
    startAddrEdit_->setValidator(hexValidator);
    controlsLayout->addWidget(startAddrEdit_);

    controlsLayout->addStretch();
    auto* actionsContainer = new QWidget(this);
    auto* actionsLayout = new QHBoxLayout(actionsContainer);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(6);

    formatBtn_ = new QPushButton(tr("Format Hex"), this);
    connect(formatBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onFormatClicked);
    formatBtn_->setMinimumWidth(86);
    actionsLayout->addWidget(formatBtn_);

    parseBtn_ = new QPushButton(tr("Parse"), this);
    connect(parseBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onParseClicked);
    parseBtn_->setMinimumWidth(86);
    actionsLayout->addWidget(parseBtn_);

    clearBtn_ = new QPushButton(tr("Clear"), this);
    connect(clearBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearClicked);
    clearBtn_->setMinimumWidth(86);
    actionsLayout->addWidget(clearBtn_);

    controlsLayout->addWidget(actionsContainer, 0, Qt::AlignRight);

    groupLayout->addLayout(controlsLayout);

    // Input Editor
    inputEditor_ = new QPlainTextEdit(this);
    inputEditor_->setPlaceholderText(tr("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)"));
    inputEditor_->setMinimumHeight(64);
    inputEditor_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    groupLayout->addWidget(inputEditor_);

    connect(startAddrEdit_, &QLineEdit::textChanged, this, &FrameAnalyzerWidget::saveSettings);
    inputGroup_->setMinimumHeight(0);
    loadSettings();
}

void FrameAnalyzerWidget::createResultGroup()
{
    resultGroup_ = new QGroupBox(tr("Analysis Result"), this);
    auto groupLayout = new QVBoxLayout(resultGroup_);

    auto resultToolbarLayout = new QHBoxLayout();
    resultToolbarLayout->setContentsMargins(0, 0, 0, 0);
    resultToolbarLayout->setSpacing(6);
    auto* statusContainer = new QWidget(this);
    auto* statusAreaLayout = new QVBoxLayout(statusContainer);
    statusAreaLayout->setContentsMargins(0, 0, 0, 0);
    statusAreaLayout->setSpacing(2);
    
    statusLabel_ = new QLabel(tr("Ready"), this);
    statusLabel_->setStyleSheet("font-weight: bold; color: gray;");
    statusAreaLayout->addWidget(statusLabel_);
    
    linkageTipLabel_ = new QLabel(this);
    linkageTipLabel_->setStyleSheet("color: #10B981; font-size: 11px; font-weight: normal;");
    linkageTipLabel_->setText(tr("Tip: \"Pause\" to edit description"));
    linkageTipLabel_->setVisible(false);
    statusAreaLayout->addWidget(linkageTipLabel_);
    
    resultToolbarLayout->addWidget(statusContainer);
    
    // Live Indicators Container
    auto* liveContainer = new QWidget(this);
    auto* liveLayout = new QHBoxLayout(liveContainer);
    liveLayout->setContentsMargins(0, 0, 0, 0);
    liveLayout->setSpacing(4);
    
    liveLabel_ = new QLabel(this);
    liveLabel_->setStyleSheet("color: #10B981; font-weight: bold; padding: 2px 6px; border: 1px solid #10B981; border-radius: 4px;");
    liveLabel_->setVisible(false);
    liveLayout->addWidget(liveLabel_);

    linkagePauseBtn_ = new QPushButton(tr("Pause Refresh"), this);
    linkagePauseBtn_->setObjectName("linkagePauseBtn");
    linkagePauseBtn_->setMinimumHeight(28);
    linkagePauseBtn_->setVisible(false);
    connect(linkagePauseBtn_, &QPushButton::clicked, this, [this]() {
        isLivePaused_ = !isLivePaused_;
        linkagePauseBtn_->setText(isLivePaused_ ? tr("Resume Refresh") : tr("Pause Refresh"));
        if (isLivePaused_) {
            linkagePauseBtn_->setStyleSheet("background-color: #F59E0B; color: white; border: 1px solid #D97706; font-weight: bold; border-radius: 4px;");
        } else {
            linkagePauseBtn_->setStyleSheet("");
        }
    });
    liveLayout->addWidget(linkagePauseBtn_);

    linkageStopBtn_ = new QPushButton(tr("Stop Link"), this);
    linkageStopBtn_->setObjectName("linkageStopBtn");
    linkageStopBtn_->setStyleSheet("color: #EF4444; border: 1px solid #EF4444; background-color: white; font-weight: bold; padding: 0 10px; border-radius: 4px;");
    linkageStopBtn_->setMinimumHeight(28);
    linkageStopBtn_->setVisible(false);
    connect(linkageStopBtn_, &QPushButton::clicked, this, [this]() {
        emit linkageStopRequested();
        clearResult(); // Locally reset UI immediately
    });
    liveLayout->addWidget(linkageStopBtn_);
    resultToolbarLayout->addWidget(liveContainer);
    resultToolbarLayout->addStretch();
    
    displayModeLabel_ = new QLabel(tr("Decode Mode:"), this);
    displayModeCombo_ = new QComboBox(this);
    displayModeCombo_->addItem(tr("Unsigned"), static_cast<int>(NumberDisplayMode::Unsigned));
    displayModeCombo_->addItem(tr("Signed"), static_cast<int>(NumberDisplayMode::Signed));
    displayModeCombo_->setCurrentIndex(0);
    displayModeCombo_->setMinimumContentsLength(8);
    displayModeCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    connect(displayModeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        if (!isLiveMode_ && !inputEditor_->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        } else if (isLiveMode_ && lastLiveResult_.isValid) {
            renderResult(lastLiveResult_);
        }
    });

    registerOrderLabel_ = new QLabel(tr("Byte Order:"), this);
    registerOrderCombo_ = new QComboBox(this);
    registerOrderCombo_->addItem(tr("ABCD(default)"), static_cast<int>(modbus::base::RegisterOrder::ABCD));
    registerOrderCombo_->addItem("BADC", static_cast<int>(modbus::base::RegisterOrder::BADC));
    registerOrderCombo_->addItem("CDAB", static_cast<int>(modbus::base::RegisterOrder::CDAB));
    registerOrderCombo_->addItem("DCBA", static_cast<int>(modbus::base::RegisterOrder::DCBA));
    registerOrderCombo_->setCurrentIndex(0);
    registerOrderCombo_->setMinimumWidth(80);
    connect(registerOrderCombo_, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        registerOrder_ = static_cast<modbus::base::RegisterOrder>(registerOrderCombo_->itemData(index).toInt());
        if (!isLiveMode_ && !inputEditor_->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        }
    });

    resultToolbarLayout->addWidget(displayModeLabel_);
    resultToolbarLayout->addWidget(displayModeCombo_);
    resultToolbarLayout->addSpacing(16);
    resultToolbarLayout->addWidget(registerOrderLabel_);
    resultToolbarLayout->addWidget(registerOrderCombo_);
    resultToolbarLayout->addSpacing(16);
    
    importJsonBtn_ = new QPushButton(tr("Import Config"), this);
    exportJsonBtn_ = new QPushButton(tr("Export Config"), this);
    exportCsvBtn_ = new QPushButton(tr("Export CSV"), this);
    const QList<QPushButton*> actionButtons = {
        importJsonBtn_, exportJsonBtn_, exportCsvBtn_
    };
    for (auto* button : actionButtons) {
        button->setMinimumWidth(0);
        button->setMinimumHeight(28);
        resultToolbarLayout->addWidget(button);
    }
    toggleHistoryBtn_ = new QPushButton(this);
    toggleHistoryBtn_->setMinimumWidth(0);
    toggleHistoryBtn_->setMinimumHeight(28);
    resultToolbarLayout->addWidget(toggleHistoryBtn_);
    groupLayout->addLayout(resultToolbarLayout);

    resultTabs_ = new QTabWidget(this);
    resultTabs_->setMinimumSize(0, 0);
    resultTabs_->setMinimumWidth(320);

    structureTab_ = new QWidget(this);
    auto structureLayout = new QVBoxLayout(structureTab_);
    structureLayout->setContentsMargins(0, 0, 0, 0);
    structureLayout->setSpacing(6);
    structureTab_->setMinimumSize(0, 0);

    overviewTree_ = new QTreeWidget(structureTab_);
    overviewTree_->setHeaderLabels({tr("Field"), tr("Value"), tr("Description")});
    overviewTree_->setColumnWidth(0, 200);
    overviewTree_->setColumnWidth(1, 150);
    overviewTree_->setMinimumSize(0, 0);
    structureLayout->addWidget(overviewTree_, 3);

    resultTabs_->addTab(structureTab_, tr("Structure"));

    // Tab 2: Decoded Data
    dataTable_ = new QTableWidget(this);
    dataTable_->setColumnCount(7);
    dataTable_->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Scale"), tr("Value"), tr("Description")});
    dataTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    dataTable_->horizontalHeader()->setMinimumSectionSize(40);
    dataTable_->horizontalHeader()->setStretchLastSection(true);
    dataTable_->setMinimumSize(0, 0);
    dataTable_->setColumnWidth(0, 110);
    dataTable_->setColumnWidth(1, 120);
    dataTable_->setColumnWidth(2, 88);
    dataTable_->setColumnWidth(3, 150);
    dataTable_->setColumnWidth(4, 88);
    dataTable_->setColumnWidth(5, 96);
    dataTable_->setColumnWidth(6, 260);
    connect(dataTable_, &QTableWidget::itemChanged, this, &FrameAnalyzerWidget::onDataTableItemChanged);
    resultTabs_->addTab(dataTable_, tr("Data Details"));

    connect(importJsonBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onImportJsonClicked);
    connect(exportJsonBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onExportJsonClicked);
    connect(exportCsvBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onExportCsvClicked);
    connect(displayModeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < 0) {
            return;
        }
        const auto selected = static_cast<NumberDisplayMode>(displayModeCombo_->itemData(index).toInt());
        if (displayMode_ == selected) {
            return;
        }
        displayMode_ = selected;
        if (!inputEditor_->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        }
    });

    historyGroup_ = new QGroupBox(tr("History"), this);
    auto historyLayout = new QVBoxLayout(historyGroup_);
    historyLayout->setContentsMargins(6, 6, 6, 6);
    historyLayout->setSpacing(4);
    historyGroup_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    historyList_ = new QListWidget(historyGroup_);
    historyGroup_->setMinimumWidth(0);
    historyGroup_->setMaximumWidth(180);
    historyLayout->addWidget(historyList_);
    historyList_->setMinimumSize(0, 0);
    clearHistoryBtn_ = new QPushButton(tr("Clear History"), historyGroup_);
    clearHistoryBtn_->setFixedHeight(24);
    historyLayout->addWidget(clearHistoryBtn_);

    connect(historyList_, &QListWidget::currentRowChanged, this, &FrameAnalyzerWidget::onHistorySelectionChanged);
    connect(clearHistoryBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearHistoryClicked);
    connect(toggleHistoryBtn_, &QPushButton::clicked, this, [this]() {
        historyAutoCollapsed_ = false;
        setHistoryCollapsed(!historyCollapsed_);
    });

    contentSplitter_ = new QSplitter(Qt::Horizontal, resultGroup_);
    contentSplitter_->setChildrenCollapsible(true);
    contentSplitter_->setHandleWidth(6);
    contentSplitter_->addWidget(resultTabs_);
    contentSplitter_->addWidget(historyGroup_);
    contentSplitter_->setCollapsible(0, false);
    contentSplitter_->setCollapsible(1, true);
    contentSplitter_->setStretchFactor(0, 1);
    contentSplitter_->setStretchFactor(1, 0);
    contentSplitter_->setSizes({720, lastHistoryPanelWidth_});
    connect(contentSplitter_, &QSplitter::splitterMoved, this, [this]() {
        if (!contentSplitter_) {
            return;
        }
        const QList<int> sizes = contentSplitter_->sizes();
        if (sizes.size() > 1 && sizes.at(1) > 0) {
            lastHistoryPanelWidth_ = sizes.at(1);
        }
    });

    setHistoryCollapsed(false);
    updateAdaptiveLayout();
    resultGroup_->setMinimumHeight(0);
    groupLayout->addWidget(contentSplitter_, 1);
}

void FrameAnalyzerWidget::onFormatClicked()
{
    const QString text = normalizeHexInput(inputEditor_->toPlainText());
    
    // Insert space every 2 chars
    QString formatted;
    for (int i = 0; i < text.length(); i += 2) {
        formatted.append(text.mid(i, 2));
        if (i + 2 < text.length()) formatted.append(" ");
    }
    
    inputEditor_->setPlainText(formatted.toUpper());
}

void FrameAnalyzerWidget::onClearClicked()
{
    ++latestParseRequestId_;
    parseInProgress_ = false;
    if (parseBtn_) {
        parseBtn_->setEnabled(true);
    }
    inputEditor_->clear();
    clearResult();
}

void FrameAnalyzerWidget::onHistorySelectionChanged(int row)
{
    if (row < 0 || row >= historyResults_.size()) {
        return;
    }
    currentResult_ = historyResults_.at(row);
    renderResult(currentResult_);
}

void FrameAnalyzerWidget::onClearHistoryClicked()
{
    historyResults_.clear();
    historyList_->clear();
}

void FrameAnalyzerWidget::onExportJsonClicked()
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export Frame Metadata"), "", tr("JSON Files (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }
    exportMetadataToJson(filePath);
}

void FrameAnalyzerWidget::onImportJsonClicked()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import Frame Metadata"), "", tr("JSON Files (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }
    importMetadataFromJson(filePath);
}

void FrameAnalyzerWidget::onExportCsvClicked()
{
    if (!currentResult_.isValid || !dataTable_ || dataTable_->rowCount() == 0) {
        QMessageBox::information(this, tr("No Data"), tr("There is no parsed data to export."));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Export CSV"),
        QString("frame_analysis_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        tr("CSV Files (*.csv)"));
    if (filePath.isEmpty()) {
        return;
    }

    exportCurrentTableToCsv(filePath);
}

void FrameAnalyzerWidget::exportMetadataToJson(const QString& filePath)
{
    QJsonObject root;
    root.insert("version", 1);
    // 规则：输入什么保存什么，如果为空则默认保存为 "0"
    QString startAddrStr = startAddrEdit_->text().trimmed();
    if (startAddrStr.isEmpty()) {
        startAddrStr = "0";
    }
    root.insert("startAddress", startAddrStr);
    root.insert("displayMode", displayMode_ == NumberDisplayMode::Signed ? "signed" : "unsigned");
    QJsonArray items;
    for (auto it = metadataByAddress_.cbegin(); it != metadataByAddress_.cend(); ++it) {
        QJsonObject item;
        item.insert("address", static_cast<int>(it.key()));
        item.insert("description", it.value().description);
        item.insert("scale", it.value().scale);
        items.append(item);
    }
    root.insert("items", items);
    const QByteArray jsonBytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    auto errorMessage = std::make_shared<QString>();
    auto* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, errorMessage]() {
        if (!errorMessage->isEmpty()) {
            QMessageBox::warning(this, tr("Export Failed"), *errorMessage);
        }
    });
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QObject::deleteLater);
    watcher->setFuture(QtConcurrent::run([filePath, jsonBytes, errorMessage]() {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            *errorMessage = QObject::tr("Cannot write file: %1").arg(filePath);
            return;
        }
        if (file.write(jsonBytes) != jsonBytes.size()) {
            *errorMessage = QObject::tr("Cannot write file: %1").arg(filePath);
        }
    }));
}

void FrameAnalyzerWidget::importMetadataFromJson(const QString& filePath)
{
    auto parsedRoot = std::make_shared<QJsonObject>();
    auto errorMessage = std::make_shared<QString>();
    auto* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, parsedRoot, errorMessage]() {
        if (!errorMessage->isEmpty()) {
            QMessageBox::warning(this, tr("Import Failed"), *errorMessage);
            return;
        }

        const QJsonObject& root = *parsedRoot;
        if (root.contains("startAddress") && startAddrEdit_) {
            const QJsonValue addrVal = root.value("startAddress");
            if (addrVal.isString()) {
                startAddrEdit_->setText(addrVal.toString());
            } else if (addrVal.isDouble()) {
                // 兼容旧版数字格式
                startAddrEdit_->setText(QString::number(addrVal.toInt()));
            }
        }
        if (root.contains("displayMode") && displayModeCombo_) {
            const QString mode = root.value("displayMode").toString().trimmed().toLower();
            const int index = mode == "signed" ? 1 : 0;
            displayModeCombo_->setCurrentIndex(index);
        }

        metadataByAddress_.clear();
        const QJsonArray items = root.value("items").toArray();
        for (const QJsonValue& value : items) {
            if (!value.isObject()) {
                continue;
            }
            const QJsonObject item = value.toObject();
            const int addressValue = item.value("address").toInt(-1);
            if (addressValue < 0 || addressValue > 65535) {
                continue;
            }
            const uint16_t address = static_cast<uint16_t>(addressValue);
            DataMetadata meta;
            meta.description = item.value("description").toString();
            meta.scale = item.value("scale").toDouble(1.0);
            metadataByAddress_.insert(address, meta);
        }
        if (!inputEditor_->toPlainText().trimmed().isEmpty()) {
            onParseClicked();
        } else {
            clearResult();
        }
    });
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QObject::deleteLater);
    watcher->setFuture(QtConcurrent::run([filePath, parsedRoot, errorMessage]() {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            *errorMessage = QObject::tr("Cannot open file: %1").arg(filePath);
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject()) {
            *errorMessage = QObject::tr("Invalid JSON format.");
            return;
        }
        *parsedRoot = doc.object();
    }));
}

void FrameAnalyzerWidget::clearResult()
{
    statusLabel_->setText(tr("Ready"));
    statusLabel_->setStyleSheet("color: gray;");
    
    // When stopping live mode, restore the structure parsing for the last received frame
    const bool wasLive = isLiveMode_;
    isLiveMode_ = false;
    isLivePaused_ = false;

    liveLabel_->setVisible(false);
    linkageStopBtn_->setVisible(false);
    if (linkageTipLabel_) {
        linkageTipLabel_->setVisible(false);
    }
    if (linkagePauseBtn_) {
        linkagePauseBtn_->setVisible(false);
        linkagePauseBtn_->setText(tr("Pause Refresh"));
        linkagePauseBtn_->setStyleSheet("");
    }
    
    if (registerOrderCombo_) {
        registerOrderCombo_->setEnabled(true);
        registerOrderCombo_->setToolTip(tr("Select the register byte/word order for diagnostic analysis."));
    }

    if (wasLive && lastLiveResult_.isValid) {
        renderResult(lastLiveResult_);
    }

    if (resultTabs_) {
        resultTabs_->setTabText(0, tr("Structure"));
    }
}

void FrameAnalyzerWidget::processLivePdu(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr) {
    isLiveMode_ = true;
    if (resultTabs_) {
        resultTabs_->setTabText(0, tr("Structure (Unavailable in Live Mode)"));
    }
    if (parseInProgress_) return; // Avoid collision if manual parse is running
    
    ParseResult result;
    result.isValid = true;
    result.protocol = protocol;
    result.timestamp = QDateTime::currentDateTime();
    result.functionCode = pdu.functionCode();
    result.isException = pdu.isException();
    result.exceptionCode = pdu.exceptionCode();
    result.pduData = pdu.toByteArray();
    
    // We assume response for live linkage
    result.type = result.isException ? FrameType::Exception : FrameType::Response;
    
    // Parse the PDU details
    ModbusFrameParser::parsePdu(result, result.pduData, addr, 0);
    
    // Update UI without adding to history
    QString protocolStr = (protocol == ProtocolType::Tcp) ? QStringLiteral("TCP") : QStringLiteral("RTU");
    liveLabel_->setText(tr("LIVE: %1").arg(protocolStr));
    liveLabel_->setVisible(true);
    linkageStopBtn_->setVisible(true);
    if (linkagePauseBtn_) {
        linkagePauseBtn_->setVisible(true);
    }

    if (registerOrderCombo_) {
        registerOrderCombo_->setEnabled(false);
        registerOrderCombo_->setToolTip(tr("Register order analysis is not available in live linkage mode."));
    }
    
    statusLabel_->setText(tr("Live Data Received at %1").arg(result.timestamp.toString("HH:mm:ss.zzz")));
    statusLabel_->setStyleSheet("color: #10B981; font-weight: bold;");
    
    if (linkageTipLabel_) {
        linkageTipLabel_->setVisible(!isLivePaused_);
    }
    
    lastLiveResult_ = result;
    if (!isLivePaused_) {
        renderResult(result);
    }
}

uint16_t FrameAnalyzerWidget::rowAddress(int row) const
{
    if (!dataTable_ || row < 0 || row >= dataTable_->rowCount()) {
        return 0;
    }
    const QTableWidgetItem* addrItem = dataTable_->item(row, 0);
    if (!addrItem) {
        return 0;
    }
    const QVariant data = addrItem->data(Qt::UserRole);
    return data.isValid() ? static_cast<uint16_t>(data.toUInt()) : 0;
}

void FrameAnalyzerWidget::onDataTableItemChanged(QTableWidgetItem* item)
{
    if (isUpdatingDataTable_ || !item) {
        return;
    }
    const int col = item->column();
    if (col != 4 && col != 6) {
        return;
    }
    const uint16_t address = rowAddress(item->row());
    DataMetadata meta = metadataByAddress_.value(address);
    if (col == 4) {
        bool ok = false;
        const double parsedScale = item->text().toDouble(&ok);
        if (!ok) {
            QSignalBlocker blocker(dataTable_);
            item->setText(QString::number(meta.scale, 'g', 12));
            return;
        }
        meta.scale = parsedScale;
    } else if (col == 6) {
        meta.description = item->text();
    }
    metadataByAddress_.insert(address, meta);
    QTableWidgetItem* decItem = dataTable_->item(item->row(), 2);
    QVariant value;
    if (decItem && decItem->text() != "-") {
        value = decItem->text().toDouble();
    }
    applyMetadataToRow(item->row(), value, meta);
}

void FrameAnalyzerWidget::onParseClicked()
{
    clearResult();
    const QString hexStr = normalizeHexInput(inputEditor_->toPlainText());
    if (hexStr.isEmpty()) {
        statusLabel_->setText(tr("Error: Empty input"));
        statusLabel_->setStyleSheet("color: red;");
        return;
    }
    
    ProtocolType type = protocolCombo_->currentData().value<ProtocolType>();
    bool addrOk = false;
    int addrVal = modbus::base::ModbusDataHelper::parseSmartInt(startAddrEdit_->text(), &addrOk);
    if (!addrOk || addrVal < 0 || addrVal > 65535) {
        statusLabel_->setText(tr("Invalid Address format or range (0-65535): %1").arg(startAddrEdit_->text()));
        statusLabel_->setStyleSheet("color: red;");
        return;
    }
    uint16_t startAddr = static_cast<uint16_t>(addrVal);

    ++latestParseRequestId_;
    parseInProgress_ = true;
    statusLabel_->setText(tr("Parsing..."));
    statusLabel_->setStyleSheet("color: #CC7A00; font-weight: bold;");
    if (parseBtn_) {
        parseBtn_->setEnabled(false);
    }
    emit parseRequested(hexStr, type, startAddr, registerOrder_, latestParseRequestId_);
}

void FrameAnalyzerWidget::loadSettings()
{
    if (!settingsService_) {
        return;
    }
    if (startAddrEdit_) {
        QSignalBlocker startAddrBlocker(startAddrEdit_);
        QString startAddr = settingsService_->value(common::settings_keys::kFrameAnalyzerStartAddr).toString();
        // Fallback for old int-based settings if string is empty
        if (startAddr.isEmpty()) {
            QVariant v = settingsService_->value(common::settings_keys::kFrameAnalyzerStartAddr);
            if (v.isValid()) {
                startAddr = QString::number(v.toInt());
            } else {
                startAddr = QString::number(app::constants::Values::Modbus::kDefaultStandardStartAddress);
            }
        }
        startAddrEdit_->setText(startAddr);
    }
}

void FrameAnalyzerWidget::saveSettings()
{
    if (!settingsService_) {
        return;
    }
    settingsService_->setValue(common::settings_keys::kFrameAnalyzerStartAddr, startAddrEdit_->text());
}

void FrameAnalyzerWidget::renderResult(const ParseResult& result)
{
    if (overviewTree_) {
        overviewTree_->clear();
        if (isLiveMode_) {
             auto root = new QTreeWidgetItem(overviewTree_, {tr("Structure"), tr("(Unavailable in Live Mode)"), tr("Logical parsing is disabled for high-frequency linkage")});
             root->setExpanded(true);
             // Skip detailed structure parsing
        }
    }
    
    
    auto addItem = [](QTreeWidgetItem* parent, const QString& field, const QString& value, const QString& description = QString()) {
        return new QTreeWidgetItem(parent, {field, value, description});
    };
    auto normalize = [](QString text) {
        return text.simplified();
    };
    auto buildDescription = [&](const QStringList& parts) {
        QStringList uniqueParts;
        for (const QString& part : parts) {
            const QString trimmed = part.trimmed();
            if (trimmed.isEmpty()) {
                continue;
            }
            bool duplicated = false;
            const QString normalizedPart = normalize(trimmed);
            for (const QString& existing : uniqueParts) {
                if (normalize(existing) == normalizedPart) {
                    duplicated = true;
                    break;
                }
            }
            if (!duplicated) {
                uniqueParts << trimmed;
            }
        }
        return uniqueParts.join(" | ");
    };

    if (!isLiveMode_) {
        auto protocolText = result.protocol == ProtocolType::Tcp ? tr("TCP") : tr("RTU");
        auto typeText =
            result.type == FrameType::Request ? tr("Request") :
            result.type == FrameType::Response ? tr("Response") :
            result.type == FrameType::Exception ? tr("Exception") :
            tr("Unknown");

        if (!result.isValid) {
            statusLabel_->setText(tr("Parse Failed: %1").arg(result.error));
            statusLabel_->setStyleSheet("color: red; font-weight: bold;");

        QTreeWidgetItem* root = new QTreeWidgetItem(overviewTree_);
        root->setText(0, tr("Frame"));
        root->setText(1, result.rawFrame.isEmpty() ? tr("Unknown") : tr("%1 bytes").arg(result.rawFrame.size()));
        root->setText(2, tr("Parse error"));

        if (!result.rawFrame.isEmpty()) {
            addItem(root, tr("Frame Bytes"), byteHex(result.rawFrame), tr("Complete raw frame"));
        }
        addItem(root, tr("Protocol"), result.protocol == ProtocolType::Unknown ? tr("Unknown") : protocolText, tr("Protocol detected before parse failed"));
        addItem(root, tr("Error"), result.error, tr("Detailed parse failure reason"));
        if (result.protocol == ProtocolType::Rtu && result.checksum != 0) {
            addItem(root,
                    tr("CRC16"),
                    QString("0x%1").arg(QString("%1").arg(result.checksum, 4, 16, QChar('0')).toUpper()),
                    result.checksumValid ? tr("CRC valid") : tr("CRC invalid"));
        }
        overviewTree_->expandAll();
        return;
    }

    QString statusText = tr("Success (%1)").arg(result.protocol == ProtocolType::Tcp ? tr("TCP") : tr("RTU"));
    if (result.isForced) {
        statusText += " [" + tr("Forced Parsing") + "]";
    }
    if (!result.warnings.isEmpty()) {
        statusText += " (" + tr("Warnings") + ")";
    }
    statusLabel_->setText(statusText);
    statusLabel_->setStyleSheet(result.warnings.isEmpty() ? "color: green; font-weight: bold;" : "color: #CC7A00; font-weight: bold;");

    const QString functionCodeHex = QString("0x%1")
                                        .arg(static_cast<int>(result.functionCode), 2, 16, QChar('0'))
                                        .toUpper();

    QTreeWidgetItem* root = new QTreeWidgetItem(overviewTree_);
    root->setText(0, tr("Frame"));
    root->setText(1, tr("%1 bytes").arg(result.rawFrame.size()));
    root->setText(2, buildDescription({protocolText, typeText}));

    if (!result.warnings.isEmpty()) {
        QTreeWidgetItem* warnRoot = new QTreeWidgetItem(overviewTree_);
        warnRoot->setText(0, tr("Warnings"));
        warnRoot->setText(2, tr("Issues ignored during forced parsing"));
        warnRoot->setForeground(0, QColor(200, 100, 0));
        warnRoot->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxWarning));
        for (const QString& warn : result.warnings) {
            auto* item = addItem(warnRoot, tr("Issue"), warn);
            item->setForeground(1, QColor(200, 100, 0));
        }
        warnRoot->setExpanded(true);
    }

    addItem(root, tr("Frame Bytes"), byteHex(result.rawFrame), tr("Complete raw frame"));

    if (result.protocol == ProtocolType::Tcp) {
        QTreeWidgetItem* mbapItem = addItem(root, tr("MBAP Header"), byteHex(result.rawFrame.left(7)), tr("Transaction + Protocol + Length + Unit ID"));
        addItem(mbapItem,
                tr("Transaction ID"),
                QString("%1 (%2)").arg(result.transactionId).arg(byteHex(result.rawFrame.mid(0, 2))),
                tr("Request/response correlation ID"));
        addItem(mbapItem,
                tr("Protocol ID"),
                QString("%1 (%2)").arg(result.protocolId).arg(byteHex(result.rawFrame.mid(2, 2))),
                tr("Modbus TCP protocol identifier"));
        addItem(mbapItem,
                tr("Length"),
                QString("%1 (%2)").arg(result.length).arg(byteHex(result.rawFrame.mid(4, 2))),
                tr("Remaining bytes after this field"));
        addItem(mbapItem,
                tr("Unit ID"),
                QString("%1 (%2)").arg(result.slaveId).arg(byteHex(result.rawFrame.mid(6, 1))),
                tr("Target slave / unit address"));
    } else {
        addItem(root,
                tr("Slave ID"),
                QString("%1 (%2)").arg(result.slaveId).arg(byteHex(result.rawFrame.mid(0, 1))),
                tr("Target slave address"));
    }

    const int pduOffset = result.protocol == ProtocolType::Tcp ? 7 : 1;
    const int pduLength = result.protocol == ProtocolType::Tcp ? qMax(0, result.rawFrame.size() - 7) : qMax(0, result.rawFrame.size() - 3);
    const QByteArray pduBytes = result.rawFrame.mid(pduOffset, pduLength);
    QTreeWidgetItem* pduItem = addItem(root, tr("PDU"), pduBytes.isEmpty() ? tr("(empty)") : byteHex(pduBytes), tr("Function code + payload"));
    addItem(pduItem,
            tr("Function Code"),
            QString("%1 (%2)").arg(functionCodeHex).arg(byteHex(result.rawFrame.mid(pduOffset, 1))),
            buildDescription({typeText, result.isException ? tr("Exception response") : tr("Normal response")}));

    const QByteArray payloadBytes = pduBytes.size() > 1 ? pduBytes.mid(1) : QByteArray();
    addItem(pduItem,
            tr("Payload"),
            payloadBytes.isEmpty() ? tr("(empty)") : byteHex(payloadBytes),
            result.isException ? tr("Exception detail payload") : tr("Application data payload"));

    if (result.isException) {
        addItem(pduItem,
                tr("Exception Code"),
                QString("0x%1").arg(static_cast<int>(result.exceptionCode), 2, 16, QChar('0')).toUpper(),
                buildDescription({result.error, tr("Exception detail payload")}));
    }

    if (result.protocol == ProtocolType::Rtu) {
        const QByteArray crcBytes = result.rawFrame.right(2);
        QString crcDescription = result.checksumValid ? tr("CRC valid") : tr("CRC invalid");
        if (!result.checksumValid) {
            crcDescription = buildDescription({
                crcDescription,
                tr("Calculated: 0x%1").arg(QString("%1").arg(result.calculatedChecksum, 4, 16, QChar('0')).toUpper())
            });
        }
        auto* crcItem = addItem(root,
                                tr("CRC16"),
                                QString("0x%1 (%2)").arg(QString("%1").arg(result.checksum, 4, 16, QChar('0')).toUpper()).arg(byteHex(crcBytes)),
                                crcDescription);
        if (!result.checksumValid) {
            crcItem->setForeground(2, Qt::red);
        }
    }

        overviewTree_->expandAll();
    }

    isUpdatingDataTable_ = true;
    const int newCount = result.dataItems.size();
    if (dataTable_->rowCount() != newCount) {
        dataTable_->setRowCount(newCount);
    }

    auto updateItem = [this](int r, int c, const QString& text, bool editable) {
        QTableWidgetItem* it = dataTable_->item(r, c);
        if (!it) {
            it = new QTableWidgetItem(text);
            if (!editable) {
                it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            }
            dataTable_->setItem(r, c, it);
        } else {
            // Safety check: Don't overwrite if the item is being edited
            bool isBeingEdited = false;
            QWidget* fw = QApplication::focusWidget();
            if (fw && dataTable_->viewport() && dataTable_->viewport()->isAncestorOf(fw)) {
                QModelIndex idx = dataTable_->currentIndex();
                if (idx.isValid() && idx.row() == r && idx.column() == c) {
                    isBeingEdited = true;
                }
            }
            
            if (!isBeingEdited && it->text() != text) {
                it->setText(text);
            }
        }
        return it;
    };

    for (int i = 0; i < result.dataItems.size(); ++i) {
        const auto& item = result.dataItems[i];
        DataMetadata meta = metadataByAddress_.value(item.address);
        
        // 0: Address - Display as "DEC (0xHEX)" but store numeric value in UserRole
        const QString addrText = QString("%1 (0x%2)")
                                     .arg(item.address)
                                     .arg(QString::number(item.address, 16).toUpper().rightJustified(4, '0'));
        auto* addrItem = updateItem(i, 0, addrText, false);
        addrItem->setData(Qt::UserRole, item.address);

        // 1: Hex
        updateItem(i, 1, formatHexValue(item.rawBytes, item.hexString), false);
        
        // 2: Decimal
        QString decStr = "-";
        QColor textColor = Qt::black;
        if (item.value.isValid()) {
            if (item.value.typeId() == QMetaType::Bool) {
                textColor = item.value.toBool() ? Qt::darkGreen : Qt::darkRed;
            } else if (item.value.typeId() == QMetaType::Short || item.value.typeId() == QMetaType::Int) {
                textColor = (item.value.toInt() < 0) ? QColor(180, 30, 30) : Qt::black;
            }
            decStr = formatDecimalValue(item.value);
        }
        auto* decItem = updateItem(i, 2, decStr, false);
        decItem->setForeground(textColor);

        // 3: Binary
        updateItem(i, 3, formatBinaryValue(item.rawBytes, item.binaryString), false);

        // 4: Scale
        updateItem(i, 4, QString::number(meta.scale, 'g', 12), true);

        // 5: Value
        updateItem(i, 5, formatScaledValue(item.value, meta), false);

        // 6: Description
        updateItem(i, 6, meta.description, true);

        metadataByAddress_.insert(item.address, meta);
        applyMetadataToRow(i, item.value, meta);
    }
    isUpdatingDataTable_ = false;
}

void FrameAnalyzerWidget::onParseFinished(const ParseResult& result, quint64 requestId)
{
    if (requestId != latestParseRequestId_) {
        return;
    }

    parseInProgress_ = false;
    if (parseBtn_) {
        parseBtn_->setEnabled(true);
    }

    currentResult_ = result;
    renderResult(result);
    if (result.isValid) {
        addToHistory(result);
    }
}

void FrameAnalyzerWidget::exportCurrentTableToCsv(const QString& filePath) const
{
    auto errorMessage = std::make_shared<QString>();
    struct SaveState {
        int nextRow = -1; // -1 means header row is pending
        bool firstChunk = true;
    };
    auto state = std::make_shared<SaveState>();
    const int totalRows = dataTable_->rowCount();
    const int chunkSize = app::constants::Values::Ui::kFrameAnalyzerCsvExportChunkRows;
    auto scheduleNextChunk = std::make_shared<std::function<void()>>();

    *scheduleNextChunk = [this, filePath, totalRows, chunkSize, state, errorMessage, scheduleNextChunk]() {
        if (!errorMessage->isEmpty()) {
            QMessageBox::warning(const_cast<FrameAnalyzerWidget*>(this), tr("Export Failed"), *errorMessage);
            return;
        }
        if (state->nextRow >= totalRows) {
            return;
        }

        QStringList lines;
        if (state->nextRow < 0) {
            QStringList headerValues;
            for (int column = 0; column < dataTable_->columnCount(); ++column) {
                const QTableWidgetItem* headerItem = dataTable_->horizontalHeaderItem(column);
                headerValues << escapeCsvValue(headerItem ? headerItem->text() : QString());
            }
            lines << headerValues.join(',');
            state->nextRow = 0;
        }

        const int begin = state->nextRow;
        const int end = qMin(begin + chunkSize, totalRows);
        for (int row = begin; row < end; ++row) {
            QStringList rowValues;
            for (int column = 0; column < dataTable_->columnCount(); ++column) {
                const QTableWidgetItem* item = dataTable_->item(row, column);
                rowValues << escapeCsvValue(item ? item->text() : QString());
            }
            lines << rowValues.join(',');
        }
        state->nextRow = end;
        const bool firstChunk = state->firstChunk;
        state->firstChunk = false;

        auto* watcher = new QFutureWatcher<void>(const_cast<FrameAnalyzerWidget*>(this));
        connect(watcher, &QFutureWatcher<void>::finished, const_cast<FrameAnalyzerWidget*>(this), [this, errorMessage, scheduleNextChunk, watcher]() {
            watcher->deleteLater();
            if (!errorMessage->isEmpty()) {
                QMessageBox::warning(const_cast<FrameAnalyzerWidget*>(this), tr("Export Failed"), *errorMessage);
                return;
            }
            QMetaObject::invokeMethod(const_cast<FrameAnalyzerWidget*>(this), [scheduleNextChunk]() {
                (*scheduleNextChunk)();
            }, Qt::QueuedConnection);
        });
        watcher->setFuture(QtConcurrent::run([filePath, lines, firstChunk, errorMessage]() {
            QFile file(filePath);
            QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
            mode |= firstChunk ? QIODevice::Truncate : QIODevice::Append;
            if (!file.open(mode)) {
                *errorMessage = QObject::tr("Cannot write file: %1").arg(filePath);
                return;
            }

            QTextStream stream(&file);
            if (firstChunk) {
                stream.setGenerateByteOrderMark(true);
            }
            for (const QString& line : lines) {
                stream << line << '\n';
            }
        }));
    };

    (*scheduleNextChunk)();
}

QString FrameAnalyzerWidget::escapeCsvValue(const QString& value) const
{
    QString escaped = value;
    escaped.replace('"', "\"\"");
    return QString("\"%1\"").arg(escaped);
}

void FrameAnalyzerWidget::addToHistory(const ParseResult& result)
{
    historyResults_.prepend(result);
    while (historyResults_.size() > app::constants::Values::Ui::kFrameAnalyzerMaxHistoryItems) {
        historyResults_.removeLast();
    }
    refreshHistoryList();
}

void FrameAnalyzerWidget::refreshHistoryList()
{
    if (!historyList_) {
        return;
    }

    const QSignalBlocker blocker(historyList_);
    historyList_->clear();
    for (const ParseResult& result : historyResults_) {
        historyList_->addItem(historyItemText(result));
    }
    if (!historyResults_.isEmpty()) {
        historyList_->setCurrentRow(0);
    }
}

QString FrameAnalyzerWidget::historyItemText(const ParseResult& result) const
{
    const QString protocolText = result.protocol == ProtocolType::Tcp ? tr("TCP") : tr("RTU");
    const QString functionCodeText = QString("0x%1")
                                         .arg(static_cast<int>(result.functionCode), 2, 16, QChar('0'))
                                         .toUpper();
    const QString typeText =
        result.type == FrameType::Request ? tr("Request") :
        result.type == FrameType::Response ? tr("Response") :
        result.type == FrameType::Exception ? tr("Exception") :
        tr("Unknown");
    return QString("[%1] %2 %3 %4")
        .arg(result.timestamp.toString("hh:mm:ss"))
        .arg(protocolText)
        .arg(functionCodeText)
        .arg(typeText);
}

void FrameAnalyzerWidget::setHistoryCollapsed(bool collapsed)
{
    historyCollapsed_ = collapsed;
    if (contentSplitter_ && historyGroup_) {
        if (collapsed) {
            const QList<int> sizes = contentSplitter_->sizes();
            if (sizes.size() > 1 && sizes.at(1) > 0) {
                lastHistoryPanelWidth_ = sizes.at(1);
            }
            historyGroup_->hide();
            contentSplitter_->setSizes({1, 0});
        } else {
            historyGroup_->show();
            const QList<int> sizes = contentSplitter_->sizes();
            int totalWidth = 0;
            for (int size : sizes) {
                totalWidth += size;
            }
            totalWidth = qMax(totalWidth, resultGroup_ ? resultGroup_->width() : 0);
            const int historyWidth = qMax(120, lastHistoryPanelWidth_);
            contentSplitter_->setSizes({qMax(0, totalWidth - historyWidth), historyWidth});
        }
    } else if (historyGroup_) {
        historyGroup_->setVisible(!collapsed);
    }
    updateHistoryToggleText();
}

void FrameAnalyzerWidget::updateAdaptiveLayout()
{
    if (!contentSplitter_ || !historyGroup_) {
        return;
    }

    const bool shouldAutoCollapse = width() < app::constants::Values::Ui::kFrameAnalyzerAdaptiveHistoryCollapseWidth;
    if (shouldAutoCollapse) {
        if (!historyCollapsed_) {
            historyAutoCollapsed_ = true;
            setHistoryCollapsed(true);
        }
        return;
    }

    if (historyAutoCollapsed_) {
        historyAutoCollapsed_ = false;
        if (historyCollapsed_) {
            setHistoryCollapsed(false);
        }
    }
}

void FrameAnalyzerWidget::updateHistoryToggleText()
{
    if (toggleHistoryBtn_) {
        toggleHistoryBtn_->setText(historyCollapsed_ ? tr("Show History") : tr("Hide History"));
    }
    if (linkageStopBtn_) {
        linkageStopBtn_->setText(tr("Stop Link"));
    }
    if (linkagePauseBtn_) {
        linkagePauseBtn_->setText(isLivePaused_ ? tr("Resume Refresh") : tr("Pause Refresh"));
    }
}

void FrameAnalyzerWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void FrameAnalyzerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateAdaptiveLayout();
}

void FrameAnalyzerWidget::retranslateUi()
{
    if (inputGroup_) {
        inputGroup_->setTitle(tr("Frame Input"));
    }
    if (protocolLabel_) {
        protocolLabel_->setText(tr("Protocol:"));
    }
    if (protocolCombo_) {
        protocolCombo_->setItemText(0, tr("Auto Detect"));
        protocolCombo_->setItemText(1, tr("Modbus TCP"));
        protocolCombo_->setItemText(2, tr("Modbus RTU"));
    }
    if (startAddrLabel_) {
        startAddrLabel_->setText(tr("Start Address (for Response):"));
    }
    if (startAddrEdit_) {
        startAddrEdit_->setToolTip(tr("Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16)."));
    }
    if (displayModeLabel_) {
        displayModeLabel_->setText(tr("Decode Mode:"));
    }
    if (displayModeCombo_) {
        displayModeCombo_->setItemText(0, tr("Unsigned"));
        displayModeCombo_->setItemText(1, tr("Signed"));
        displayModeCombo_->setToolTip(tr("Choose how parsed numeric values are displayed."));
    }
    if (registerOrderLabel_) {
        registerOrderLabel_->setText(tr("Byte Order:"));
    }
    if (registerOrderCombo_) {
        registerOrderCombo_->setItemText(0, tr("ABCD(default)"));
        registerOrderCombo_->setItemText(1, "BADC");
        registerOrderCombo_->setItemText(2, "CDAB");
        registerOrderCombo_->setItemText(3, "DCBA");
        if (isLiveMode_) {
            registerOrderCombo_->setToolTip(tr("Register order analysis is not available in live linkage mode."));
        } else {
            registerOrderCombo_->setToolTip(tr("Select the register byte/word order for diagnostic analysis."));
        }
    }
    if (formatBtn_) {
        formatBtn_->setText(tr("Format Hex"));
    }
    if (importJsonBtn_) {
        importJsonBtn_->setText(tr("Import Config"));
        importJsonBtn_->setToolTip(tr("Import saved field scale and description settings from a JSON file."));
    }
    if (exportJsonBtn_) {
        exportJsonBtn_->setText(tr("Export Config"));
        exportJsonBtn_->setToolTip(tr("Export current field scale and description settings to a JSON file."));
    }
    if (exportCsvBtn_) {
        exportCsvBtn_->setText(tr("Export CSV"));
        exportCsvBtn_->setToolTip(tr("Export the parsed register data in the current table as a CSV file."));
    }
    if (toggleHistoryBtn_) {
        toggleHistoryBtn_->setToolTip(tr("Show or hide the parse history panel."));
    }
    updateHistoryToggleText();
    if (linkageTipLabel_) {
        linkageTipLabel_->setText(tr("Tip: \"Pause\" to edit description"));
    }
    if (parseBtn_) {
        parseBtn_->setText(tr("Parse"));
        parseBtn_->setEnabled(!parseInProgress_);
    }
    if (clearBtn_) {
        clearBtn_->setText(tr("Clear"));
    }
    if (inputEditor_) {
        inputEditor_->setPlaceholderText(tr("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)"));
    }

    if (resultGroup_) {
        resultGroup_->setTitle(tr("Analysis Result"));
    }
    if (historyGroup_) {
        historyGroup_->setTitle(tr("History"));
    }
    if (resultTabs_) {
        if (isLiveMode_) {
            resultTabs_->setTabText(0, tr("Structure (Unavailable in Live Mode)"));
        } else {
            resultTabs_->setTabText(0, tr("Structure"));
        }
        resultTabs_->setTabText(1, tr("Data Details"));
    }
    if (overviewTree_) {
        QTreeWidgetItem* header = overviewTree_->headerItem();
        header->setText(0, tr("Field"));
        header->setText(1, tr("Value"));
        header->setText(2, tr("Description"));
    }
    if (dataTable_) {
        dataTable_->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Scale"), tr("Value"), tr("Description")});
    }
    if (clearHistoryBtn_) {
        clearHistoryBtn_->setText(tr("Clear History"));
    }
    if (historyList_) {
        refreshHistoryList();
    }

    if (isLiveMode_) {
        QString protocolStr = (currentResult_.protocol == ProtocolType::Tcp) ? tr("TCP") : tr("RTU");
        if (liveLabel_) {
            liveLabel_->setText(tr("LIVE: %1").arg(protocolStr));
        }
        if (statusLabel_) {
            statusLabel_->setText(tr("Live Data Received at %1").arg(currentResult_.timestamp.toString("HH:mm:ss.zzz")));
        }
    } else if (statusLabel_ && statusLabel_->styleSheet().contains("color: gray", Qt::CaseInsensitive)) {
        statusLabel_->setText(tr("Ready"));
    }
}

} // namespace ui::widgets

#include "FrameAnalyzerWidget.moc"
