#include "TcpConnectionWidget.h"
#include "CollapsibleSection.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QEvent>
#include <QSettings>
#include <QApplication>
#include <QSignalBlocker>

namespace ui::widgets {

TcpConnectionWidget::TcpConnectionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TcpConnectionWidget::~TcpConnectionWidget() = default;

void TcpConnectionWidget::setupUi() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    section_ = new CollapsibleSection(this);
    auto layout = new QHBoxLayout(section_->contentWidget());

    // IP Address
    hostLabel_ = new QLabel(this);
    layout->addWidget(hostLabel_);
    ipEdit_ = new QLineEdit(this);
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
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    const QString key = settingsGroup_ + "/port";
    if (!settings.contains(key)) {
        portEdit_->setValue(port);
        settings.setValue(key, port);
    } else {
        portEdit_->setValue(settings.value(key, port).toInt());
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
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QSignalBlocker b1(ipEdit_);
    QSignalBlocker b2(portEdit_);

    const QString ipKey = settingsGroup_ + "/ip";
    const QString portKey = settingsGroup_ + "/port";
    const QString ip = settings.value(ipKey, "127.0.0.1").toString();
    const int port = settings.value(portKey, defaultPort_).toInt();

    ipEdit_->setText(ip);
    portEdit_->setValue(port);

    if (!settings.contains(ipKey)) settings.setValue(ipKey, ip);
    if (!settings.contains(portKey)) settings.setValue(portKey, port);
}

void TcpConnectionWidget::saveSettings() {
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue(settingsGroup_ + "/ip", ipEdit_->text());
    settings.setValue(settingsGroup_ + "/port", portEdit_->value());
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
