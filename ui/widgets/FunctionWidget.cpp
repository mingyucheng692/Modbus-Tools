#include "FunctionWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>

#include <QTextEdit>
#include <QTabWidget>

namespace ui::widgets {

FunctionWidget::FunctionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

FunctionWidget::~FunctionWidget() = default;

void FunctionWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto tabWidget = new QTabWidget(this);
    
    // Tab 1: Standard
    auto standardTab = new QWidget();
    setupStandardUi(standardTab);
    tabWidget->addTab(standardTab, "Standard");

    // Tab 2: Raw
    auto rawTab = new QWidget();
    setupRawUi(rawTab);
    tabWidget->addTab(rawTab, "Raw");

    mainLayout->addWidget(tabWidget);
}

void FunctionWidget::setupStandardUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);
    
    // Row 1: Parameters
    auto paramLayout = new QHBoxLayout();
    
    paramLayout->addWidget(new QLabel("Slave ID:", parent));
    slaveIdEdit_ = new QSpinBox(parent);
    slaveIdEdit_->setRange(1, 247);
    slaveIdEdit_->setValue(1);
    paramLayout->addWidget(slaveIdEdit_);

    paramLayout->addWidget(new QLabel("Start Addr:", parent));
    addressEdit_ = new QSpinBox(parent);
    addressEdit_->setRange(0, 65535);
    addressEdit_->setValue(0);
    paramLayout->addWidget(addressEdit_);

    paramLayout->addWidget(new QLabel("Quantity:", parent));
    quantityEdit_ = new QSpinBox(parent);
    quantityEdit_->setRange(1, 125); // Typical Modbus Limit
    quantityEdit_->setValue(10);
    paramLayout->addWidget(quantityEdit_);
    
    paramLayout->addStretch();
    layout->addLayout(paramLayout);

    // Row 2: Write Data Input
    auto writeLayout = new QHBoxLayout();
    writeLayout->addWidget(new QLabel("Write Data:", parent));
    writeDataEdit_ = new QLineEdit(parent);
    writeDataEdit_->setPlaceholderText("Space separated hex (e.g., 01 02) or values");
    writeLayout->addWidget(writeDataEdit_);
    
    writeLayout->addWidget(new QLabel("Format:", parent));
    dataFormatBox_ = new QComboBox(parent);
    dataFormatBox_->addItems({"Hex", "Decimal", "ASCII"});
    writeLayout->addWidget(dataFormatBox_);
    
    layout->addLayout(writeLayout);

    // Row 3: Function Buttons
    auto btnLayout = new QGridLayout();
    
    // Read Buttons
    auto btn01 = new QPushButton("Read Coils (0x01)", parent);
    auto btn02 = new QPushButton("Read Discrete (0x02)", parent);
    auto btn03 = new QPushButton("Read Holding (0x03)", parent);
    auto btn04 = new QPushButton("Read Input (0x04)", parent);
    
    connect(btn01, &QPushButton::clicked, [this](){ onReadClicked(0x01); });
    connect(btn02, &QPushButton::clicked, [this](){ onReadClicked(0x02); });
    connect(btn03, &QPushButton::clicked, [this](){ onReadClicked(0x03); });
    connect(btn04, &QPushButton::clicked, [this](){ onReadClicked(0x04); });

    // Add buttons to layout
    btnLayout->addWidget(btn01, 0, 0);
    btnLayout->addWidget(btn02, 0, 1);
    btnLayout->addWidget(btn03, 0, 2);
    btnLayout->addWidget(btn04, 0, 3);

    // Write Buttons
    auto btn05 = new QPushButton("Write Coil (0x05)", parent);
    auto btn06 = new QPushButton("Write Reg (0x06)", parent);
    auto btn0F = new QPushButton("Write Multi Coils (0x0F)", parent);
    auto btn10 = new QPushButton("Write Multi Regs (0x10)", parent);

    connect(btn05, &QPushButton::clicked, [this](){ onWriteClicked(0x05); });
    connect(btn06, &QPushButton::clicked, [this](){ onWriteClicked(0x06); });
    connect(btn0F, &QPushButton::clicked, [this](){ onWriteClicked(0x0F); });
    connect(btn10, &QPushButton::clicked, [this](){ onWriteClicked(0x10); });

    btnLayout->addWidget(btn05, 1, 0);
    btnLayout->addWidget(btn06, 1, 1);
    btnLayout->addWidget(btn0F, 1, 2);
    btnLayout->addWidget(btn10, 1, 3);

    layout->addLayout(btnLayout);
    layout->addStretch();
}

void FunctionWidget::setupRawUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);

    layout->addWidget(new QLabel("Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):", parent));
    
    rawDataEdit_ = new QTextEdit(parent);
    layout->addWidget(rawDataEdit_);

    auto btnLayout = new QHBoxLayout();
    auto sendBtn = new QPushButton("Send Raw", parent);
    connect(sendBtn, &QPushButton::clicked, this, &FunctionWidget::onRawSendClicked);
    
    btnLayout->addStretch();
    btnLayout->addWidget(sendBtn);
    
    layout->addLayout(btnLayout);
}

int FunctionWidget::getSlaveId() const {
    return slaveIdEdit_ ? slaveIdEdit_->value() : 1;
}

void FunctionWidget::onReadClicked(uint8_t functionCode) {
    emit readRequested(functionCode, addressEdit_->value(), quantityEdit_->value(), slaveIdEdit_->value());
}

void FunctionWidget::onWriteClicked(uint8_t functionCode) {
    emit writeRequested(functionCode, addressEdit_->value(), writeDataEdit_->text(), dataFormatBox_->currentText(), slaveIdEdit_->value());
}

void FunctionWidget::onRawSendClicked() {
    QString text = rawDataEdit_->toPlainText();
    // Simple Hex parsing: remove spaces, convert to byte array
    text = text.remove(' ');
    text = text.remove('\n');
    text = text.remove('\r');
    
    QByteArray data = QByteArray::fromHex(text.toLatin1());
    if (!data.isEmpty()) {
        emit rawSendRequested(data);
    }
}

} // namespace ui::widgets
