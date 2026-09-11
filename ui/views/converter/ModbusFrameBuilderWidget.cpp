/**
 * @file ModbusFrameBuilderWidget.cpp
 * @brief Implementation of ModbusFrameBuilderWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusFrameBuilderWidget.h"
#include "common/ModbusDataHelper.h"
#include "infra/config/ISettingsService.h"
#include "modbus/base/ModbusAduBuilder.h"
#include "modbus/base/ModbusPduBuilder.h"
#include "modbus/base/ModbusEndianCodec.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpressionValidator>
#include <QToolTip>
#include <QFontDatabase>
#include <QSignalBlocker>
#include <QCursor>

namespace ui::views::converter {

namespace {

QString formatFunctionCodeName(modbus::base::FunctionCode fc) {
    switch (fc) {
    case modbus::base::FunctionCode::ReadCoils:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Read Coils (0x01)");
    case modbus::base::FunctionCode::ReadDiscreteInputs:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Read Discrete Inputs (0x02)");
    case modbus::base::FunctionCode::ReadHoldingRegisters:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Read Holding Registers (0x03)");
    case modbus::base::FunctionCode::ReadInputRegisters:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Read Input Registers (0x04)");
    case modbus::base::FunctionCode::WriteSingleCoil:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Write Single Coil (0x05)");
    case modbus::base::FunctionCode::WriteSingleRegister:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Write Single Register (0x06)");
    case modbus::base::FunctionCode::WriteMultipleCoils:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Write Multiple Coils (0x0F)");
    case modbus::base::FunctionCode::WriteMultipleRegisters:
        return QCoreApplication::translate("ui::views::converter::ModbusFrameBuilderWidget", "Write Multiple Registers (0x10)");
    default:
        return QStringLiteral("0x%1").arg(static_cast<int>(fc), 2, 16, QLatin1Char('0')).toUpper();
    }
}

void addBreakdownRow(QTableWidget* table, const QString& field, const QString& hex, const QString& desc) {
    const int row = table->rowCount();
    table->insertRow(row);

    auto* itemField = new QTableWidgetItem(field);
    itemField->setFlags(itemField->flags() & ~Qt::ItemIsEditable);
    table->setItem(row, 0, itemField);

    auto* itemHex = new QTableWidgetItem(hex);
    itemHex->setFlags(itemHex->flags() & ~Qt::ItemIsEditable);
    itemHex->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    table->setItem(row, 1, itemHex);

    auto* itemDesc = new QTableWidgetItem(desc);
    itemDesc->setFlags(itemDesc->flags() & ~Qt::ItemIsEditable);
    table->setItem(row, 2, itemDesc);
}

} // namespace

ModbusFrameBuilderWidget::ModbusFrameBuilderWidget(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
    loadSettings();
    updateFormVisibility();
    rebuildFrame();
}

void ModbusFrameBuilderWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // =========================================================================
    // 1. Parameter Settings Group
    // =========================================================================
    paramGroup_ = new QGroupBox(tr("Frame Parameters"), this);
    auto* paramLayout = new QGridLayout(paramGroup_);
    paramLayout->setContentsMargins(12, 10, 12, 10);
    paramLayout->setSpacing(8);

    // Row 0: Protocol & Slave/Unit ID
    protocolLabel_ = new QLabel(tr("Protocol:"), paramGroup_);
    protocolCombo_ = new QComboBox(paramGroup_);
    protocolCombo_->addItem(QStringLiteral("Modbus RTU"), 0);
    protocolCombo_->addItem(QStringLiteral("Modbus TCP"), 1);
    protocolCombo_->addItem(QStringLiteral("Modbus ASCII"), 2);
    paramLayout->addWidget(protocolLabel_, 0, 0);
    paramLayout->addWidget(protocolCombo_, 0, 1);

    slaveIdLabel_ = new QLabel(tr("Slave ID:"), paramGroup_);
    slaveIdEdit_ = new QLineEdit(paramGroup_);
    slaveIdEdit_->setText(QStringLiteral("1"));
    slaveIdEdit_->setPlaceholderText(tr("1 - 247"));
    paramLayout->addWidget(slaveIdLabel_, 0, 2);
    paramLayout->addWidget(slaveIdEdit_, 0, 3);

    // Row 1: Function Code & Address Base
    functionLabel_ = new QLabel(tr("Function Code:"), paramGroup_);
    functionCombo_ = new QComboBox(paramGroup_);
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::ReadCoils),
                            static_cast<int>(modbus::base::FunctionCode::ReadCoils));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::ReadDiscreteInputs),
                            static_cast<int>(modbus::base::FunctionCode::ReadDiscreteInputs));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::ReadHoldingRegisters),
                            static_cast<int>(modbus::base::FunctionCode::ReadHoldingRegisters));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::ReadInputRegisters),
                            static_cast<int>(modbus::base::FunctionCode::ReadInputRegisters));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::WriteSingleCoil),
                            static_cast<int>(modbus::base::FunctionCode::WriteSingleCoil));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::WriteSingleRegister),
                            static_cast<int>(modbus::base::FunctionCode::WriteSingleRegister));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::WriteMultipleCoils),
                            static_cast<int>(modbus::base::FunctionCode::WriteMultipleCoils));
    functionCombo_->addItem(formatFunctionCodeName(modbus::base::FunctionCode::WriteMultipleRegisters),
                            static_cast<int>(modbus::base::FunctionCode::WriteMultipleRegisters));
    paramLayout->addWidget(functionLabel_, 1, 0);
    paramLayout->addWidget(functionCombo_, 1, 1);

    addressBaseLabel_ = new QLabel(tr("Address Base:"), paramGroup_);
    addressBaseCombo_ = new QComboBox(paramGroup_);
    addressBaseCombo_->addItem(tr("0-Based (PDU: 0-65535)"), static_cast<int>(modbus::address::AddressBase::Offset0Based));
    addressBaseCombo_->addItem(tr("1-Based (PLC: 1-65536)"), static_cast<int>(modbus::address::AddressBase::PlcAddress1Based));
    paramLayout->addWidget(addressBaseLabel_, 1, 2);
    paramLayout->addWidget(addressBaseCombo_, 1, 3);

    // Row 2: Start Address & Quantity
    addressLabel_ = new QLabel(tr("Start Address:"), paramGroup_);
    addressEdit_ = new QLineEdit(paramGroup_);
    addressEdit_->setText(QStringLiteral("0x0000"));
    addressEdit_->setPlaceholderText(tr("e.g. 0, 0x0000, 40001"));
    paramLayout->addWidget(addressLabel_, 2, 0);
    paramLayout->addWidget(addressEdit_, 2, 1);

    quantityLabel_ = new QLabel(tr("Quantity:"), paramGroup_);
    quantitySpin_ = new QSpinBox(paramGroup_);
    quantitySpin_->setRange(1, 125);
    quantitySpin_->setValue(2);
    paramLayout->addWidget(quantityLabel_, 2, 2);
    paramLayout->addWidget(quantitySpin_, 2, 3);

    // Row 3: Write Data & Format
    writeDataLabel_ = new QLabel(tr("Write Data:"), paramGroup_);
    writeDataEdit_ = new QLineEdit(paramGroup_);
    writeDataEdit_->setPlaceholderText(tr("e.g. 0001 0002 or 1, 2"));
    paramLayout->addWidget(writeDataLabel_, 3, 0);
    paramLayout->addWidget(writeDataEdit_, 3, 1);

    dataFormatLabel_ = new QLabel(tr("Data Format:"), paramGroup_);
    dataFormatCombo_ = new QComboBox(paramGroup_);
    dataFormatCombo_->addItem(QStringLiteral("Hex"), 0);
    dataFormatCombo_->addItem(QStringLiteral("Decimal"), 1);
    dataFormatCombo_->addItem(QStringLiteral("Binary"), 2);
    paramLayout->addWidget(dataFormatLabel_, 3, 2);
    paramLayout->addWidget(dataFormatCombo_, 3, 3);

    mainLayout->addWidget(paramGroup_);

    // =========================================================================
    // 2. Output & Breakdown Group
    // =========================================================================
    outputGroup_ = new QGroupBox(tr("Generated Frame"), this);
    auto* outputLayout = new QVBoxLayout(outputGroup_);
    outputLayout->setContentsMargins(12, 10, 12, 10);
    outputLayout->setSpacing(8);

    // Full Frame Raw Hex display
    auto* hexRow = new QHBoxLayout();
    hexOutputLabel_ = new QLabel(tr("Full Frame Hex:"), outputGroup_);
    hexOutputEdit_ = new QLineEdit(outputGroup_);
    hexOutputEdit_->setReadOnly(true);
    hexOutputEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    hexOutputEdit_->setStyleSheet(QStringLiteral("QLineEdit { font-size: 13px; font-weight: 500; }"));
    hexRow->addWidget(hexOutputLabel_);
    hexRow->addWidget(hexOutputEdit_, 1);
    outputLayout->addLayout(hexRow);

    // Error notification label
    errorLabel_ = new QLabel(outputGroup_);
    errorLabel_->setStyleSheet(QStringLiteral("QLabel { color: #EF4444; font-weight: bold; padding: 2px; }"));
    errorLabel_->setWordWrap(true);
    errorLabel_->setVisible(false);
    outputLayout->addWidget(errorLabel_);

    // Field Breakdown Table
    breakdownTable_ = new QTableWidget(outputGroup_);
    breakdownTable_->setColumnCount(3);
    breakdownTable_->setHorizontalHeaderLabels({tr("Field"), tr("Hex"), tr("Description")});
    breakdownTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    breakdownTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    breakdownTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    breakdownTable_->setAlternatingRowColors(true);
    breakdownTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    breakdownTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    breakdownTable_->verticalHeader()->setVisible(false);
    breakdownTable_->setMinimumHeight(180);
    outputLayout->addWidget(breakdownTable_, 1);

    // Action buttons toolbar
    auto* actionsLayout = new QHBoxLayout();
    copySpacedBtn_ = new QPushButton(tr("Copy Hex (spaced)"), outputGroup_);
    copyCompactBtn_ = new QPushButton(tr("Copy Hex (compact)"), outputGroup_);
    copyCArrayBtn_ = new QPushButton(tr("Copy C array"), outputGroup_);
    inspectAnalyzerBtn_ = new QPushButton(tr("Inspect in Frame Analyzer"), outputGroup_);
    inspectAnalyzerBtn_->setStyleSheet(QStringLiteral("QPushButton { font-weight: bold; }"));

    actionsLayout->addWidget(copySpacedBtn_);
    actionsLayout->addWidget(copyCompactBtn_);
    actionsLayout->addWidget(copyCArrayBtn_);
    actionsLayout->addStretch(1);
    actionsLayout->addWidget(inspectAnalyzerBtn_);
    outputLayout->addLayout(actionsLayout);

    mainLayout->addWidget(outputGroup_, 1);

    // =========================================================================
    // 3. Connect signals and slots
    // =========================================================================
    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusFrameBuilderWidget::onProtocolChanged);
    connect(functionCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusFrameBuilderWidget::onFunctionCodeChanged);
    connect(addressBaseCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusFrameBuilderWidget::onParameterChanged);
    connect(dataFormatCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusFrameBuilderWidget::onParameterChanged);

    connect(slaveIdEdit_, &QLineEdit::textChanged, this, &ModbusFrameBuilderWidget::onParameterChanged);
    connect(addressEdit_, &QLineEdit::textChanged, this, &ModbusFrameBuilderWidget::onParameterChanged);
    connect(writeDataEdit_, &QLineEdit::textChanged, this, &ModbusFrameBuilderWidget::onParameterChanged);
    connect(quantitySpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusFrameBuilderWidget::onParameterChanged);

    connect(copySpacedBtn_, &QPushButton::clicked, this, &ModbusFrameBuilderWidget::onCopySpacedHexClicked);
    connect(copyCompactBtn_, &QPushButton::clicked, this, &ModbusFrameBuilderWidget::onCopyCompactHexClicked);
    connect(copyCArrayBtn_, &QPushButton::clicked, this, &ModbusFrameBuilderWidget::onCopyCArrayClicked);
    connect(inspectAnalyzerBtn_, &QPushButton::clicked, this, &ModbusFrameBuilderWidget::onInspectInAnalyzerClicked);
}

void ModbusFrameBuilderWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void ModbusFrameBuilderWidget::retranslateUi() {
    if (paramGroup_) paramGroup_->setTitle(tr("Frame Parameters"));
    if (protocolLabel_) protocolLabel_->setText(tr("Protocol:"));
    if (addressBaseLabel_) addressBaseLabel_->setText(tr("Address Base:"));
    if (addressLabel_) addressLabel_->setText(tr("Start Address:"));
    if (quantityLabel_) quantityLabel_->setText(tr("Quantity:"));
    if (writeDataLabel_) writeDataLabel_->setText(tr("Write Data:"));
    if (dataFormatLabel_) dataFormatLabel_->setText(tr("Data Format:"));
    if (outputGroup_) outputGroup_->setTitle(tr("Generated Frame"));
    if (hexOutputLabel_) hexOutputLabel_->setText(tr("Full Frame Hex:"));
    if (copySpacedBtn_) copySpacedBtn_->setText(tr("Copy Hex (spaced)"));
    if (copyCompactBtn_) copyCompactBtn_->setText(tr("Copy Hex (compact)"));
    if (copyCArrayBtn_) copyCArrayBtn_->setText(tr("Copy C array"));
    if (inspectAnalyzerBtn_) inspectAnalyzerBtn_->setText(tr("Inspect in Frame Analyzer"));

    if (breakdownTable_) {
        breakdownTable_->setHorizontalHeaderLabels({tr("Field"), tr("Hex"), tr("Description")});
    }

    if (addressBaseCombo_) {
        const QSignalBlocker blocker(addressBaseCombo_);
        addressBaseCombo_->setItemText(0, tr("0-Based (PDU: 0-65535)"));
        addressBaseCombo_->setItemText(1, tr("1-Based (PLC: 1-65536)"));
    }

    if (functionCombo_) {
        const QSignalBlocker blocker(functionCombo_);
        for (int i = 0; i < functionCombo_->count(); ++i) {
            const auto fc = static_cast<modbus::base::FunctionCode>(functionCombo_->itemData(i).toInt());
            functionCombo_->setItemText(i, formatFunctionCodeName(fc));
        }
    }

    updateFormVisibility();
    rebuildFrame();
}

void ModbusFrameBuilderWidget::onProtocolChanged(int /*index*/) {
    updateFormVisibility();
    saveSettings();
    rebuildFrame();
}

