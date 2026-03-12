#include "FrameAnalyzerWidget.h"
#include "modbus/parser/ModbusFrameParser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QMessageBox>
#include <QEvent>
#include <QSettings>
#include <QApplication>
#include <QSignalBlocker>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace ui::widgets {

using namespace modbus::core::parser;

namespace {
QString groupBits(const QString& bits)
{
    QString grouped = bits;
    for (int k = grouped.size() - 4; k > 0; k -= 4) {
        grouped.insert(k, ' ');
    }
    return grouped;
}

uint32_t maskForBits(int bitWidth)
{
    if (bitWidth >= 32) {
        return 0xFFFFFFFFu;
    }
    return (1u << bitWidth) - 1u;
}
}

FrameAnalyzerWidget::FrameAnalyzerWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

FrameAnalyzerWidget::~FrameAnalyzerWidget() = default;

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

QString FrameAnalyzerWidget::buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta) const
{
    QStringList lines;
    if (!meta.dataType.trimmed().isEmpty()) {
        lines << tr("Type: %1").arg(meta.dataType.trimmed());
    }
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
    QTableWidgetItem* scaleItem = dataTable_->item(row, 5);
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
    if (displayMode_ == NumberDisplayMode::Unsigned) {
        return QString("0x%1").arg(rawHex);
    }
    if (bitWidth == 16) {
        const int16_t signedValue = static_cast<int16_t>(masked);
        return QString("0x%1 (%2)").arg(rawHex).arg(signedValue);
    }
    if (bitWidth == 8) {
        const int8_t signedValue = static_cast<int8_t>(masked);
        return QString("0x%1 (%2)").arg(rawHex).arg(signedValue);
    }
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
    if (displayMode_ == NumberDisplayMode::Unsigned) {
        return bits;
    }
    if (bitWidth == 16) {
        const int16_t signedValue = static_cast<int16_t>(masked);
        return QString("%1 (%2)").arg(bits).arg(signedValue);
    }
    if (bitWidth == 8) {
        const int8_t signedValue = static_cast<int8_t>(masked);
        return QString("%1 (%2)").arg(bits).arg(signedValue);
    }
    return bits;
}

void FrameAnalyzerWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    createInputGroup();
    createResultGroup();
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
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
    startAddressSpin_ = new QSpinBox(this);
    startAddressSpin_->setRange(0, 65535);
    startAddressSpin_->setValue(0);
    controlsLayout->addWidget(startAddressSpin_);

    controlsLayout->addSpacing(20);
    displayModeLabel_ = new QLabel(tr("Decode Mode:"), this);
    controlsLayout->addWidget(displayModeLabel_);
    displayModeCombo_ = new QComboBox(this);
    displayModeCombo_->addItem(tr("Unsigned"), static_cast<int>(NumberDisplayMode::Unsigned));
    displayModeCombo_->addItem(tr("Signed"), static_cast<int>(NumberDisplayMode::Signed));
    displayModeCombo_->setCurrentIndex(0);
    controlsLayout->addWidget(displayModeCombo_);

    controlsLayout->addStretch();
    
    formatBtn_ = new QPushButton(tr("Format Hex"), this);
    connect(formatBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onFormatClicked);
    controlsLayout->addWidget(formatBtn_);

    importJsonBtn_ = new QPushButton(tr("Import JSON"), this);
    connect(importJsonBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onImportJsonClicked);
    controlsLayout->addWidget(importJsonBtn_);

    exportJsonBtn_ = new QPushButton(tr("Export JSON"), this);
    connect(exportJsonBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onExportJsonClicked);
    controlsLayout->addWidget(exportJsonBtn_);

    parseBtn_ = new QPushButton(tr("Parse"), this);
    connect(parseBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onParseClicked);
    controlsLayout->addWidget(parseBtn_);

    clearBtn_ = new QPushButton(tr("Clear"), this);
    connect(clearBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearClicked);
    controlsLayout->addWidget(clearBtn_);

    groupLayout->addLayout(controlsLayout);

    // Input Editor
    inputEditor_ = new QPlainTextEdit(this);
    inputEditor_->setPlaceholderText(tr("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)"));
    inputEditor_->setMaximumHeight(72);
    groupLayout->addWidget(inputEditor_);

    connect(startAddressSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &FrameAnalyzerWidget::saveSettings);
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

    layout()->addWidget(inputGroup_);
    loadSettings();
}

