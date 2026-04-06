#include "TcpConnectionWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

TcpConnectionWidget::TcpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
}

TcpConnectionWidget::~TcpConnectionWidget() = default;

void TcpConnectionWidget::setupUi() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    section_ = new CollapsibleSection(settingsService_, this);
    auto layout = new QHBoxLayout(section_->contentWidget());
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);

    // IP Address
    hostLabel_ = new QLabel(this);
    layout->addWidget(hostLabel_);
    ipEdit_ = new QLineEdit(this);
    ipEdit_->setMinimumWidth(112);
    ipEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(ipEdit_);

    // Port
    portLabel_ = new QLabel(this);
    layout->addWidget(portLabel_);
    portEdit_ = new QSpinBox(this);
    portEdit_->setRange(app::constants::Constants::Network::kMinTcpPort,
                        app::constants::Constants::Network::kMaxTcpPort);
    portEdit_->setValue(app::constants::Constants::Network::kDefaultModbusTcpPort);
    portEdit_->setFixedWidth(76);
    layout->addWidget(portEdit_);

    // Connect Button
    connectBtn_ = new QPushButton(this);
    connectBtn_->setMinimumWidth(84);
    connectBtn_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout->addWidget(connectBtn_);

    // Status
    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("color: red; font-weight: bold;");
    statusLabel_->setMinimumWidth(96);
    statusLabel_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    layout->addWidget(statusLabel_);

    layout->addStretch();
    mainLayout->addWidget(section_);

    // Connections
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isConnected_) {
            emit disconnectClicked();
        } else {
            saveSettings();
            emit connectClicked(ipEdit_->text(), portEdit_->value());
        }
    });
    connect(ipEdit_, &QLineEdit::textChanged, this, &TcpConnectionWidget::saveSettings);
    connect(portEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &TcpConnectionWidget::saveSettings);

    loadSettings();
    section_->setSettingsKey(settingsGroup_ + "/ui/connectionSettingsCollapsed");
    retranslateUi();
}

void TcpConnectionWidget::setDefaultPort(int port) {
    defaultPort_ = port;
    if (!portEdit_) return;
    if (settingsGroup_.isEmpty()) {
        portEdit_->setValue(port);
        return;
    }
    const QString key = settingsGroup_ + "/port";
    if (!settingsService_ || !settingsService_->contains(key)) {
        portEdit_->setValue(port);
        if (settingsService_) {
            settingsService_->setValue(key, port);
        }
    } else {
        portEdit_->setValue(settingsService_->value(key).toInt());
    }
}

void TcpConnectionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (section_) {
        section_->setSettingsKey(settingsGroup_ + "/ui/connectionSettingsCollapsed");
    }
    loadSettings();
}

void TcpConnectionWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(ipEdit_);
    QSignalBlocker b2(portEdit_);

    const QString ipKey = settingsGroup_ + "/ip";
    const QString portKey = settingsGroup_ + "/port";
    const QString ip = settingsService_->value(ipKey).toString();
    const int port = settingsService_->value(portKey).toInt();

    ipEdit_->setText(ip);
    portEdit_->setValue(port);
}

void TcpConnectionWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/ip", ipEdit_->text());
    settingsService_->setValue(settingsGroup_ + "/port", portEdit_->value());
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
    if (section_) {
        section_->setTitle(tr("Connection Settings"));
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
