#include "FunctionWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "modbus/base/ModbusDataHelper.h"
#include "modbus/base/ModbusCrc.h"
#include "../common/ISettingsService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QEvent>
#include <QFont>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

FunctionWidget::FunctionWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
}

FunctionWidget::~FunctionWidget() = default;

void FunctionWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    standardSection_ = new CollapsibleSection(settingsService_, this);
    setupStandardUi(standardSection_->contentWidget());
    mainLayout->addWidget(standardSection_);

    rawSection_ = new CollapsibleSection(settingsService_, this);
    setupRawUi(rawSection_->contentWidget());
    mainLayout->addWidget(rawSection_);

    retranslateUi();
}

void FunctionWidget::setupStandardUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);
    
    // Row 1: Parameters
    auto paramLayout = new QHBoxLayout();
    paramLayout->setContentsMargins(0, 0, 0, 0);
    paramLayout->setSpacing(2);
    
    slaveIdLabel_ = new QLabel(parent);
    paramLayout->addWidget(slaveIdLabel_);
    slaveIdEdit_ = new QSpinBox(parent);
    slaveIdEdit_->setRange(app::constants::Values::Modbus::kMinSlaveId,
                           app::constants::Values::Modbus::kMaxSlaveId);
    slaveIdEdit_->setValue(app::constants::Values::Modbus::kDefaultSlaveId);
    slaveIdEdit_->setFixedWidth(60);
    paramLayout->addWidget(slaveIdEdit_);

    addressLabel_ = new QLabel(parent);
    paramLayout->addWidget(addressLabel_);
    addressEdit_ = new QSpinBox(parent);
    addressEdit_->setRange(app::constants::Values::Modbus::kMinAddress,
                           app::constants::Values::Modbus::kMaxAddress);
    addressEdit_->setValue(app::constants::Values::Modbus::kDefaultStandardStartAddress);
    addressEdit_->setFixedWidth(80);
    paramLayout->addWidget(addressEdit_);

    quantityLabel_ = new QLabel(parent);
    paramLayout->addWidget(quantityLabel_);
    quantityEdit_ = new QSpinBox(parent);
    quantityEdit_->setRange(app::constants::Values::Modbus::kMinQuantity,
                            app::constants::Values::Modbus::kMaxReadQuantity);
    quantityEdit_->setValue(app::constants::Values::Modbus::kDefaultStandardQuantity);
    quantityEdit_->setFixedWidth(72);
    paramLayout->addWidget(quantityEdit_);
    
    paramLayout->addStretch();
    layout->addLayout(paramLayout);

    // Row 2: Write Data Input
    auto writeLayout = new QHBoxLayout();
    writeLayout->setContentsMargins(0, 0, 0, 0);
    writeLayout->setSpacing(2);
    writeDataLabel_ = new QLabel(parent);
    writeLayout->addWidget(writeDataLabel_);
    writeDataEdit_ = new QLineEdit(parent);
    const int compactWriteWidth = 60 + 80 + 72 + 100;
    writeDataEdit_->setFixedWidth(compactWriteWidth);
    writeLayout->addWidget(writeDataEdit_);
    
    formatLabel_ = new QLabel(parent);
    writeLayout->addWidget(formatLabel_);
    dataFormatBox_ = new QComboBox(parent);
    dataFormatBox_->addItem("", "Hex");
    dataFormatBox_->addItem("", "Decimal");
    dataFormatBox_->addItem("", "Binary");
    dataFormatBox_->setFixedWidth(88);
    writeLayout->addWidget(dataFormatBox_);
    writeLayout->addStretch();
    
    layout->addLayout(writeLayout);

    // Row 3: Function Buttons
    auto btnLayout = new QGridLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setHorizontalSpacing(2);
    btnLayout->setVerticalSpacing(2);
    
    // Read Buttons
    readBtn01_ = new QPushButton(parent);
    readBtn02_ = new QPushButton(parent);
    readBtn03_ = new QPushButton(parent);
    readBtn04_ = new QPushButton(parent);
    
    connect(readBtn01_, &QPushButton::clicked, [this](){ onReadClicked(0x01); });
    connect(readBtn02_, &QPushButton::clicked, [this](){ onReadClicked(0x02); });
    connect(readBtn03_, &QPushButton::clicked, [this](){ onReadClicked(0x03); });
    connect(readBtn04_, &QPushButton::clicked, [this](){ onReadClicked(0x04); });

    // Add buttons to layout
    btnLayout->addWidget(readBtn01_, 0, 0);
    btnLayout->addWidget(readBtn02_, 0, 1);
    btnLayout->addWidget(readBtn03_, 0, 2);
    btnLayout->addWidget(readBtn04_, 0, 3);

    // Write Buttons
    writeBtn05_ = new QPushButton(parent);
    writeBtn06_ = new QPushButton(parent);
    writeBtn0F_ = new QPushButton(parent);
    writeBtn10_ = new QPushButton(parent);

    connect(writeBtn05_, &QPushButton::clicked, [this](){ onWriteClicked(0x05); });
    connect(writeBtn06_, &QPushButton::clicked, [this](){ onWriteClicked(0x06); });
    connect(writeBtn0F_, &QPushButton::clicked, [this](){ onWriteClicked(0x0F); });
    connect(writeBtn10_, &QPushButton::clicked, [this](){ onWriteClicked(0x10); });

    btnLayout->addWidget(writeBtn05_, 1, 0);
    btnLayout->addWidget(writeBtn06_, 1, 1);
    btnLayout->addWidget(writeBtn0F_, 1, 2);
    btnLayout->addWidget(writeBtn10_, 1, 3);

    for (int col = 0; col < 4; ++col) {
        btnLayout->setColumnStretch(col, 1);
    }

    const QList<QPushButton*> fcButtons = {
        readBtn01_, readBtn02_, readBtn03_, readBtn04_,
        writeBtn05_, writeBtn06_, writeBtn0F_, writeBtn10_
    };
    QFont buttonFont = readBtn01_->font();
    if (buttonFont.pointSizeF() > 0.0) {
        buttonFont.setPointSizeF(buttonFont.pointSizeF() - 1.5);
        if (buttonFont.pointSizeF() < 8.0) {
            buttonFont.setPointSizeF(8.0);
        }
    }
    for (auto* button : fcButtons) {
        button->setFont(buttonFont);
        button->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        button->setMinimumWidth(0);
        button->setMinimumHeight(26);
        button->setStyleSheet("padding-left: 2px; padding-right: 2px;");
    }

    layout->addLayout(btnLayout);

    connect(slaveIdEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &FunctionWidget::saveSettings);
    connect(addressEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &FunctionWidget::saveSettings);
    connect(quantityEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &FunctionWidget::saveSettings);
    connect(dataFormatBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, &FunctionWidget::saveSettings);
    connect(dataFormatBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, &FunctionWidget::onFormatChanged);
    
    onFormatChanged();
}

