#include "ControlWidget.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>

ControlWidget::ControlWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QGroupBox* grp = new QGroupBox("Modbus Function", this);
    QFormLayout* form = new QFormLayout(grp);
    
    slaveIdSpin_ = new QSpinBox(this);
    slaveIdSpin_->setRange(0, 255);
    slaveIdSpin_->setValue(1);
    
    funcCodeCombo_ = new QComboBox(this);
    funcCodeCombo_->addItem("01 Read Coils", 1);
    funcCodeCombo_->addItem("02 Read Discrete Inputs", 2);
    funcCodeCombo_->addItem("03 Read Holding Registers", 3);
    funcCodeCombo_->addItem("04 Read Input Registers", 4);
    funcCodeCombo_->addItem("05 Write Single Coil", 5);
    funcCodeCombo_->addItem("06 Write Single Register", 6);
    funcCodeCombo_->addItem("0F Write Multiple Coils", 15);
    funcCodeCombo_->addItem("10 Write Multiple Registers", 16);
    funcCodeCombo_->setCurrentIndex(2); // 03 Default
    
    startAddrSpin_ = new QSpinBox(this);
    startAddrSpin_->setRange(0, 65535);
    
    countSpin_ = new QSpinBox(this);
    countSpin_->setRange(1, 2000);
    countSpin_->setValue(10);
    
    dataInput_ = new QLineEdit(this);
    dataInput_->setPlaceholderText("Hex Data (e.g. 01 02 FF)");
    
    // form->addRow("Slave ID:", slaveIdSpin_); // Managed externally by ConnectionDock
    slaveIdSpin_->hide();

    form->addRow("Function:", funcCodeCombo_);
    form->addRow("Address:", startAddrSpin_);
    form->addRow("Count/Val:", countSpin_);
    form->addRow("Data (Hex):", dataInput_);
    
    // Poll Controls
    QHBoxLayout* pollLayout = new QHBoxLayout();
    pollCheck_ = new QCheckBox("Auto Poll", this);
    pollIntervalSpin_ = new QSpinBox(this);
    pollIntervalSpin_->setRange(10, 60000);
    pollIntervalSpin_->setValue(1000);
    pollIntervalSpin_->setSuffix(" ms");
    
    pollLayout->addWidget(pollCheck_);
    pollLayout->addWidget(pollIntervalSpin_);
    
    sendBtn_ = new QPushButton("Send Request", this);
    
    mainLayout->addWidget(grp);
    mainLayout->addLayout(pollLayout);
    mainLayout->addWidget(sendBtn_);
    mainLayout->addStretch();
    
    connect(sendBtn_, &QPushButton::clicked, this, &ControlWidget::onSendClicked);
    connect(pollCheck_, &QCheckBox::toggled, this, &ControlWidget::onPollToggled);
    connect(funcCodeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ControlWidget::updateUiState);
    
    updateUiState();
}

void ControlWidget::setSlaveId(int id) {
    if (slaveIdSpin_) {
        slaveIdSpin_->setValue(id);
    }
}

void ControlWidget::updateUiState() {
    int fc = funcCodeCombo_->currentData().toInt();
    bool isWrite = (fc == 5 || fc == 6 || fc == 15 || fc == 16);
    bool isMultiWrite = (fc == 15 || fc == 16);
    
    dataInput_->setEnabled(isWrite);
    if (!isWrite) {
        dataInput_->clear();
    }
    
    // Adjust label for count/value
    // For 05/06, countSpin can act as the value (0xFF00 or val) if dataInput is empty?
    // But strictly, 05/06 takes a value. 
    // We will let user input value in dataInput for 05/06, or use countSpin for read count.
}

void ControlWidget::onSendClicked() {
    int slaveId = slaveIdSpin_->value();
    int fc = funcCodeCombo_->currentData().toInt();
    int addr = startAddrSpin_->value();
    int count = countSpin_->value();
    QString data = dataInput_->text();
    
    emit sendRequest(slaveId, fc, addr, count, data);
}

void ControlWidget::onPollToggled(bool checked) {
    emit pollToggled(checked, pollIntervalSpin_->value());
    sendBtn_->setEnabled(!checked);
    // Disable inputs while polling? Maybe.
}