void FrameAnalyzerWidget::createResultGroup()
{
    resultGroup_ = new QGroupBox(tr("Analysis Result"), this);
    auto groupLayout = new QVBoxLayout(resultGroup_);

    statusLabel_ = new QLabel(tr("Ready"), this);
    statusLabel_->setStyleSheet("font-weight: bold; color: gray;");
    groupLayout->addWidget(statusLabel_);

    resultTabs_ = new QTabWidget(this);

    // Tab 1: Structure Overview
    overviewTree_ = new QTreeWidget(this);
    overviewTree_->setHeaderLabels({tr("Field"), tr("Value"), tr("Description")});
    overviewTree_->setColumnWidth(0, 200);
    overviewTree_->setColumnWidth(1, 150);
    resultTabs_->addTab(overviewTree_, tr("Structure"));

    // Tab 2: Decoded Data
    dataTable_ = new QTableWidget(this);
    dataTable_->setColumnCount(7);
    dataTable_->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Data Type"), tr("Scale"), tr("Description")});
    dataTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    dataTable_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    connect(dataTable_, &QTableWidget::itemChanged, this, &FrameAnalyzerWidget::onDataTableItemChanged);
    resultTabs_->addTab(dataTable_, tr("Data Details"));

    groupLayout->addWidget(resultTabs_);
    layout()->addWidget(resultGroup_);
}

void FrameAnalyzerWidget::onFormatClicked()
{
    QString text = inputEditor_->toPlainText();
    // Remove all non-hex chars
    text.remove(QRegularExpression("[^0-9A-Fa-f]"));
    
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
    inputEditor_->clear();
    clearResult();
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

void FrameAnalyzerWidget::exportMetadataToJson(const QString& filePath)
{
    QJsonObject root;
    root.insert("version", 1);
    root.insert("startAddress", startAddressSpin_ ? startAddressSpin_->value() : 0);
    root.insert("displayMode", displayMode_ == NumberDisplayMode::Signed ? "signed" : "unsigned");
    QJsonArray items;
    for (auto it = metadataByAddress_.cbegin(); it != metadataByAddress_.cend(); ++it) {
        QJsonObject item;
        item.insert("address", static_cast<int>(it.key()));
        item.insert("dataType", it.value().dataType);
        item.insert("description", it.value().description);
        item.insert("scale", it.value().scale);
        items.append(item);
    }
    root.insert("items", items);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write file: %1").arg(filePath));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

void FrameAnalyzerWidget::importMetadataFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Import Failed"), tr("Cannot open file: %1").arg(filePath));
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        QMessageBox::warning(this, tr("Import Failed"), tr("Invalid JSON format."));
        return;
    }
    const QJsonObject root = doc.object();
    if (root.contains("startAddress") && startAddressSpin_) {
        startAddressSpin_->setValue(root.value("startAddress").toInt(startAddressSpin_->value()));
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
        meta.dataType = item.value("dataType").toString();
        meta.description = item.value("description").toString();
        meta.scale = item.value("scale").toDouble(1.0);
        metadataByAddress_.insert(address, meta);
    }
    if (!inputEditor_->toPlainText().trimmed().isEmpty()) {
        onParseClicked();
    } else {
        clearResult();
    }
}

void FrameAnalyzerWidget::clearResult()
{
    statusLabel_->setText(tr("Ready"));
    statusLabel_->setStyleSheet("color: gray;");
    overviewTree_->clear();
    dataTable_->setRowCount(0);
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
    bool ok = false;
    const uint16_t address = static_cast<uint16_t>(addrItem->text().toUInt(&ok));
    return ok ? address : 0;
}