void ModbusFrameBuilderWidget::onFunctionCodeChanged(int /*index*/) {
    updateFormVisibility();
    saveSettings();
    rebuildFrame();
}

void ModbusFrameBuilderWidget::onParameterChanged() {
    if (isSyncing_) return;
    saveSettings();
    rebuildFrame();
}

void ModbusFrameBuilderWidget::updateFormVisibility() {
    const int proto = protocolCombo_ ? protocolCombo_->currentIndex() : 0;
    const bool isTcp = (proto == 1);

    if (slaveIdLabel_) {
        slaveIdLabel_->setText(isTcp ? tr("Unit ID:") : tr("Slave ID:"));
    }
    if (slaveIdEdit_) {
        slaveIdEdit_->setPlaceholderText(isTcp ? tr("1 - 255") : tr("1 - 247"));
    }

    const auto fc = functionCombo_
                        ? static_cast<modbus::base::FunctionCode>(functionCombo_->currentData().toInt())
                        : modbus::base::FunctionCode::ReadHoldingRegisters;

    const bool isRead = (fc == modbus::base::FunctionCode::ReadCoils ||
                         fc == modbus::base::FunctionCode::ReadDiscreteInputs ||
                         fc == modbus::base::FunctionCode::ReadHoldingRegisters ||
                         fc == modbus::base::FunctionCode::ReadInputRegisters);

    const bool isSingleWrite = (fc == modbus::base::FunctionCode::WriteSingleCoil ||
                                fc == modbus::base::FunctionCode::WriteSingleRegister);

    const bool isMultiWrite = (fc == modbus::base::FunctionCode::WriteMultipleCoils ||
                               fc == modbus::base::FunctionCode::WriteMultipleRegisters);

    // Quantity visibility and ranges
    if (quantityLabel_) quantityLabel_->setVisible(isRead || isMultiWrite);
    if (quantitySpin_) {
        quantitySpin_->setVisible(isRead || isMultiWrite);
        const QSignalBlocker blocker(quantitySpin_);
        if (fc == modbus::base::FunctionCode::ReadCoils || fc == modbus::base::FunctionCode::ReadDiscreteInputs) {
            quantitySpin_->setRange(1, 2000);
        } else if (fc == modbus::base::FunctionCode::ReadHoldingRegisters || fc == modbus::base::FunctionCode::ReadInputRegisters) {
            quantitySpin_->setRange(1, 125);
        } else if (fc == modbus::base::FunctionCode::WriteMultipleCoils) {
            quantitySpin_->setRange(1, 1968);
        } else if (fc == modbus::base::FunctionCode::WriteMultipleRegisters) {
            quantitySpin_->setRange(1, 123);
        }
    }

    // Write data controls visibility
    const bool showWrite = (isSingleWrite || isMultiWrite);
    if (writeDataLabel_) writeDataLabel_->setVisible(showWrite);
    if (writeDataEdit_) {
        writeDataEdit_->setVisible(showWrite);
        if (fc == modbus::base::FunctionCode::WriteSingleCoil) {
            writeDataEdit_->setPlaceholderText(tr("e.g. ON, OFF, 1, 0, or FF00"));
        } else if (fc == modbus::base::FunctionCode::WriteSingleRegister) {
            writeDataEdit_->setPlaceholderText(tr("e.g. 1234 or 0x1234"));
        } else if (fc == modbus::base::FunctionCode::WriteMultipleCoils) {
            writeDataEdit_->setPlaceholderText(tr("e.g. 1 0 1 1 (matching quantity)"));
        } else if (fc == modbus::base::FunctionCode::WriteMultipleRegisters) {
            writeDataEdit_->setPlaceholderText(tr("e.g. 0001 0002 or 1, 2"));
        }
    }
    if (dataFormatLabel_) dataFormatLabel_->setVisible(showWrite);
    if (dataFormatCombo_) dataFormatCombo_->setVisible(showWrite);
}

