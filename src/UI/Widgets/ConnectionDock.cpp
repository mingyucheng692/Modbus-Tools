#include "ConnectionDock.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

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
    mainLayout->addStretch();
    
    connect(connectBtn_, &QPushButton::clicked, this, &ConnectionDock::onConnectClicked);
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