void FrameAnalyzerWidget::onDataTableItemChanged(QTableWidgetItem* item)
{
    if (isUpdatingDataTable_ || !item) {
        return;
    }
    const int col = item->column();
    if (col != 4 && col != 5 && col != 6) {
        return;
    }
    const uint16_t address = rowAddress(item->row());
    DataMetadata meta = metadataByAddress_.value(address);
    if (col == 4) {
        meta.dataType = item->text().trimmed();
    } else if (col == 5) {
        bool ok = false;
        const double parsedScale = item->text().toDouble(&ok);
        if (!ok) {
            QSignalBlocker blocker(dataTable_);
            item->setText(QString::number(meta.scale, 'g', 12));
            return;
        }
        meta.scale = parsedScale;
    } else {
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

    QString hexStr = inputEditor_->toPlainText().remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (hexStr.isEmpty()) {
        statusLabel_->setText(tr("Error: Empty input"));
        statusLabel_->setStyleSheet("color: red;");
        return;
    }

    QByteArray frame = QByteArray::fromHex(hexStr.toLatin1());
    
    ProtocolType type = protocolCombo_->currentData().value<ProtocolType>();
    uint16_t startAddr = static_cast<uint16_t>(startAddressSpin_->value());

    ParseResult result = ModbusFrameParser::parse(frame, type, startAddr);
    renderResult(result);
}

void FrameAnalyzerWidget::loadSettings()
{
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QSignalBlocker blocker(startAddressSpin_);
    const QString key = "frame_analyzer/startAddr";
    const int value = settings.value(key, startAddressSpin_->value()).toInt();
    startAddressSpin_->setValue(value);
    if (!settings.contains(key)) {
        settings.setValue(key, value);
    }
}

void FrameAnalyzerWidget::saveSettings()
{
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue("frame_analyzer/startAddr", startAddressSpin_->value());
}

void FrameAnalyzerWidget::renderResult(const ParseResult& result)
{
    if (!result.isValid) {
        statusLabel_->setText(tr("Parse Failed: %1").arg(result.error));
        statusLabel_->setStyleSheet("color: red; font-weight: bold;");
        return;
    }

    statusLabel_->setText(tr("Success (%1)").arg(result.protocol == ProtocolType::Tcp ? tr("TCP") : tr("RTU")));
    statusLabel_->setStyleSheet("color: green; font-weight: bold;");

    // 1. Populate Overview Tree
    QTreeWidgetItem* root = new QTreeWidgetItem(overviewTree_);
    root->setText(0, tr("Frame"));
    root->setText(1, tr("%1 bytes").arg(result.pduData.size() + (result.protocol == ProtocolType::Tcp ? 7 : 3))); // Approximate
    
    // Header Info
    if (result.protocol == ProtocolType::Tcp) {
        new QTreeWidgetItem(root, {tr("Transaction ID"), QString::number(result.transactionId), ""});
        new QTreeWidgetItem(root, {tr("Protocol ID"), QString::number(result.protocolId), ""});
        new QTreeWidgetItem(root, {tr("Length"), QString::number(result.length), ""});
        new QTreeWidgetItem(root, {tr("Unit ID"), QString::number(result.slaveId), ""});
    } else {
        new QTreeWidgetItem(root, {tr("Slave ID"), QString::number(result.slaveId), ""});
    }

    // Function Code
    new QTreeWidgetItem(root, {tr("Function Code"), QString::number((uint8_t)result.functionCode), result.isException ? tr("Exception") : tr("Normal")});

    // PDU Data
    if (result.isException) {
        new QTreeWidgetItem(root, {"Exception Code", QString::number((uint8_t)result.exceptionCode), result.error});
    }

    // Checksum (RTU)
    if (result.protocol == ProtocolType::Rtu) {
        auto crcItem = new QTreeWidgetItem(root, {"CRC16", QString("%1").arg(result.checksum, 4, 16, QChar('0')).toUpper(), result.checksumValid ? "Valid" : "Invalid"});
        if (!result.checksumValid) {
            crcItem->setForeground(2, Qt::red);
            crcItem->setText(2, QString("Invalid (Calc: %1)").arg(QString::number(result.calculatedChecksum, 16).toUpper()));
        }
    }

    overviewTree_->expandAll();

    // 2. Populate Data Table
    isUpdatingDataTable_ = true;
    dataTable_->setRowCount(result.dataItems.size());
    for (int i = 0; i < result.dataItems.size(); ++i) {
        const auto& item = result.dataItems[i];
        DataMetadata meta = metadataByAddress_.value(item.address);
        
        auto* addressItem = new QTableWidgetItem(QString::number(item.address));
        addressItem->setFlags(addressItem->flags() & ~Qt::ItemIsEditable);
        dataTable_->setItem(i, 0, addressItem);

        auto* hexItem = new QTableWidgetItem(formatHexValue(item.rawBytes, item.hexString));
        hexItem->setFlags(hexItem->flags() & ~Qt::ItemIsEditable);
        dataTable_->setItem(i, 1, hexItem);
        
        // Decimal (Smart display)
        QString decStr = "-";
        QColor textColor = Qt::black;
        
        if (item.value.isValid()) {
            if (item.value.typeId() == QMetaType::Bool) {
                const bool val = item.value.toBool();
                textColor = val ? Qt::darkGreen : Qt::darkRed;
            } else if (item.value.typeId() == QMetaType::Short || item.value.typeId() == QMetaType::Int) {
                const int numeric = item.value.toInt();
                textColor = numeric < 0 ? QColor(180, 30, 30) : Qt::black;
            }
            decStr = formatDecimalValue(item.value);
        }
        auto decItem = new QTableWidgetItem(decStr);
        decItem->setForeground(textColor);
        decItem->setFlags(decItem->flags() & ~Qt::ItemIsEditable);
        dataTable_->setItem(i, 2, decItem);

        auto* binaryItem = new QTableWidgetItem(formatBinaryValue(item.rawBytes, item.binaryString));
        binaryItem->setFlags(binaryItem->flags() & ~Qt::ItemIsEditable);
        dataTable_->setItem(i, 3, binaryItem);

        auto* dataTypeItem = new QTableWidgetItem(meta.dataType);
        dataTable_->setItem(i, 4, dataTypeItem);

        auto* scaleItem = new QTableWidgetItem(QString::number(meta.scale, 'g', 12));
        dataTable_->setItem(i, 5, scaleItem);

        auto* descItem = new QTableWidgetItem(meta.description);
        dataTable_->setItem(i, 6, descItem);

        metadataByAddress_.insert(item.address, meta);
        applyMetadataToRow(i, item.value, meta);
    }
    isUpdatingDataTable_ = false;
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
    if (displayModeLabel_) {
        displayModeLabel_->setText(tr("Decode Mode:"));
    }
    if (displayModeCombo_) {
        displayModeCombo_->setItemText(0, tr("Unsigned"));
        displayModeCombo_->setItemText(1, tr("Signed"));
    }
    if (formatBtn_) {
        formatBtn_->setText(tr("Format Hex"));
    }
    if (importJsonBtn_) {
        importJsonBtn_->setText(tr("Import JSON"));
    }
    if (exportJsonBtn_) {
        exportJsonBtn_->setText(tr("Export JSON"));
    }
    if (parseBtn_) {
        parseBtn_->setText(tr("Parse"));
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
    if (resultTabs_) {
        resultTabs_->setTabText(0, tr("Structure"));
        resultTabs_->setTabText(1, tr("Data Details"));
    }
    if (overviewTree_) {
        QTreeWidgetItem* header = overviewTree_->headerItem();
        header->setText(0, tr("Field"));
        header->setText(1, tr("Value"));
        header->setText(2, tr("Description"));
    }
    if (dataTable_) {
        dataTable_->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Data Type"), tr("Scale"), tr("Description")});
    }

    // Refresh result text if needed (simple approach: clear or keep existing static text)
    // statusLabel_ text is dynamic, so we might leave it or reset to Ready if empty
    if (statusLabel_ && statusLabel_->text() == "Ready") { // Basic check, ideally track state
         statusLabel_->setText(tr("Ready"));
    }
}

} // namespace ui::widgets