void ModbusFrameBuilderWidget::rebuildFrame() {
    if (!protocolCombo_ || !functionCombo_ || !slaveIdEdit_ || !addressEdit_ || !quantitySpin_) {
        return;
    }

    const int proto = protocolCombo_->currentIndex();
    const bool isTcp = (proto == 1);
    const auto fc = static_cast<modbus::base::FunctionCode>(functionCombo_->currentData().toInt());

    // 1. Parse Slave/Unit ID
    bool idOk = false;
    const int parsedId = ui::common::data_helper::parseSmartInt(slaveIdEdit_->text().trimmed(), &idOk);
    const int maxId = isTcp ? 255 : 247;
    if (!idOk || parsedId < 1 || parsedId > maxId) {
        errorLabel_->setText(tr("Invalid %1 (%2). Valid range: 1 - %3.")
                                 .arg(isTcp ? tr("Unit ID") : tr("Slave ID"))
                                 .arg(slaveIdEdit_->text().trimmed())
                                 .arg(maxId));
        errorLabel_->setVisible(true);
        hexOutputEdit_->clear();
        breakdownTable_->setRowCount(0);
        copySpacedBtn_->setEnabled(false);
        copyCompactBtn_->setEnabled(false);
        copyCArrayBtn_->setEnabled(false);
        inspectAnalyzerBtn_->setEnabled(false);
        currentAdu_.clear();
        currentSpacedHex_.clear();
        return;
    }
    const uint8_t slaveOrUnitId = static_cast<uint8_t>(parsedId);

    // 2. Parse Start Address
    const auto base = static_cast<modbus::address::AddressBase>(addressBaseCombo_->currentData().toInt());
    const auto addrResult = modbus::address::toPduAddress(addressEdit_->text().trimmed(), base);
    if (!addrResult.isValid) {
        errorLabel_->setText(addrResult.errorMessage.isEmpty()
                                 ? tr("Invalid start address: %1").arg(addressEdit_->text().trimmed())
                                 : addrResult.errorMessage);
        errorLabel_->setVisible(true);
        hexOutputEdit_->clear();
        breakdownTable_->setRowCount(0);
        copySpacedBtn_->setEnabled(false);
        copyCompactBtn_->setEnabled(false);
        copyCArrayBtn_->setEnabled(false);
        inspectAnalyzerBtn_->setEnabled(false);
        currentAdu_.clear();
        currentSpacedHex_.clear();
        return;
    }
    const uint16_t startAddress = addrResult.pduAddress;

    // 3. Resolve Quantity and Write Data
    const int quantity = quantitySpin_->value();
    const QString writeRawText = writeDataEdit_->text().trimmed();
    const int fmtIdx = dataFormatCombo_->currentIndex();

    std::optional<modbus::base::Pdu> pduOpt;
    QString errorMsg;

    const bool isRead = (fc == modbus::base::FunctionCode::ReadCoils ||
                         fc == modbus::base::FunctionCode::ReadDiscreteInputs ||
                         fc == modbus::base::FunctionCode::ReadHoldingRegisters ||
                         fc == modbus::base::FunctionCode::ReadInputRegisters);

    if (isRead) {
        pduOpt = modbus::base::pdu_builder::buildReadRequest(fc, startAddress, quantity, &errorMsg);
    } else {
        QByteArray rawPayload;
        if (fc == modbus::base::FunctionCode::WriteSingleCoil) {
            const QString lower = writeRawText.toLower();
            bool coilOn = false;
            if (lower == QStringLiteral("on") || lower == QStringLiteral("true") || lower == QStringLiteral("1")) {
                coilOn = true;
            } else if (lower == QStringLiteral("off") || lower == QStringLiteral("false") || lower == QStringLiteral("0")) {
                coilOn = false;
            } else {
                bool intOk = false;
                const int val = ui::common::data_helper::parseSmartInt(writeRawText, &intOk);
                if (intOk) {
                    coilOn = (val != 0);
                } else {
                    const QByteArray hexBytes = ui::common::data_helper::parseHex(writeRawText);
                    if (!hexBytes.isEmpty()) {
                        coilOn = (hexBytes.at(0) != '\0');
                    }
                }
            }
            rawPayload.resize(2);
            rawPayload[0] = coilOn ? static_cast<char>(0xFF) : '\0';
            rawPayload[1] = '\0';
        } else if (fc == modbus::base::FunctionCode::WriteSingleRegister) {
            if (writeRawText.isEmpty()) {
                errorMsg = tr("Write data cannot be empty for Single Register.");
            } else if (fmtIdx == 1) { // Decimal
                bool ok = false;
                const int val = ui::common::data_helper::parseSmartInt(writeRawText, &ok);
                if (!ok || val < 0 || val > 65535) {
                    errorMsg = tr("Invalid decimal value for register (0 - 65535): %1").arg(writeRawText);
                } else {
                    rawPayload.resize(2);
                    rawPayload[0] = static_cast<char>((val >> 8) & 0xFF);
                    rawPayload[1] = static_cast<char>(val & 0xFF);
                }
            } else if (fmtIdx == 2) { // Binary
                rawPayload = ui::common::data_helper::parseBinary(writeRawText);
                if (rawPayload.size() == 1) rawPayload.prepend('\0');
                if (rawPayload.size() != 2) {
                    errorMsg = tr("Register write requires exactly 16 bits.");
                }
            } else { // Hex
                rawPayload = ui::common::data_helper::parseHex(writeRawText);
                if (rawPayload.size() == 1) rawPayload.prepend('\0');
                if (rawPayload.size() != 2) {
                    errorMsg = tr("Register write requires a 16-bit hex value.");
                }
            }
        } else if (fc == modbus::base::FunctionCode::WriteMultipleCoils) {
            if (fmtIdx == 2) { // Binary
                QString bits = writeRawText;
                bits.remove(QRegularExpression(QStringLiteral("[^01]")));
                if (bits.size() != quantity) {
                    errorMsg = tr("Binary bit count (%1) does not match Quantity (%2).").arg(bits.size()).arg(quantity);
                } else {
                    rawPayload = ui::common::data_helper::parseBinary(bits);
                }
            } else { // Hex or Decimal fallback to Hex
                rawPayload = ui::common::data_helper::parseHex(writeRawText);
                const int expectedBytes = (quantity + 7) / 8;
                if (rawPayload.size() != expectedBytes) {
                    errorMsg = tr("Hex byte count (%1) does not match expected (%2) for %3 coils.")
                                   .arg(rawPayload.size()).arg(expectedBytes).arg(quantity);
                }
            }
        } else if (fc == modbus::base::FunctionCode::WriteMultipleRegisters) {
            if (fmtIdx == 1) { // Decimal
                bool okList = false;
                rawPayload = ui::common::data_helper::parseDecimalList(writeRawText, okList);
                if (!okList) {
                    errorMsg = tr("Invalid decimal list for Multiple Registers.");
                } else if (rawPayload.size() != quantity * 2) {
                    errorMsg = tr("Parsed register count (%1) does not match Quantity (%2).")
                                   .arg(rawPayload.size() / 2).arg(quantity);
                }
            } else { // Hex
                rawPayload = ui::common::data_helper::parseHex(writeRawText);
                if (rawPayload.size() != quantity * 2) {
                    errorMsg = tr("Hex byte count (%1) does not match expected (%2) for %3 registers.")
                                   .arg(rawPayload.size()).arg(quantity * 2).arg(quantity);
                }
            }
        }

        if (errorMsg.isEmpty()) {
            pduOpt = modbus::base::pdu_builder::buildWriteRequest(fc, startAddress, quantity, rawPayload, &errorMsg);
        }
    }

    if (!pduOpt || !errorMsg.isEmpty()) {
        errorLabel_->setText(errorMsg.isEmpty() ? tr("Failed to build Modbus PDU.") : errorMsg);
        errorLabel_->setVisible(true);
        hexOutputEdit_->clear();
        breakdownTable_->setRowCount(0);
        copySpacedBtn_->setEnabled(false);
        copyCompactBtn_->setEnabled(false);
        copyCArrayBtn_->setEnabled(false);
        inspectAnalyzerBtn_->setEnabled(false);
        currentAdu_.clear();
        currentSpacedHex_.clear();
        return;
    }

    // 4. Encapsulate into Application Data Unit (ADU)
    if (proto == 0) { // RTU
        currentAdu_ = modbus::base::buildRtuAdu(slaveOrUnitId, *pduOpt);
        currentSpacedHex_ = QString::fromLatin1(currentAdu_.toHex(' ')).toUpper();
        hexOutputEdit_->setText(currentSpacedHex_);
    } else if (proto == 1) { // TCP
        currentAdu_ = modbus::base::buildTcpAdu(slaveOrUnitId, *pduOpt, 0x0000);
        currentSpacedHex_ = QString::fromLatin1(currentAdu_.toHex(' ')).toUpper();
        hexOutputEdit_->setText(currentSpacedHex_);
    } else { // ASCII
        currentAdu_ = modbus::base::buildAsciiAdu(slaveOrUnitId, *pduOpt);
        currentSpacedHex_ = QString::fromLatin1(currentAdu_).trimmed();
        hexOutputEdit_->setText(currentSpacedHex_);
    }

    errorLabel_->setVisible(false);
    copySpacedBtn_->setEnabled(true);
    copyCompactBtn_->setEnabled(true);
    copyCArrayBtn_->setEnabled(true);
    inspectAnalyzerBtn_->setEnabled(true);

    // 5. Populate Detailed Field Breakdown
    populateFieldTable(proto, slaveOrUnitId, fc, startAddress, quantity, currentAdu_);
}

