#include "SerialConnectionWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSerialPortInfo>
#include <QEvent>
#include <QSettings>
#include <QApplication>
#include <QSignalBlocker>

namespace ui::widgets {

SerialConnectionWidget::SerialConnectionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    refreshPorts();
}

SerialConnectionWidget::~SerialConnectionWidget() = default;

io::SerialConfig SerialConnectionWidget::getConfig() const {
    io::SerialConfig config;
    config.portName = portCombo_->currentData().toString(); // Use user data for port name
    if (config.portName.isEmpty()) {
        config.portName = portCombo_->currentText(); // Fallback
    }
    
    bool ok;
    config.baudRate = baudCombo_->currentText().toInt(&ok);
    if (!ok) config.baudRate = 9600;
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
    connectBtn_->setText(connected ? tr("Disconnect") : tr("Connect"));
    
    portCombo_->setEnabled(!connected);
    baudCombo_->setEnabled(!connected);
    dataBitsCombo_->setEnabled(!connected);
    parityCombo_->setEnabled(!connected);
    stopBitsCombo_->setEnabled(!connected);
    refreshBtn_->setEnabled(!connected);
}

void SerialConnectionWidget::refreshPorts() {
    // Store current port name (not the full text)
    QString currentPortName = portCombo_->currentData().toString();
    if (currentPortName.isEmpty()) {
        currentPortName = portCombo_->currentText();
    }
    
    portCombo_->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QString itemText = info.portName();
        if (!info.description().isEmpty()) {
            itemText = info.portName() + " (" + info.description() + ")";
        }
        portCombo_->addItem(itemText, info.portName()); // Store port name as user data
    }
    
    // Restore selection if possible (match user data which is the real port name)
    int index = portCombo_->findData(currentPortName); 
    
    if (index != -1) {
        portCombo_->setCurrentIndex(index);
    } else if (portCombo_->count() > 0) {
        portCombo_->setCurrentIndex(0);
    }
}

void SerialConnectionWidget::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Port
    portLabel_ = new QLabel(this);
    layout->addWidget(portLabel_);
    portCombo_ = new QComboBox(this);
    portCombo_->setMinimumWidth(80);
    layout->addWidget(portCombo_);

    refreshBtn_ = new QPushButton("R", this);
    refreshBtn_->setFixedWidth(30);
    refreshBtn_->setToolTip(tr("Refresh Ports"));
    connect(refreshBtn_, &QPushButton::clicked, this, &SerialConnectionWidget::refreshPorts);
    layout->addWidget(refreshBtn_);

    // Baud
    baudLabel_ = new QLabel(this);
    layout->addWidget(baudLabel_);
    baudCombo_ = new QComboBox(this);
    baudCombo_->addItems({"9600", "19200", "38400", "57600", "115200"});
    baudCombo_->setCurrentText("9600");
    layout->addWidget(baudCombo_);

    // Data Bits
    dataBitsLabel_ = new QLabel(this);
    layout->addWidget(dataBitsLabel_);
    dataBitsCombo_ = new QComboBox(this);
    dataBitsCombo_->addItems({"8", "7"});
    dataBitsCombo_->setCurrentText("8");
    dataBitsCombo_->setFixedWidth(50);
    layout->addWidget(dataBitsCombo_);

    // Parity
    parityLabel_ = new QLabel(this);
    layout->addWidget(parityLabel_);
    parityCombo_ = new QComboBox(this);
    parityCombo_->addItems({"None", "Even", "Odd", "Space", "Mark"});
    parityCombo_->setFixedWidth(70);
    layout->addWidget(parityCombo_);

    // Stop Bits
    stopBitsLabel_ = new QLabel(this);
    layout->addWidget(stopBitsLabel_);
    stopBitsCombo_ = new QComboBox(this);
    stopBitsCombo_->addItems({"1", "1.5", "2"});
    stopBitsCombo_->setFixedWidth(50);
    layout->addWidget(stopBitsCombo_);

    // Connect Button
    connectBtn_ = new QPushButton(this);
    connect(connectBtn_, &QPushButton::clicked, [this]() {
        if (isConnected_) {
            emit disconnectClicked();
        } else {
            const auto config = getConfig();
            saveSettings();
            emit connectClicked(config);
        }
    });
    layout->addWidget(connectBtn_);
    
    layout->addStretch();

    connect(baudCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(dataBitsCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(parityCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(stopBitsCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);

    loadSettings();

    retranslateUi();
}

void SerialConnectionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    loadSettings();
}

void SerialConnectionWidget::loadSettings() {
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QSignalBlocker b1(baudCombo_);
    QSignalBlocker b2(dataBitsCombo_);
    QSignalBlocker b3(parityCombo_);
    QSignalBlocker b4(stopBitsCombo_);

    const QString baudKey = settingsGroup_ + "/baudRate";
    const QString dataKey = settingsGroup_ + "/dataBits";
    const QString parityKey = settingsGroup_ + "/parity";
    const QString stopKey = settingsGroup_ + "/stopBits";

    const int fallbackBaud = settings.value("serial/baudRate", baudCombo_->currentText().toInt()).toInt();
    const int baudRate = settings.value(baudKey, fallbackBaud).toInt();
    const QString dataBits = settings.value(dataKey, dataBitsCombo_->currentText()).toString();
    const QString parity = settings.value(parityKey, parityCombo_->currentText()).toString();
    const QString stopBits = settings.value(stopKey, stopBitsCombo_->currentText()).toString();

    const int baudIndex = baudCombo_->findText(QString::number(baudRate));
    if (baudIndex >= 0) {
        baudCombo_->setCurrentIndex(baudIndex);
    }

    const int dataIndex = dataBitsCombo_->findText(dataBits);
    if (dataIndex >= 0) {
        dataBitsCombo_->setCurrentIndex(dataIndex);
    }

    const int parityIndex = parityCombo_->findText(parity);
    if (parityIndex >= 0) {
        parityCombo_->setCurrentIndex(parityIndex);
    }

    const int stopIndex = stopBitsCombo_->findText(stopBits);
    if (stopIndex >= 0) {
        stopBitsCombo_->setCurrentIndex(stopIndex);
    }

    if (!settings.contains(baudKey)) settings.setValue(baudKey, baudRate);
    if (!settings.contains(dataKey)) settings.setValue(dataKey, dataBitsCombo_->currentText());
    if (!settings.contains(parityKey)) settings.setValue(parityKey, parityCombo_->currentText());
    if (!settings.contains(stopKey)) settings.setValue(stopKey, stopBitsCombo_->currentText());
}

void SerialConnectionWidget::saveSettings() {
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue(settingsGroup_ + "/baudRate", baudCombo_->currentText().toInt());
    settings.setValue(settingsGroup_ + "/dataBits", dataBitsCombo_->currentText());
    settings.setValue(settingsGroup_ + "/parity", parityCombo_->currentText());
    settings.setValue(settingsGroup_ + "/stopBits", stopBitsCombo_->currentText());
}

void SerialConnectionWidget::retranslateUi() {
    if (portLabel_) {
        portLabel_->setText(tr("Port:"));
    }
    if (baudLabel_) {
        baudLabel_->setText(tr("Baud:"));
    }
    if (dataBitsLabel_) {
        dataBitsLabel_->setText(tr("Data:"));
    }
    if (parityLabel_) {
        parityLabel_->setText(tr("Parity:"));
    }
    if (stopBitsLabel_) {
        stopBitsLabel_->setText(tr("Stop:"));
    }
    if (refreshBtn_) {
        refreshBtn_->setToolTip(tr("Refresh Ports"));
    }
    setConnected(isConnected_);
}

void SerialConnectionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
