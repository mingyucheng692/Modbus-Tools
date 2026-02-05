#include "ConnectionDock.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ConnectionDock::ConnectionDock(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QGroupBox* tcpGroup = new QGroupBox("Connection Settings", this);
    QFormLayout* form = new QFormLayout(tcpGroup);
    
    typeCombo_ = new QComboBox(this);
    typeCombo_->addItem("Modbus TCP");
    typeCombo_->addItem("Modbus RTU"); // Placeholder
    
    ipEdit_ = new QLineEdit("127.0.0.1", this);
    portSpin_ = new QSpinBox(this);
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(502);
    
    form->addRow("Type:", typeCombo_);
    form->addRow("IP:", ipEdit_);
    form->addRow("Port:", portSpin_);
    
    connectBtn_ = new QPushButton("Connect", this);
    
    mainLayout->addWidget(tcpGroup);
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
    controlWidget_ = new ControlWidget(this);
    mainLayout->addWidget(controlWidget_);
    
    mainLayout->addStretch();
    
    connect(connectBtn_, &QPushButton::clicked, this, &ConnectionDock::onConnectClicked);
    connect(simApplyBtn_, &QPushButton::clicked, this, &ConnectionDock::onSimApplyClicked);
    
    connect(controlWidget_, &ControlWidget::sendRequest, this, &ConnectionDock::sendRequest);
    connect(controlWidget_, &ControlWidget::pollToggled, this, &ConnectionDock::pollToggled);
}

void ConnectionDock::onSimApplyClicked() {
    emit simulationChanged(simDropSpin_->value(), simMinDelaySpin_->value(), simMaxDelaySpin_->value());
}

void ConnectionDock::onConnectClicked() {
    if (!isConnected_) {
        emit connectTcp(ipEdit_->text(), portSpin_->value());
        connectBtn_->setText("Disconnect");
        isConnected_ = true;
    } else {
        emit disconnectRequested();
        connectBtn_->setText("Connect");
        isConnected_ = false;
    }
}