void ModbusFrameBuilderWidget::populateFieldTable(int protocol, uint8_t slaveOrUnitId,
                                                 ::modbus::base::FunctionCode fc,
                                                 uint16_t startPduAddr, int quantity,
                                                 const QByteArray& adu) {
    breakdownTable_->setRowCount(0);
    const bool isCoil = (fc == modbus::base::FunctionCode::ReadCoils ||
                         fc == modbus::base::FunctionCode::ReadDiscreteInputs ||
                         fc == modbus::base::FunctionCode::WriteSingleCoil ||
                         fc == modbus::base::FunctionCode::WriteMultipleCoils);

    if (protocol == 0) {
        // ======================== MODBUS RTU ========================
        // Slave ID (1 byte)
        addBreakdownRow(breakdownTable_, tr("Slave ID"),
                        QString::fromLatin1(adu.mid(0, 1).toHex()).toUpper(),
                        tr("Device address %1 (0x%2)").arg(slaveOrUnitId).arg(slaveOrUnitId, 2, 16, QLatin1Char('0')).toUpper());

        // Function Code (1 byte)
        addBreakdownRow(breakdownTable_, tr("Function Code"),
                        QString::fromLatin1(adu.mid(1, 1).toHex()).toUpper(),
                        formatFunctionCodeName(fc));

        // PDU Payload
        if (fc == modbus::base::FunctionCode::ReadCoils ||
            fc == modbus::base::FunctionCode::ReadDiscreteInputs ||
            fc == modbus::base::FunctionCode::ReadHoldingRegisters ||
            fc == modbus::base::FunctionCode::ReadInputRegisters) {
            addBreakdownRow(breakdownTable_, tr("Start Address"),
                            QString::fromLatin1(adu.mid(2, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            addBreakdownRow(breakdownTable_, tr("Quantity"),
                            QString::fromLatin1(adu.mid(4, 2).toHex(' ')).toUpper(),
                            tr("%1 %2 (0x%3)").arg(quantity).arg(isCoil ? tr("coils") : tr("registers")).arg(quantity, 4, 16, QLatin1Char('0')).toUpper());
        } else if (fc == modbus::base::FunctionCode::WriteSingleCoil) {
            addBreakdownRow(breakdownTable_, tr("Coil Address"),
                            QString::fromLatin1(adu.mid(2, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            const bool on = (adu.mid(4, 2) == QByteArray::fromHex("FF00"));
            addBreakdownRow(breakdownTable_, tr("Output Value"),
                            QString::fromLatin1(adu.mid(4, 2).toHex(' ')).toUpper(),
                            on ? tr("Coil ON (0xFF00)") : tr("Coil OFF (0x0000)"));
        } else if (fc == modbus::base::FunctionCode::WriteSingleRegister) {
            addBreakdownRow(breakdownTable_, tr("Register Address"),
                            QString::fromLatin1(adu.mid(2, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            const uint16_t regVal = (static_cast<uint8_t>(adu[4]) << 8) | static_cast<uint8_t>(adu[5]);
            addBreakdownRow(breakdownTable_, tr("Register Value"),
                            QString::fromLatin1(adu.mid(4, 2).toHex(' ')).toUpper(),
                            tr("Value: %1 (0x%2)").arg(regVal).arg(regVal, 4, 16, QLatin1Char('0')).toUpper());
        } else if (fc == modbus::base::FunctionCode::WriteMultipleCoils ||
                   fc == modbus::base::FunctionCode::WriteMultipleRegisters) {
            addBreakdownRow(breakdownTable_, tr("Start Address"),
                            QString::fromLatin1(adu.mid(2, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            addBreakdownRow(breakdownTable_, tr("Quantity"),
                            QString::fromLatin1(adu.mid(4, 2).toHex(' ')).toUpper(),
                            tr("%1 %2").arg(quantity).arg(isCoil ? tr("coils") : tr("registers")));
            addBreakdownRow(breakdownTable_, tr("Byte Count"),
                            QString::fromLatin1(adu.mid(6, 1).toHex()).toUpper(),
                            tr("%1 bytes").arg(static_cast<uint8_t>(adu[6])));
            const int dataLen = qMax(0, adu.size() - 7 - 2);
            addBreakdownRow(breakdownTable_, tr("Write Payload"),
                            QString::fromLatin1(adu.mid(7, dataLen).toHex(' ')).toUpper(),
                            tr("Packed output data (%1 bytes)").arg(dataLen));
        }

        // CRC-16 (2 bytes, LE)
        addBreakdownRow(breakdownTable_, tr("CRC-16"),
                        QString::fromLatin1(adu.right(2).toHex(' ')).toUpper(),
                        tr("CRC-16/MODBUS (LE: Low byte first)"));

    } else if (protocol == 1) {
        // ======================== MODBUS TCP ========================
        // MBAP Header (7 bytes)
        addBreakdownRow(breakdownTable_, tr("Transaction ID"),
                        QString::fromLatin1(adu.mid(0, 2).toHex(' ')).toUpper(),
                        tr("Transaction Identifier (0x0000)"));
        addBreakdownRow(breakdownTable_, tr("Protocol ID"),
                        QString::fromLatin1(adu.mid(2, 2).toHex(' ')).toUpper(),
                        tr("0x0000 = Modbus Protocol"));
        const uint16_t len = (static_cast<uint8_t>(adu[4]) << 8) | static_cast<uint8_t>(adu[5]);
        addBreakdownRow(breakdownTable_, tr("Length"),
                        QString::fromLatin1(adu.mid(4, 2).toHex(' ')).toUpper(),
                        tr("%1 bytes following").arg(len));
        addBreakdownRow(breakdownTable_, tr("Unit ID"),
                        QString::fromLatin1(adu.mid(6, 1).toHex()).toUpper(),
                        tr("Unit identifier %1 (0x%2)").arg(slaveOrUnitId).arg(slaveOrUnitId, 2, 16, QLatin1Char('0')).toUpper());

        // Function Code (1 byte)
        addBreakdownRow(breakdownTable_, tr("Function Code"),
                        QString::fromLatin1(adu.mid(7, 1).toHex()).toUpper(),
                        formatFunctionCodeName(fc));

        // PDU Payload (starting from index 8)
        if (fc == modbus::base::FunctionCode::ReadCoils ||
            fc == modbus::base::FunctionCode::ReadDiscreteInputs ||
            fc == modbus::base::FunctionCode::ReadHoldingRegisters ||
            fc == modbus::base::FunctionCode::ReadInputRegisters) {
            addBreakdownRow(breakdownTable_, tr("Start Address"),
                            QString::fromLatin1(adu.mid(8, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            addBreakdownRow(breakdownTable_, tr("Quantity"),
                            QString::fromLatin1(adu.mid(10, 2).toHex(' ')).toUpper(),
                            tr("%1 %2 (0x%3)").arg(quantity).arg(isCoil ? tr("coils") : tr("registers")).arg(quantity, 4, 16, QLatin1Char('0')).toUpper());
        } else if (fc == modbus::base::FunctionCode::WriteSingleCoil) {
            addBreakdownRow(breakdownTable_, tr("Coil Address"),
                            QString::fromLatin1(adu.mid(8, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            const bool on = (adu.mid(10, 2) == QByteArray::fromHex("FF00"));
            addBreakdownRow(breakdownTable_, tr("Output Value"),
                            QString::fromLatin1(adu.mid(10, 2).toHex(' ')).toUpper(),
                            on ? tr("Coil ON (0xFF00)") : tr("Coil OFF (0x0000)"));
        } else if (fc == modbus::base::FunctionCode::WriteSingleRegister) {
            addBreakdownRow(breakdownTable_, tr("Register Address"),
                            QString::fromLatin1(adu.mid(8, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            const uint16_t regVal = (static_cast<uint8_t>(adu[10]) << 8) | static_cast<uint8_t>(adu[11]);
            addBreakdownRow(breakdownTable_, tr("Register Value"),
                            QString::fromLatin1(adu.mid(10, 2).toHex(' ')).toUpper(),
                            tr("Value: %1 (0x%2)").arg(regVal).arg(regVal, 4, 16, QLatin1Char('0')).toUpper());
        } else if (fc == modbus::base::FunctionCode::WriteMultipleCoils ||
                   fc == modbus::base::FunctionCode::WriteMultipleRegisters) {
            addBreakdownRow(breakdownTable_, tr("Start Address"),
                            QString::fromLatin1(adu.mid(8, 2).toHex(' ')).toUpper(),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            addBreakdownRow(breakdownTable_, tr("Quantity"),
                            QString::fromLatin1(adu.mid(10, 2).toHex(' ')).toUpper(),
                            tr("%1 %2").arg(quantity).arg(isCoil ? tr("coils") : tr("registers")));
            addBreakdownRow(breakdownTable_, tr("Byte Count"),
                            QString::fromLatin1(adu.mid(12, 1).toHex()).toUpper(),
                            tr("%1 bytes").arg(static_cast<uint8_t>(adu[12])));
            const int dataLen = qMax(0, adu.size() - 13);
            addBreakdownRow(breakdownTable_, tr("Write Payload"),
                            QString::fromLatin1(adu.mid(13, dataLen).toHex(' ')).toUpper(),
                            tr("Packed output data (%1 bytes)").arg(dataLen));
        }

    } else {
        // ======================== MODBUS ASCII ========================
        addBreakdownRow(breakdownTable_, tr("Start Delimiter"),
                        QStringLiteral("3A (':')"),
                        tr("ASCII Colon delimiter"));

        addBreakdownRow(breakdownTable_, tr("Slave ID"),
                        QString::fromLatin1(adu.mid(1, 2)),
                        tr("Device address %1 (0x%2)").arg(slaveOrUnitId).arg(slaveOrUnitId, 2, 16, QLatin1Char('0')).toUpper());

        addBreakdownRow(breakdownTable_, tr("Function Code"),
                        QString::fromLatin1(adu.mid(3, 2)),
                        formatFunctionCodeName(fc));

        if (fc == modbus::base::FunctionCode::ReadCoils ||
            fc == modbus::base::FunctionCode::ReadDiscreteInputs ||
            fc == modbus::base::FunctionCode::ReadHoldingRegisters ||
            fc == modbus::base::FunctionCode::ReadInputRegisters) {
            addBreakdownRow(breakdownTable_, tr("Start Address"),
                            QString::fromLatin1(adu.mid(5, 4)),
                            tr("Address %1 (0x%2)").arg(startPduAddr).arg(startPduAddr, 4, 16, QLatin1Char('0')).toUpper());
            addBreakdownRow(breakdownTable_, tr("Quantity"),
                            QString::fromLatin1(adu.mid(9, 4)),
                            tr("%1 %2").arg(quantity).arg(isCoil ? tr("coils") : tr("registers")));
        } else {
            const int payloadLen = qMax(0, adu.size() - 5 - 4); // minus start(1)+slave(2)+fc(2) and lrc(2)+crlf(2)
            addBreakdownRow(breakdownTable_, tr("PDU Payload"),
                            QString::fromLatin1(adu.mid(5, payloadLen)),
                            tr("ASCII encoded PDU data"));
        }

        addBreakdownRow(breakdownTable_, tr("LRC Checksum"),
                        QString::fromLatin1(adu.mid(adu.size() - 4, 2)),
                        tr("Longitudinal Redundancy Check"));

        addBreakdownRow(breakdownTable_, tr("End Delimiter"),
                        QStringLiteral("0D 0A (\"\\r\\n\")"),
                        tr("Carriage Return & Line Feed"));
    }
}

void ModbusFrameBuilderWidget::onCopySpacedHexClicked() {
    if (currentAdu_.isEmpty()) return;
    const int proto = protocolCombo_ ? protocolCombo_->currentIndex() : 0;
    const QString text = (proto == 2)
                             ? QString::fromLatin1(currentAdu_.toHex(' ')).toUpper()
                             : currentSpacedHex_;
    copyToClipboard(text, tr("Spaced Hex copied to clipboard"));
}

void ModbusFrameBuilderWidget::onCopyCompactHexClicked() {
    if (currentAdu_.isEmpty()) return;
    const int proto = protocolCombo_ ? protocolCombo_->currentIndex() : 0;
    const QString text = (proto == 2)
                             ? QString::fromLatin1(currentAdu_).trimmed()
                             : QString::fromLatin1(currentAdu_.toHex()).toUpper();
    copyToClipboard(text, tr("Compact Hex copied to clipboard"));
}

void ModbusFrameBuilderWidget::onCopyCArrayClicked() {
    if (currentAdu_.isEmpty()) return;
    const int proto = protocolCombo_ ? protocolCombo_->currentIndex() : 0;
    QString cCode;
    if (proto == 2) {
        cCode = QString("const char frame[] = \"%1\\r\\n\";")
                    .arg(QString::fromLatin1(currentAdu_).trimmed());
    } else {
        QStringList hexBytes;
        hexBytes.reserve(currentAdu_.size());
        for (int i = 0; i < currentAdu_.size(); ++i) {
            hexBytes.append(QString("0x%1")
                                .arg(static_cast<uint8_t>(currentAdu_.at(i)), 2, 16, QLatin1Char('0'))
                                .toUpper());
        }
        cCode = QString("const uint8_t frame[%1] = { %2 };")
                    .arg(currentAdu_.size())
                    .arg(hexBytes.join(QStringLiteral(", ")));
    }
    copyToClipboard(cCode, tr("C array declaration copied to clipboard"));
}

void ModbusFrameBuilderWidget::onInspectInAnalyzerClicked() {
    if (currentAdu_.isEmpty()) return;
    emit inspectInAnalyzerRequested(currentSpacedHex_);
}

void ModbusFrameBuilderWidget::copyToClipboard(const QString& text, const QString& message) {
    if (auto* cb = QGuiApplication::clipboard()) {
        cb->setText(text);
    }
    QToolTip::showText(QCursor::pos(), message, this, {}, 1500);
}

void ModbusFrameBuilderWidget::loadSettings() {
    if (!settingsService_) return;
    isSyncing_ = true;

    auto getSetting = [this](const QString& key, const QVariant& def) -> QVariant {
        if (!settingsService_->contains(key)) return def;
        const QVariant v = settingsService_->value(key);
        return v.isValid() ? v : def;
    };

    const int proto = getSetting(QStringLiteral("tools/builder/protocol"), 0).toInt();
    if (proto >= 0 && proto < protocolCombo_->count()) {
        protocolCombo_->setCurrentIndex(proto);
    }

    slaveIdEdit_->setText(getSetting(QStringLiteral("tools/builder/slaveId"), QStringLiteral("1")).toString());

    const int fcIdx = getSetting(QStringLiteral("tools/builder/functionCode"), 2).toInt();
    if (fcIdx >= 0 && fcIdx < functionCombo_->count()) {
        functionCombo_->setCurrentIndex(fcIdx);
    }

    addressBaseCombo_->setCurrentIndex(
        getSetting(QStringLiteral("tools/builder/addressBase"), 0).toInt());

    addressEdit_->setText(
        getSetting(QStringLiteral("tools/builder/startAddress"), QStringLiteral("0x0000")).toString());

    quantitySpin_->setValue(
        getSetting(QStringLiteral("tools/builder/quantity"), 2).toInt());

    writeDataEdit_->setText(
        getSetting(QStringLiteral("tools/builder/writeData"), QStringLiteral("0001 0002")).toString());

    dataFormatCombo_->setCurrentIndex(
        getSetting(QStringLiteral("tools/builder/dataFormat"), 0).toInt());

    isSyncing_ = false;
}

void ModbusFrameBuilderWidget::saveSettings() {
    if (!settingsService_ || isSyncing_) return;

    settingsService_->setValue(QStringLiteral("tools/builder/protocol"), protocolCombo_->currentIndex());
    settingsService_->setValue(QStringLiteral("tools/builder/slaveId"), slaveIdEdit_->text());
    settingsService_->setValue(QStringLiteral("tools/builder/functionCode"), functionCombo_->currentIndex());
    settingsService_->setValue(QStringLiteral("tools/builder/addressBase"), addressBaseCombo_->currentIndex());
    settingsService_->setValue(QStringLiteral("tools/builder/startAddress"), addressEdit_->text());
    settingsService_->setValue(QStringLiteral("tools/builder/quantity"), quantitySpin_->value());
    settingsService_->setValue(QStringLiteral("tools/builder/writeData"), writeDataEdit_->text());
    settingsService_->setValue(QStringLiteral("tools/builder/dataFormat"), dataFormatCombo_->currentIndex());
}

} // namespace ui::views::converter
