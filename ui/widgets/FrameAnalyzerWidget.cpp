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

namespace ui::widgets {

using namespace modbus::core::parser;

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

QString FrameAnalyzerWidget::formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex) const
{
    Q_UNUSED(rawBytes);
    return fallbackHex;
}

QString FrameAnalyzerWidget::formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary) const
{
    Q_UNUSED(rawBytes);
    return fallbackBinary;
}

void FrameAnalyzerWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    createInputGroup();
    createResultGroup();
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
    inputEditor_->setMaximumHeight(100);
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
    dataTable_->setColumnCount(5);
    dataTable_->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Description")});
    dataTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    dataTable_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); // Description stretches
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

void FrameAnalyzerWidget::clearResult()
{
    statusLabel_->setText(tr("Ready"));
    statusLabel_->setStyleSheet("color: gray;");
    overviewTree_->clear();
    dataTable_->setRowCount(0);
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
    dataTable_->setRowCount(result.dataItems.size());
    for (int i = 0; i < result.dataItems.size(); ++i) {
        const auto& item = result.dataItems[i];
        
        // Address
        dataTable_->setItem(i, 0, new QTableWidgetItem(QString::number(item.address)));
        
        // Hex
        dataTable_->setItem(i, 1, new QTableWidgetItem(formatHexValue(item.rawBytes, item.hexString)));
        
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
        dataTable_->setItem(i, 2, decItem);

        // Binary
        dataTable_->setItem(i, 3, new QTableWidgetItem(formatBinaryValue(item.rawBytes, item.binaryString)));

        // Description
        dataTable_->setItem(i, 4, new QTableWidgetItem(item.desc));
    }
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
        dataTable_->setHorizontalHeaderLabels({tr("Address"), tr("Hex"), tr("Decimal"), tr("Binary"), tr("Description")});
    }

    // Refresh result text if needed (simple approach: clear or keep existing static text)
    // statusLabel_ text is dynamic, so we might leave it or reset to Ready if empty
    if (statusLabel_ && statusLabel_->text() == "Ready") { // Basic check, ideally track state
         statusLabel_->setText(tr("Ready"));
    }
}

} // namespace ui::widgets
