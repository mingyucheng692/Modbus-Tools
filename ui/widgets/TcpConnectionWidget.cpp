#include "TcpConnectionWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QEvent>

namespace ui::widgets {

TcpConnectionWidget::TcpConnectionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TcpConnectionWidget::~TcpConnectionWidget() = default;

void TcpConnectionWidget::setupUi() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    groupBox_ = new QGroupBox(this);
    auto layout = new QHBoxLayout(groupBox_);

    // IP Address
    hostLabel_ = new QLabel(this);
    layout->addWidget(hostLabel_);
    ipEdit_ = new QLineEdit("127.0.0.1", this);
    layout->addWidget(ipEdit_);

    // Port
    portLabel_ = new QLabel(this);
    layout->addWidget(portLabel_);
    portEdit_ = new QSpinBox(this);
    portEdit_->setRange(1, 65535);
    portEdit_->setValue(502);
    layout->addWidget(portEdit_);

    // Connect Button
    connectBtn_ = new QPushButton(this);
    layout->addWidget(connectBtn_);

    // Status
    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("color: red; font-weight: bold;");
    layout->addWidget(statusLabel_);

    layout->addStretch();
    mainLayout->addWidget(groupBox_);

    // Connections
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isConnected_) {
            emit disconnectClicked();
        } else {
            emit connectClicked(ipEdit_->text(), portEdit_->value());
        }
    });

    retranslateUi();
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
        connectBtn_->setText(tr("Disconnect"));
        statusLabel_->setText(tr("Connected"));
        statusLabel_->setStyleSheet("color: green; font-weight: bold;");
        ipEdit_->setEnabled(false);
        portEdit_->setEnabled(false);
    } else {
        connectBtn_->setText(tr("Connect"));
        statusLabel_->setText(tr("Disconnected"));
        statusLabel_->setStyleSheet("color: red; font-weight: bold;");
        ipEdit_->setEnabled(true);
        portEdit_->setEnabled(true);
    }
}

void TcpConnectionWidget::retranslateUi() {
    if (groupBox_) {
        groupBox_->setTitle(tr("Connection Settings"));
    }
    if (hostLabel_) {
        hostLabel_->setText(tr("Host:"));
    }
    if (portLabel_) {
        portLabel_->setText(tr("Port:"));
    }
    setConnected(isConnected_);
}

void TcpConnectionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
