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

namespace ui::widgets {

using namespace modbus::core::parser;

FrameAnalyzerWidget::FrameAnalyzerWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

FrameAnalyzerWidget::~FrameAnalyzerWidget() = default;

void FrameAnalyzerWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    createInputGroup();
    createResultGroup();

    mainLayout->addWidget(new QGroupBox("Input Frame", this)); // Placeholder for layout structure
    
    // Re-organize layout by adding widgets to main layout
    // (The create functions will return widgets or add to layout directly? 
    // Let's refactor to standard Qt pattern: create member widgets and add to layout here)
}

void FrameAnalyzerWidget::createInputGroup()
{
    auto group = new QGroupBox("Frame Input", this);
    auto groupLayout = new QVBoxLayout(group);

    // Controls Row
    auto controlsLayout = new QHBoxLayout();
    
    controlsLayout->addWidget(new QLabel("Protocol:", this));
    protocolCombo_ = new QComboBox(this);
    protocolCombo_->addItem("Auto Detect", QVariant::fromValue(ProtocolType::Unknown));
    protocolCombo_->addItem("Modbus TCP", QVariant::fromValue(ProtocolType::Tcp));
    protocolCombo_->addItem("Modbus RTU", QVariant::fromValue(ProtocolType::Rtu));
    controlsLayout->addWidget(protocolCombo_);

    controlsLayout->addSpacing(20);
    controlsLayout->addWidget(new QLabel("Start Address (for Response):", this));
    startAddressSpin_ = new QSpinBox(this);
    startAddressSpin_->setRange(0, 65535);
    startAddressSpin_->setValue(0);
    controlsLayout->addWidget(startAddressSpin_);

    controlsLayout->addStretch();
    
    formatBtn_ = new QPushButton("Format Hex", this);
    connect(formatBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onFormatClicked);
    controlsLayout->addWidget(formatBtn_);

    parseBtn_ = new QPushButton("Parse", this);
    connect(parseBtn_, &QPushButton::clicked, this, &FrameAnalyzerWidget::onParseClicked);
    controlsLayout->addWidget(parseBtn_);

    auto clearBtn = new QPushButton("Clear", this);
    connect(clearBtn, &QPushButton::clicked, this, &FrameAnalyzerWidget::onClearClicked);
    controlsLayout->addWidget(clearBtn);

    groupLayout->addLayout(controlsLayout);

    // Input Editor
    inputEditor_ = new QPlainTextEdit(this);
    inputEditor_->setPlaceholderText("Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)");
    inputEditor_->setMaximumHeight(100);
    groupLayout->addWidget(inputEditor_);

    layout()->addWidget(group);
}

void FrameAnalyzerWidget::createResultGroup()
{
    auto group = new QGroupBox("Analysis Result", this);
    auto groupLayout = new QVBoxLayout(group);

    statusLabel_ = new QLabel("Ready", this);
    statusLabel_->setStyleSheet("font-weight: bold; color: gray;");
    groupLayout->addWidget(statusLabel_);

    resultTabs_ = new QTabWidget(this);

    // Tab 1: Structure Overview
    overviewTree_ = new QTreeWidget(this);
    overviewTree_->setHeaderLabels({"Field", "Value", "Description"});
    overviewTree_->setColumnWidth(0, 200);
    overviewTree_->setColumnWidth(1, 150);
    resultTabs_->addTab(overviewTree_, "Structure");

    // Tab 2: Decoded Data
    dataTable_ = new QTableWidget(this);
    dataTable_->setColumnCount(4);
    dataTable_->setHorizontalHeaderLabels({"Address", "Hex", "Decimal", "Description"});
    dataTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultTabs_->addTab(dataTable_, "Data Details");

    groupLayout->addWidget(resultTabs_);
    layout()->addWidget(group);
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
    statusLabel_->setText("Ready");
    statusLabel_->setStyleSheet("color: gray;");
    overviewTree_->clear();
    dataTable_->setRowCount(0);
}

void FrameAnalyzerWidget::onParseClicked()
{
    clearResult();

    QString hexStr = inputEditor_->toPlainText().remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (hexStr.isEmpty()) {
        statusLabel_->setText("Error: Empty input");
        statusLabel_->setStyleSheet("color: red;");
        return;
    }

    QByteArray frame = QByteArray::fromHex(hexStr.toLatin1());
    
    ProtocolType type = protocolCombo_->currentData().value<ProtocolType>();
    uint16_t startAddr = static_cast<uint16_t>(startAddressSpin_->value());

    ParseResult result = ModbusFrameParser::parse(frame, type, startAddr);
    renderResult(result);
}

void FrameAnalyzerWidget::renderResult(const ParseResult& result)
{
    if (!result.isValid) {
        statusLabel_->setText("Parse Failed: " + result.error);
        statusLabel_->setStyleSheet("color: red; font-weight: bold;");
        return;
    }

    statusLabel_->setText(QString("Success (%1)").arg(result.protocol == ProtocolType::Tcp ? "TCP" : "RTU"));
    statusLabel_->setStyleSheet("color: green; font-weight: bold;");

    // 1. Populate Overview Tree
    QTreeWidgetItem* root = new QTreeWidgetItem(overviewTree_);
    root->setText(0, "Frame");
    root->setText(1, QString("%1 bytes").arg(result.pduData.size() + (result.protocol == ProtocolType::Tcp ? 7 : 3))); // Approximate
    
    // Header Info
    if (result.protocol == ProtocolType::Tcp) {
        new QTreeWidgetItem(root, {"Transaction ID", QString::number(result.transactionId), ""});
        new QTreeWidgetItem(root, {"Protocol ID", QString::number(result.protocolId), ""});
        new QTreeWidgetItem(root, {"Length", QString::number(result.length), ""});
        new QTreeWidgetItem(root, {"Unit ID", QString::number(result.slaveId), ""});
    } else {
        new QTreeWidgetItem(root, {"Slave ID", QString::number(result.slaveId), ""});
    }

    // Function Code
    new QTreeWidgetItem(root, {"Function Code", QString::number((uint8_t)result.functionCode), result.isException ? "Exception" : "Normal"});

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
        dataTable_->setItem(i, 0, new QTableWidgetItem(QString::number(item.address)));
        dataTable_->setItem(i, 1, new QTableWidgetItem(item.hexString));
        
        // Try to show decimal if value is valid
        QString decStr = "-";
        if (item.value.isValid()) {
            decStr = item.value.toString();
        }
        dataTable_->setItem(i, 2, new QTableWidgetItem(decStr));
        dataTable_->setItem(i, 3, new QTableWidgetItem(item.desc));
    }
}

} // namespace ui::widgets