void FunctionWidget::setupRawUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);
    layout->setContentsMargins(8, 0, 8, 0);

    rawDataLabel_ = new QLabel(parent);
    layout->addWidget(rawDataLabel_);
    
    rawDataEdit_ = new QTextEdit(parent);
    rawDataEdit_->setMaximumHeight(96);
    layout->addWidget(rawDataEdit_);

    auto btnLayout = new QHBoxLayout();
    
    appendCrcBtn_ = new QPushButton(parent);
    appendCrcBtn_->setVisible(!isTcp_);
    connect(appendCrcBtn_, &QPushButton::clicked, this, &FunctionWidget::onAppendCrcClicked);
    
    addMbapBtn_ = new QPushButton(parent);
    addMbapBtn_->setVisible(isTcp_);
    connect(addMbapBtn_, &QPushButton::clicked, this, &FunctionWidget::onAddMbapClicked);

    rawSendBtn_ = new QPushButton(parent);
    connect(rawSendBtn_, &QPushButton::clicked, this, &FunctionWidget::onRawSendClicked);
    
    btnLayout->addWidget(appendCrcBtn_);
    btnLayout->addWidget(addMbapBtn_);
    btnLayout->addStretch();
    btnLayout->addWidget(rawSendBtn_);
    
    layout->addLayout(btnLayout);
}

int FunctionWidget::getSlaveId() const {
    return slaveIdEdit_ ? slaveIdEdit_->value() : 1;
}

