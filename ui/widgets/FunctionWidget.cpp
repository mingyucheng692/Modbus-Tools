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
#include <QEvent>

namespace ui::widgets {

FunctionWidget::FunctionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

FunctionWidget::~FunctionWidget() = default;

void FunctionWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    tabWidget_ = new QTabWidget(this);
    
    // Tab 1: Standard
    auto standardTab = new QWidget();
    setupStandardUi(standardTab);
    tabWidget_->addTab(standardTab, "");

    // Tab 2: Raw
    auto rawTab = new QWidget();
    setupRawUi(rawTab);
    tabWidget_->addTab(rawTab, "");

    mainLayout->addWidget(tabWidget_);
    retranslateUi();
}

void FunctionWidget::setupStandardUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);
    
    // Row 1: Parameters
    auto paramLayout = new QHBoxLayout();
    
    slaveIdLabel_ = new QLabel(parent);
    paramLayout->addWidget(slaveIdLabel_);
    slaveIdEdit_ = new QSpinBox(parent);
    slaveIdEdit_->setRange(1, 247);
    slaveIdEdit_->setValue(1);
    paramLayout->addWidget(slaveIdEdit_);

    addressLabel_ = new QLabel(parent);
    paramLayout->addWidget(addressLabel_);
    addressEdit_ = new QSpinBox(parent);
    addressEdit_->setRange(0, 65535);
    addressEdit_->setValue(0);
    paramLayout->addWidget(addressEdit_);

    quantityLabel_ = new QLabel(parent);
    paramLayout->addWidget(quantityLabel_);
    quantityEdit_ = new QSpinBox(parent);
    quantityEdit_->setRange(1, 125); // Typical Modbus Limit
    quantityEdit_->setValue(10);
    paramLayout->addWidget(quantityEdit_);
    
    paramLayout->addStretch();
    layout->addLayout(paramLayout);

    // Row 2: Write Data Input
    auto writeLayout = new QHBoxLayout();
    writeDataLabel_ = new QLabel(parent);
    writeLayout->addWidget(writeDataLabel_);
    writeDataEdit_ = new QLineEdit(parent);
    writeLayout->addWidget(writeDataEdit_);
    
    formatLabel_ = new QLabel(parent);
    writeLayout->addWidget(formatLabel_);
    dataFormatBox_ = new QComboBox(parent);
    dataFormatBox_->addItem("", "Hex");
    dataFormatBox_->addItem("", "Decimal");
    writeLayout->addWidget(dataFormatBox_);
    
    layout->addLayout(writeLayout);

    // Row 3: Function Buttons
    auto btnLayout = new QGridLayout();
    
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

    layout->addLayout(btnLayout);
    layout->addStretch();
}

void FunctionWidget::setupRawUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);

    rawDataLabel_ = new QLabel(parent);
    layout->addWidget(rawDataLabel_);
    
    rawDataEdit_ = new QTextEdit(parent);
    layout->addWidget(rawDataEdit_);

    auto btnLayout = new QHBoxLayout();
    rawSendBtn_ = new QPushButton(parent);
    connect(rawSendBtn_, &QPushButton::clicked, this, &FunctionWidget::onRawSendClicked);
    
    btnLayout->addStretch();
    btnLayout->addWidget(rawSendBtn_);
    
    layout->addLayout(btnLayout);
}

int FunctionWidget::getSlaveId() const {
    return slaveIdEdit_ ? slaveIdEdit_->value() : 1;
}

int FunctionWidget::getQuantity() const {
    return quantityEdit_ ? quantityEdit_->value() : 1;
}

void FunctionWidget::onReadClicked(uint8_t functionCode) {
    emit readRequested(functionCode, addressEdit_->value(), quantityEdit_->value(), slaveIdEdit_->value());
}

void FunctionWidget::onWriteClicked(uint8_t functionCode) {
    emit writeRequested(functionCode, addressEdit_->value(), writeDataEdit_->text(), dataFormatBox_->currentData().toString(), slaveIdEdit_->value());
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

void FunctionWidget::retranslateUi() {
    if (tabWidget_) {
        tabWidget_->setTabText(0, tr("Standard"));
        tabWidget_->setTabText(1, tr("Raw"));
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
    if (writeDataLabel_) {
        writeDataLabel_->setText(tr("Write Data:"));
    }
    if (writeDataEdit_) {
        writeDataEdit_->setPlaceholderText(tr("Space separated hex (e.g., 01 02) or values"));
    }
    if (formatLabel_) {
        formatLabel_->setText(tr("Format:"));
    }
    if (dataFormatBox_) {
        dataFormatBox_->setItemText(0, tr("Hex"));
        dataFormatBox_->setItemText(1, tr("Decimal"));
    }
    if (readBtn01_) {
        readBtn01_->setText(tr("Read Coils (0x01)"));
    }
    if (readBtn02_) {
        readBtn02_->setText(tr("Read Discrete (0x02)"));
    }
    if (readBtn03_) {
        readBtn03_->setText(tr("Read Holding (0x03)"));
    }
    if (readBtn04_) {
        readBtn04_->setText(tr("Read Input (0x04)"));
    }
    if (writeBtn05_) {
        writeBtn05_->setText(tr("Write Coil (0x05)"));
    }
    if (writeBtn06_) {
        writeBtn06_->setText(tr("Write Reg (0x06)"));
    }
    if (writeBtn0F_) {
        writeBtn0F_->setText(tr("Write Multi Coils (0x0F)"));
    }
    if (writeBtn10_) {
        writeBtn10_->setText(tr("Write Multi Regs (0x10)"));
    }
    if (rawDataLabel_) {
        rawDataLabel_->setText(tr("Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):"));
    }
    if (rawSendBtn_) {
        rawSendBtn_->setText(tr("Send Raw"));
    }
}

void FunctionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
