#include "ConnectionDock.h"
#include "ControlWidget.h"
#include "GenericSenderWidget.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSerialPortInfo>

ConnectionDock::ConnectionDock(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QGroupBox* connGroup = new QGroupBox("Connection Settings", this);
    QVBoxLayout* connLayout = new QVBoxLayout(connGroup);
    
    typeCombo_ = new QComboBox(this);
    typeCombo_->addItem("Modbus TCP");
    typeCombo_->addItem("Modbus RTU");
    typeCombo_->addItem("Generic TCP Client");
    // typeCombo_->addItem("Generic Serial"); // Future support
    
    settingsStack_ = new QStackedWidget(this);
    
    // 1. TCP Page
    tcpWidget_ = new QWidget(this);
    QFormLayout* tcpForm = new QFormLayout(tcpWidget_);
    tcpForm->setContentsMargins(0,0,0,0);
    ipEdit_ = new QLineEdit("127.0.0.1", this);
    portSpin_ = new QSpinBox(this);
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(502);
    tcpForm->addRow("IP:", ipEdit_);
    tcpForm->addRow("Port:", portSpin_);
    settingsStack_->addWidget(tcpWidget_);
    
    // 2. RTU Page
    rtuWidget_ = new QWidget(this);
    QFormLayout* rtuForm = new QFormLayout(rtuWidget_);
    rtuForm->setContentsMargins(0,0,0,0);
    
    portCombo_ = new QComboBox(this);
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        portCombo_->addItem(port.portName());
    }
    
    baudCombo_ = new QComboBox(this);
    baudCombo_->addItems({"9600", "19200", "38400", "57600", "115200"});
    baudCombo_->setCurrentText("9600");
    
    dataCombo_ = new QComboBox(this);
    dataCombo_->addItems({"8", "7"});
    
    stopCombo_ = new QComboBox(this);
    stopCombo_->addItems({"1", "1.5", "2"});
    
    parityCombo_ = new QComboBox(this);
    parityCombo_->addItem("None", 0);
    parityCombo_->addItem("Even", 2);
    parityCombo_->addItem("Odd", 3);
    
    rtuForm->addRow("Port:", portCombo_);
    rtuForm->addRow("Baud:", baudCombo_);
    rtuForm->addRow("Data:", dataCombo_);
    rtuForm->addRow("Stop:", stopCombo_);
    rtuForm->addRow("Parity:", parityCombo_);
    settingsStack_->addWidget(rtuWidget_);
    
    // 3. Generic TCP Page (Reuse TCP widget for now, or clone)
    // settingsStack_->addWidget(tcpWidget_); // Do NOT add again, it moves the widget!
    
    connLayout->addWidget(new QLabel("Type:"));
    connLayout->addWidget(typeCombo_);
    connLayout->addWidget(settingsStack_);
    
    connectBtn_ = new QPushButton("Connect", this);
    
    mainLayout->addWidget(connGroup);
    mainLayout->addWidget(connectBtn_);
    
    // Simulation Group
    QGroupBox* simGrp = new QGroupBox("Physical Simulation", this);
    QFormLayout* simLayout = new QFormLayout(simGrp);
    simDropSpin_ = new QSpinBox(this);
    simDropSpin_->setRange(0, 100);
    simDropSpin_->setSuffix("%");
    
    QHBoxLayout* delayLayout = new QHBoxLayout();
    simMinDelaySpin_ = new QSpinBox(this);
    simMinDelaySpin_->setRange(0, 5000);
    simMinDelaySpin_->setSuffix("ms");
    simMaxDelaySpin_ = new QSpinBox(this);
    simMaxDelaySpin_->setRange(0, 5000);
    simMaxDelaySpin_->setSuffix("ms");
    delayLayout->addWidget(new QLabel("Min:", this));
    delayLayout->addWidget(simMinDelaySpin_);
    delayLayout->addWidget(new QLabel("Max:", this));
    delayLayout->addWidget(simMaxDelaySpin_);
    
    simApplyBtn_ = new QPushButton("Apply Simulation", this);
    
    simLayout->addRow("Packet Loss:", simDropSpin_);
    simLayout->addRow("Latency:", delayLayout);
    simLayout->addWidget(simApplyBtn_);
    
    mainLayout->addWidget(simGrp);
    
    // Add Control Widget
    senderStack_ = new QStackedWidget(this);
    controlWidget_ = new ControlWidget(this);
    genericSender_ = new GenericSenderWidget(this);
    
    senderStack_->addWidget(controlWidget_);
    senderStack_->addWidget(genericSender_);
    
    mainLayout->addWidget(senderStack_);
    
    mainLayout->addStretch();
    
    connect(typeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionDock::onTypeChanged);
    connect(connectBtn_, &QPushButton::clicked, this, &ConnectionDock::onConnectClicked);
    connect(simApplyBtn_, &QPushButton::clicked, this, &ConnectionDock::onSimApplyClicked);
    
    connect(controlWidget_, &ControlWidget::sendRequest, this, &ConnectionDock::sendRequest);
    connect(controlWidget_, &ControlWidget::pollToggled, this, &ConnectionDock::pollToggled);
    connect(genericSender_, &GenericSenderWidget::sendRaw, this, &ConnectionDock::sendRaw);
}

void ConnectionDock::onTypeChanged(int index) {
    // 0: Modbus TCP, 1: Modbus RTU, 2: Generic TCP
    if (index == 2) {
        settingsStack_->setCurrentWidget(tcpWidget_); // Reuse TCP settings
        senderStack_->setCurrentWidget(genericSender_);
        emit modeChanged(1); // Generic
    } else if (index == 0) {
        settingsStack_->setCurrentWidget(tcpWidget_);
        senderStack_->setCurrentWidget(controlWidget_);
        emit modeChanged(0); // Modbus
    } else { // RTU
        settingsStack_->setCurrentWidget(rtuWidget_);
        senderStack_->setCurrentWidget(controlWidget_);
        emit modeChanged(0); // Modbus
    }
}

void ConnectionDock::onSimApplyClicked() {
    emit simulationChanged(simDropSpin_->value(), simMinDelaySpin_->value(), simMaxDelaySpin_->value());
}

void ConnectionDock::onConnectClicked() {
    if (!isConnected_) {
        int idx = typeCombo_->currentIndex();
        if (idx == 0 || idx == 2) { // TCP or Generic TCP
            emit connectTcp(ipEdit_->text(), portSpin_->value());
        } else {
            // RTU
            emit connectRtu(
                portCombo_->currentText(),
                baudCombo_->currentText().toInt(),
                dataCombo_->currentText().toInt(),
                stopCombo_->currentText().toInt(), // Simplified mapping
                parityCombo_->currentData().toInt()
            );
        }
        connectBtn_->setText("Disconnect");
        isConnected_ = true;
        typeCombo_->setEnabled(false);
    } else {
        emit disconnectRequested();
        connectBtn_->setText("Connect");
        isConnected_ = false;
        typeCombo_->setEnabled(true);
    }
}
