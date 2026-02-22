#include "TcpConnectionWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>

namespace ui::widgets {

TcpConnectionWidget::TcpConnectionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TcpConnectionWidget::~TcpConnectionWidget() = default;

void TcpConnectionWidget::setupUi() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto groupBox = new QGroupBox("Connection Settings", this);
    auto layout = new QHBoxLayout(groupBox);

    // IP Address
    layout->addWidget(new QLabel("Host:", this));
    ipEdit_ = new QLineEdit("127.0.0.1", this);
    layout->addWidget(ipEdit_);

    // Port
    layout->addWidget(new QLabel("Port:", this));
    portEdit_ = new QSpinBox(this);
    portEdit_->setRange(1, 65535);
    portEdit_->setValue(502);
    layout->addWidget(portEdit_);

    // Connect Button
    connectBtn_ = new QPushButton("Connect", this);
    layout->addWidget(connectBtn_);

    // Status
    statusLabel_ = new QLabel("Disconnected", this);
    statusLabel_->setStyleSheet("color: red; font-weight: bold;");
    layout->addWidget(statusLabel_);

    layout->addStretch();
    mainLayout->addWidget(groupBox);

    // Connections
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isConnected_) {
            emit disconnectClicked();
        } else {
            emit connectClicked(ipEdit_->text(), portEdit_->value());
        }
    });
}

void TcpConnectionWidget::setDefaultPort(int port) {
    if (portEdit_) {
        portEdit_->setValue(port);
    }
}

QString TcpConnectionWidget::getIpAddress() const {
    return ipEdit_->text();
}

int TcpConnectionWidget::getPort() const {
    return portEdit_->value();
}

void TcpConnectionWidget::setConnected(bool connected) {
    isConnected_ = connected;
    if (connected) {
        connectBtn_->setText("Disconnect");
        statusLabel_->setText("Connected");
        statusLabel_->setStyleSheet("color: green; font-weight: bold;");
        ipEdit_->setEnabled(false);
        portEdit_->setEnabled(false);
    } else {
        connectBtn_->setText("Connect");
        statusLabel_->setText("Disconnected");
        statusLabel_->setStyleSheet("color: red; font-weight: bold;");
        ipEdit_->setEnabled(true);
        portEdit_->setEnabled(true);
    }
}

} // namespace ui::widgets