int FunctionWidget::getStartAddress() const {
    return addressEdit_ ? addressEdit_->value() : 0;
}

int FunctionWidget::getQuantity() const {
    return quantityEdit_ ? quantityEdit_->value() : 1;
}

int FunctionWidget::getFormatIndex() const {
    return dataFormatBox_ ? dataFormatBox_->currentIndex() : 0;
}

void FunctionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (standardSection_) {
        standardSection_->setSettingsKey(settingsGroup_ + "/ui/standardCollapsed");
    }
    if (rawSection_) {
        rawSection_->setSettingsKey(settingsGroup_ + "/ui/rawCollapsed");
    }
    loadSettings();
}

void FunctionWidget::setTcpMode(bool tcp) {
    isTcp_ = tcp;
    if (appendCrcBtn_) appendCrcBtn_->setVisible(!isTcp_);
    if (addMbapBtn_) addMbapBtn_->setVisible(isTcp_);
}

void FunctionWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(slaveIdEdit_);
    QSignalBlocker b2(addressEdit_);
    QSignalBlocker b3(quantityEdit_);
    QSignalBlocker b4(dataFormatBox_);

    const QString slaveKey = settingsGroup_ + "/slaveId";
    const QString addrKey = settingsGroup_ + "/startAddr";
    const QString qtyKey = settingsGroup_ + "/quantity";
    const QString formatKey = settingsGroup_ + "/formatIndex";

    const int slaveId = settingsService_->value(slaveKey).toInt();
    const int startAddr = settingsService_->value(addrKey).toInt();
    const int quantity = settingsService_->value(qtyKey).toInt();
    const int formatIndex = settingsService_->value(formatKey).toInt();

    slaveIdEdit_->setValue(slaveId);
    addressEdit_->setValue(startAddr);
    quantityEdit_->setValue(quantity);
    if (formatIndex >= 0 && formatIndex < dataFormatBox_->count()) {
        dataFormatBox_->setCurrentIndex(formatIndex);
    }
    onFormatChanged();
}

void FunctionWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/slaveId", slaveIdEdit_->value());
    settingsService_->setValue(settingsGroup_ + "/startAddr", addressEdit_->value());
    settingsService_->setValue(settingsGroup_ + "/quantity", quantityEdit_->value());
    settingsService_->setValue(settingsGroup_ + "/formatIndex", dataFormatBox_->currentIndex());
}

void FunctionWidget::onReadClicked(uint8_t functionCode) {
    emit readRequested(functionCode, addressEdit_->value(), quantityEdit_->value(), slaveIdEdit_->value());
}

void FunctionWidget::onWriteClicked(uint8_t functionCode) {
    emit writeRequested(functionCode, addressEdit_->value(), writeDataEdit_->text(), dataFormatBox_->currentData().toString(), slaveIdEdit_->value());
}

void FunctionWidget::onRawSendClicked() {
    QString text = rawDataEdit_->toPlainText();
    text = text.remove(' ');
    text = text.remove('\n');
    text = text.remove('\r');
    
    QByteArray data = QByteArray::fromHex(text.toLatin1());
    emit rawSendRequested(data);
}

void FunctionWidget::onFormatChanged() {
    if (!dataFormatBox_ || !writeDataEdit_) return;
    
    // index 0: Hex, index 1: Decimal, index 2: Binary
    const int formatIndex = dataFormatBox_->currentIndex();
    const bool isBinary = (formatIndex == 2);
    
    // Update placeholder based on format
    if (formatIndex == 0) {
        writeDataEdit_->setPlaceholderText(tr("Space separated hex (e.g., 01 02)"));
    } else if (formatIndex == 1) {
        writeDataEdit_->setPlaceholderText(tr("Space separated decimal (e.g., 100 200)"));
    } else if (formatIndex == 2) {
        writeDataEdit_->setPlaceholderText(tr("Bit string (e.g., 1 1 0 1)"));
    }
    
    const QList<QPushButton*> others = {
        readBtn01_, readBtn02_, readBtn03_, readBtn04_,
        writeBtn06_, writeBtn10_
    };
    
    for (auto* btn : others) {
        if (btn) btn->setEnabled(!isBinary);
    }
    
    // Always enable 0x05 / 0x0F since they are coil-relative
    if (writeBtn05_) writeBtn05_->setEnabled(true);
    if (writeBtn0F_) writeBtn0F_->setEnabled(true);
}

