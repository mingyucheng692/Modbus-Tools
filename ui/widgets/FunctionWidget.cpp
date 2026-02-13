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

namespace ui::widgets {

FunctionWidget::FunctionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

FunctionWidget::~FunctionWidget() = default;

void FunctionWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto groupBox = new QGroupBox("Modbus Functions", this);
    auto layout = new QVBoxLayout(groupBox);

    // Row 1: Parameters
    auto paramLayout = new QHBoxLayout();
    
    paramLayout->addWidget(new QLabel("Slave ID:", this));
    slaveIdEdit_ = new QSpinBox(this);
    slaveIdEdit_->setRange(1, 247);
    slaveIdEdit_->setValue(1);
    paramLayout->addWidget(slaveIdEdit_);

    paramLayout->addWidget(new QLabel("Start Addr:", this));
    addressEdit_ = new QSpinBox(this);
    addressEdit_->setRange(0, 65535);
    addressEdit_->setValue(0);
    paramLayout->addWidget(addressEdit_);

    paramLayout->addWidget(new QLabel("Quantity:", this));
    quantityEdit_ = new QSpinBox(this);
    quantityEdit_->setRange(1, 125); // Typical Modbus Limit
    quantityEdit_->setValue(10);
    paramLayout->addWidget(quantityEdit_);
    
    paramLayout->addStretch();
    layout->addLayout(paramLayout);

    // Row 2: Write Data Input
    auto writeLayout = new QHBoxLayout();
    writeLayout->addWidget(new QLabel("Write Data:", this));
    writeDataEdit_ = new QLineEdit(this);
    writeDataEdit_->setPlaceholderText("Space separated hex (e.g., 01 02) or values");
    writeLayout->addWidget(writeDataEdit_);
    
    writeLayout->addWidget(new QLabel("Format:", this));
    dataFormatBox_ = new QComboBox(this);
    dataFormatBox_->addItems({"Hex", "Decimal", "ASCII"});
    writeLayout->addWidget(dataFormatBox_);
    
    layout->addLayout(writeLayout);

    // Row 3: Function Buttons
    auto btnLayout = new QGridLayout();
    
    // Read Buttons
    auto btn01 = new QPushButton("Read Coils (0x01)", this);
    auto btn02 = new QPushButton("Read Discrete (0x02)", this);
    auto btn03 = new QPushButton("Read Holding (0x03)", this);
    auto btn04 = new QPushButton("Read Input (0x04)", this);
    
    connect(btn01, &QPushButton::clicked, [this](){ onReadClicked(0x01); });
    connect(btn02, &QPushButton::clicked, [this](){ onReadClicked(0x02); });
    connect(btn03, &QPushButton::clicked, [this](){ onReadClicked(0x03); });
    connect(btn04, &QPushButton::clicked, [this](){ onReadClicked(0x04); });

    btnLayout->addWidget(btn01, 0, 0);
    btnLayout->addWidget(btn02, 0, 1);
    btnLayout->addWidget(btn03, 0, 2);
    btnLayout->addWidget(btn04, 0, 3);

    // Write Buttons
    auto btn05 = new QPushButton("Write Coil (0x05)", this);
    auto btn06 = new QPushButton("Write Reg (0x06)", this);
    auto btn0F = new QPushButton("Write Multi Coils (0x0F)", this);
    auto btn10 = new QPushButton("Write Multi Regs (0x10)", this);

    connect(btn05, &QPushButton::clicked, [this](){ onWriteClicked(0x05); });
    connect(btn06, &QPushButton::clicked, [this](){ onWriteClicked(0x06); });
    connect(btn0F, &QPushButton::clicked, [this](){ onWriteClicked(0x0F); });
    connect(btn10, &QPushButton::clicked, [this](){ onWriteClicked(0x10); });

    btnLayout->addWidget(btn05, 1, 0);
    btnLayout->addWidget(btn06, 1, 1);
    btnLayout->addWidget(btn0F, 1, 2);
    btnLayout->addWidget(btn10, 1, 3);

    layout->addLayout(btnLayout);
    
    mainLayout->addWidget(groupBox);
}

void FunctionWidget::onReadClicked(uint8_t functionCode) {
    emit readRequested(functionCode, addressEdit_->value(), quantityEdit_->value());
}

void FunctionWidget::onWriteClicked(uint8_t functionCode) {
    emit writeRequested(functionCode, addressEdit_->value(), writeDataEdit_->text(), dataFormatBox_->currentText());
}

} // namespace ui::widgets
