#include "SerialConnectionWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSerialPortInfo>

namespace ui::widgets {

SerialConnectionWidget::SerialConnectionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    refreshPorts();
}

SerialConnectionWidget::~SerialConnectionWidget() = default;

io::SerialConfig SerialConnectionWidget::getConfig() const {
    io::SerialConfig config;
    config.portName = portCombo_->currentText();
    config.baudRate = baudCombo_->currentText().toInt();
    config.dataBits = dataBitsCombo_->currentText().toInt();
    
    // Stop Bits
    QString stopStr = stopBitsCombo_->currentText();
    if (stopStr == "1") config.stopBits = QSerialPort::OneStop;
    else if (stopStr == "1.5") config.stopBits = QSerialPort::OneAndHalfStop;
    else if (stopStr == "2") config.stopBits = QSerialPort::TwoStop;
    
    // Parity
    QString parityStr = parityCombo_->currentText();
    if (parityStr == "None") config.parity = QSerialPort::NoParity;
    else if (parityStr == "Even") config.parity = QSerialPort::EvenParity;
    else if (parityStr == "Odd") config.parity = QSerialPort::OddParity;
    else if (parityStr == "Space") config.parity = QSerialPort::SpaceParity;
    else if (parityStr == "Mark") config.parity = QSerialPort::MarkParity;

    return config;
}

void SerialConnectionWidget::setConnected(bool connected) {
    isConnected_ = connected;
    connectBtn_->setText(connected ? "Disconnect" : "Connect");
    
    portCombo_->setEnabled(!connected);
    baudCombo_->setEnabled(!connected);
    dataBitsCombo_->setEnabled(!connected);
    parityCombo_->setEnabled(!connected);
    stopBitsCombo_->setEnabled(!connected);
    refreshBtn_->setEnabled(!connected);
}

void SerialConnectionWidget::refreshPorts() {
    QString current = portCombo_->currentText();
    portCombo_->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        portCombo_->addItem(info.portName());
    }
    // Restore selection if possible
    int index = portCombo_->findText(current);
    if (index != -1) portCombo_->setCurrentIndex(index);
}

void SerialConnectionWidget::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Port
    layout->addWidget(new QLabel("Port:", this));
    portCombo_ = new QComboBox(this);
    portCombo_->setMinimumWidth(80);
    layout->addWidget(portCombo_);

    refreshBtn_ = new QPushButton("R", this);
    refreshBtn_->setFixedWidth(30);
    refreshBtn_->setToolTip("Refresh Ports");
    connect(refreshBtn_, &QPushButton::clicked, this, &SerialConnectionWidget::refreshPorts);
    layout->addWidget(refreshBtn_);

    // Baud
    layout->addWidget(new QLabel("Baud:", this));
    baudCombo_ = new QComboBox(this);
    baudCombo_->addItems({"9600", "19200", "38400", "57600", "115200"});
    baudCombo_->setCurrentText("9600");
    layout->addWidget(baudCombo_);

    // Data Bits
    layout->addWidget(new QLabel("Data:", this));
    dataBitsCombo_ = new QComboBox(this);
    dataBitsCombo_->addItems({"8", "7"});
    dataBitsCombo_->setCurrentText("8");
    dataBitsCombo_->setFixedWidth(50);
    layout->addWidget(dataBitsCombo_);

    // Parity
    layout->addWidget(new QLabel("Parity:", this));
    parityCombo_ = new QComboBox(this);
    parityCombo_->addItems({"None", "Even", "Odd", "Space", "Mark"});
    parityCombo_->setFixedWidth(70);
    layout->addWidget(parityCombo_);

    // Stop Bits
    layout->addWidget(new QLabel("Stop:", this));
    stopBitsCombo_ = new QComboBox(this);
    stopBitsCombo_->addItems({"1", "1.5", "2"});
    stopBitsCombo_->setFixedWidth(50);
    layout->addWidget(stopBitsCombo_);

    // Connect Button
    connectBtn_ = new QPushButton("Connect", this);
    connect(connectBtn_, &QPushButton::clicked, [this]() {
        if (isConnected_) {
            emit disconnectClicked();
        } else {
            emit connectClicked(getConfig());
        }
    });
    layout->addWidget(connectBtn_);
    
    layout->addStretch();
}

} // namespace ui::widgets