void FunctionWidget::retranslateUi() {
    if (standardSection_) {
        standardSection_->setTitle(tr("Standard"));
    }
    if (rawSection_) {
        rawSection_->setTitle(tr("Raw"));
    }
    if (slaveIdLabel_) {
        slaveIdLabel_->setText(tr("Slave ID:"));
    }
    if (addressLabel_) {
        addressLabel_->setText(tr("Start Addr:"));
    }
    if (quantityLabel_) {
        quantityLabel_->setText(tr("Quantity:"));
    }
    if (writeDataLabel_) writeDataLabel_->setText(tr("Write Data:"));
    if (formatLabel_) formatLabel_->setText(tr("Format:"));
    
    // Refresh placeholders and button states based on selected format
    onFormatChanged();

    if (dataFormatBox_) {
        dataFormatBox_->setItemText(0, tr("Hex"));
        dataFormatBox_->setItemText(1, tr("Decimal"));
        dataFormatBox_->setItemText(2, tr("Binary"));
    }
    if (readBtn01_) {
        readBtn01_->setText(tr("Read Coils (0x01)"));
    }
    if (readBtn02_) {
        readBtn02_->setText(tr("Read Discrete Inputs (0x02)"));
    }
    if (readBtn03_) {
        readBtn03_->setText(tr("Read Holding Registers (0x03)"));
    }
    if (readBtn04_) {
        readBtn04_->setText(tr("Read Input Registers (0x04)"));
    }
    if (writeBtn05_) {
        writeBtn05_->setText(tr("Write Single Coil (0x05)"));
    }
    if (writeBtn06_) {
        writeBtn06_->setText(tr("Write Single Register (0x06)"));
    }
    if (writeBtn0F_) {
        writeBtn0F_->setText(tr("Write Multiple Coils (0x0F)"));
    }
    if (writeBtn10_) {
        writeBtn10_->setText(tr("Write Multiple Registers (0x10)"));
    }
    if (rawDataLabel_) {
        rawDataLabel_->setText(tr("Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):"));
    }
    if (rawSendBtn_) {
        rawSendBtn_->setText(tr("Send Raw"));
    }
    if (appendCrcBtn_) {
        appendCrcBtn_->setText(tr("Append CRC"));
    }
    if (addMbapBtn_) {
        addMbapBtn_->setText(tr("Add MBAP"));
    }
}

void FunctionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void FunctionWidget::onAppendCrcClicked() {
    if (!rawDataEdit_) return;
    QString input = rawDataEdit_->toPlainText();
    QByteArray data = modbus::base::ModbusDataHelper::parseHex(input);
    if (data.isEmpty()) return;
    
    uint16_t crc = modbus::base::calculateModbusRtuCrc(data);
    data.append(static_cast<char>(crc & 0xFF));        // Low byte first
    data.append(static_cast<char>((crc >> 8) & 0xFF)); // High byte second
    
    rawDataEdit_->setPlainText(data.toHex(' ').toUpper());
}

void FunctionWidget::onAddMbapClicked() {
    if (!rawDataEdit_) return;
    QString input = rawDataEdit_->toPlainText();
    QByteArray data = modbus::base::ModbusDataHelper::parseHex(input);
    if (data.isEmpty()) return;
    
    QByteArray mbap;
    mbap.resize(7);
    mbap[0] = 0; // Trans ID High
    mbap[1] = 0; // Trans ID Low
    mbap[2] = 0; // Proto ID High
    mbap[3] = 0; // Proto ID Low
    uint16_t len = static_cast<uint16_t>(data.size() + 1);
    mbap[4] = static_cast<char>((len >> 8) & 0xFF);
    mbap[5] = static_cast<char>(len & 0xFF);
    mbap[6] = static_cast<char>(getSlaveId());
    
    data.prepend(mbap);
    rawDataEdit_->setPlainText(data.toHex(' ').toUpper());
}

} // namespace ui::widgets
