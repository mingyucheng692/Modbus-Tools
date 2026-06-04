/**
 * @file SerialConnectionWidget.cpp
 * @brief Implementation of SerialConnectionWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SerialConnectionWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QSerialPortInfo>
#include <QEvent>
#include <QSignalBlocker>

namespace ui::widgets {

namespace {

constexpr auto kParityNone = "none";
constexpr auto kParityEven = "even";
constexpr auto kParityOdd = "odd";
constexpr auto kParitySpace = "space";
constexpr auto kParityMark = "mark";
constexpr auto kFlowNone = "none";
constexpr auto kFlowRtsCts = "rts_cts";
constexpr auto kFlowXonXoff = "xon_xoff";
constexpr auto kSerialContext = "ui::widgets::SerialConnectionWidget";
constexpr auto kNoneText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "None");
constexpr auto kEvenText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Even");
constexpr auto kOddText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Odd");
constexpr auto kSpaceText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Space");
constexpr auto kMarkText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Mark");
constexpr auto kRtsCtsText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "RTS/CTS");
constexpr auto kXonXoffText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "XON/XOFF");

QString trSerialOption(const char* sourceText)
{
    return QCoreApplication::translate(kSerialContext, sourceText);
}

void repopulateParityOptions(QComboBox* combo)
{
    if (!combo) {
        return;
    }

    const QString currentValue = combo->currentData().toString();
    QSignalBlocker blocker(combo);
    combo->clear();
    combo->addItem(trSerialOption(kNoneText), QString::fromLatin1(kParityNone));
    combo->addItem(trSerialOption(kEvenText), QString::fromLatin1(kParityEven));
    combo->addItem(trSerialOption(kOddText), QString::fromLatin1(kParityOdd));
    combo->addItem(trSerialOption(kSpaceText), QString::fromLatin1(kParitySpace));
    combo->addItem(trSerialOption(kMarkText), QString::fromLatin1(kParityMark));

    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

void repopulateFlowControlOptions(QComboBox* combo)
{
    if (!combo) {
        return;
    }

    const QString currentValue = combo->currentData().toString();
    QSignalBlocker blocker(combo);
    combo->clear();
    combo->addItem(trSerialOption(kNoneText), QString::fromLatin1(kFlowNone));
    combo->addItem(trSerialOption(kRtsCtsText), QString::fromLatin1(kFlowRtsCts));
    combo->addItem(trSerialOption(kXonXoffText), QString::fromLatin1(kFlowXonXoff));

    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

bool locksInputs(SerialConnectionWidget::DisplayState state)
{
    return state != SerialConnectionWidget::DisplayState::Disconnected;
}

bool usesDisconnectAction(SerialConnectionWidget::DisplayState state)
{
    switch (state) {
    case SerialConnectionWidget::DisplayState::Connecting:
    case SerialConnectionWidget::DisplayState::TransportConnected:
    case SerialConnectionWidget::DisplayState::Connected:
        return true;
    case SerialConnectionWidget::DisplayState::Disconnected:
    case SerialConnectionWidget::DisplayState::Disconnecting:
        return false;
    }

    return false;
}

} // namespace

SerialConnectionWidget::SerialConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
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
    if (!ok) config.baudRate = app::constants::Values::Serial::kDefaultBaudRate;
    config.dataBits = dataBitsCombo_->currentText().toInt();
    
    // Stop Bits
    QString stopStr = stopBitsCombo_->currentText();
    if (stopStr == "1") config.stopBits = QSerialPort::OneStop;
    else if (stopStr == "1.5") config.stopBits = QSerialPort::OneAndHalfStop;
    else if (stopStr == "2") config.stopBits = QSerialPort::TwoStop;
    
    // Parity
    const QString parityValue = parityCombo_->currentData().toString();
    if (parityValue == QLatin1String(kParityEven)) config.parity = QSerialPort::EvenParity;
    else if (parityValue == QLatin1String(kParityOdd)) config.parity = QSerialPort::OddParity;
    else if (parityValue == QLatin1String(kParitySpace)) config.parity = QSerialPort::SpaceParity;
    else if (parityValue == QLatin1String(kParityMark)) config.parity = QSerialPort::MarkParity;
    else config.parity = QSerialPort::NoParity;

    // Flow Control
    const QString flowValue = flowControlCombo_->currentData().toString();
    if (flowValue == QLatin1String(kFlowRtsCts)) config.flowControl = QSerialPort::HardwareControl;
    else if (flowValue == QLatin1String(kFlowXonXoff)) config.flowControl = QSerialPort::SoftwareControl;
    else config.flowControl = QSerialPort::NoFlowControl;

    return config;
}

void SerialConnectionWidget::setConnected(bool connected) {
    setDisplayState(connected ? DisplayState::Connected : DisplayState::Disconnected);
}

void SerialConnectionWidget::setDisplayState(DisplayState state)
{
    displayState_ = state;
    isConnected_ = (state == DisplayState::TransportConnected
                    || state == DisplayState::Connected);
    applyDisplayState();
}

void SerialConnectionWidget::applyDisplayState()
{
    QString buttonText;
    QString statusText;
    QString statusStyle = QStringLiteral("color: red; font-weight: bold;");

    switch (displayState_) {
    case DisplayState::Disconnected:
        buttonText = tr("Connect");
        statusText = tr("Disconnected");
        break;
    case DisplayState::Connecting:
        buttonText = tr("Disconnect");
        statusText = tr("Connecting");
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    case DisplayState::TransportConnected:
        buttonText = tr("Disconnect");
        statusText = tr("Transport Connected");
        statusStyle = QStringLiteral("color: #1f6feb; font-weight: bold;");
        break;
    case DisplayState::Connected:
        buttonText = tr("Disconnect");
        statusText = tr("Connected");
        statusStyle = QStringLiteral("color: green; font-weight: bold;");
        break;
    case DisplayState::Disconnecting:
        buttonText = tr("Disconnecting");
        statusText = tr("Disconnecting");
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    }

    connectBtn_->setText(buttonText);
    statusLabel_->setText(statusText);
    statusLabel_->setStyleSheet(statusStyle);

    const bool inputsEnabled = !locksInputs(displayState_);
    portCombo_->setEnabled(inputsEnabled);
    baudCombo_->setEnabled(inputsEnabled);
    dataBitsCombo_->setEnabled(inputsEnabled);
    parityCombo_->setEnabled(inputsEnabled);
    stopBitsCombo_->setEnabled(inputsEnabled);
    flowControlCombo_->setEnabled(inputsEnabled);
    refreshBtn_->setEnabled(inputsEnabled);
    autoReconnectCheck_->setEnabled(inputsEnabled);
    reconnectDelaySpin_->setEnabled(inputsEnabled);
    connectBtn_->setEnabled(displayState_ != DisplayState::Disconnecting);
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
    saveSettings();
}

void SerialConnectionWidget::setupUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    section_ = new CollapsibleSection(settingsService_, this);
    auto* layout = new QHBoxLayout(section_->contentWidget());

    // Port
    portLabel_ = new QLabel(this);
    layout->addWidget(portLabel_);
    portCombo_ = new QComboBox(this);
    portCombo_->setMinimumWidth(64);
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
    baudCombo_->addItems({QString::fromLatin1(app::constants::Values::Serial::kDefaultBaudRateText),
                          QStringLiteral("19200"),
                          QStringLiteral("38400"),
                          QStringLiteral("57600"),
                          QStringLiteral("115200")});
    baudCombo_->setCurrentText(QString::fromLatin1(app::constants::Values::Serial::kDefaultBaudRateText));
    layout->addWidget(baudCombo_);

    // Data Bits
    dataBitsLabel_ = new QLabel(this);
    layout->addWidget(dataBitsLabel_);
    dataBitsCombo_ = new QComboBox(this);
    dataBitsCombo_->addItems({QString::fromLatin1(app::constants::Values::Serial::kDefaultDataBitsText), QStringLiteral("7")});
    dataBitsCombo_->setCurrentText(QString::fromLatin1(app::constants::Values::Serial::kDefaultDataBitsText));
    dataBitsCombo_->setMinimumWidth(40);
    layout->addWidget(dataBitsCombo_);

    // Parity
    parityLabel_ = new QLabel(this);
    layout->addWidget(parityLabel_);
    parityCombo_ = new QComboBox(this);
    parityCombo_->setMinimumWidth(58);
    layout->addWidget(parityCombo_);

    // Stop Bits
    stopBitsLabel_ = new QLabel(this);
    layout->addWidget(stopBitsLabel_);
    stopBitsCombo_ = new QComboBox(this);
    stopBitsCombo_->addItems({QString::fromLatin1(app::constants::Values::Serial::kDefaultStopBitsText),
                              QStringLiteral("1.5"),
                              QStringLiteral("2")});
    stopBitsCombo_->setMinimumWidth(40);
    layout->addWidget(stopBitsCombo_);

    // Flow Control
    flowControlLabel_ = new QLabel(this);
    layout->addWidget(flowControlLabel_);
    flowControlCombo_ = new QComboBox(this);
    flowControlCombo_->setMinimumWidth(80);
    layout->addWidget(flowControlCombo_);

    autoReconnectCheck_ = new QCheckBox(this);
    layout->addWidget(autoReconnectCheck_);

    reconnectDelaySpin_ = new QSpinBox(this);
    reconnectDelaySpin_->setRange(500, 30000);
    reconnectDelaySpin_->setValue(3000);
    reconnectDelaySpin_->setSuffix(QStringLiteral("ms"));
    reconnectDelaySpin_->setFixedWidth(76);
    layout->addWidget(reconnectDelaySpin_);

    layout->addSpacing(4);

    // Connect Button
    connectBtn_ = new QPushButton(this);
    connect(connectBtn_, &QPushButton::clicked, [this]() {
        if (usesDisconnectAction(displayState_)) {
            emit disconnectClicked();
        } else {
            const auto config = getConfig();
            saveSettings();
            emit connectClicked(config);
        }
    });
    layout->addWidget(connectBtn_);

    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("color: red; font-weight: bold;");
    layout->addWidget(statusLabel_);
    
    layout->addStretch();
    mainLayout->addWidget(section_);

    connect(portCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &SerialConnectionWidget::saveSettings);
    connect(baudCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(dataBitsCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(parityCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(stopBitsCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(flowControlCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(autoReconnectCheck_, &QCheckBox::toggled, this, &SerialConnectionWidget::saveSettings);
    connect(autoReconnectCheck_, &QCheckBox::toggled, this, [this]() {
        reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
    });
    connect(reconnectDelaySpin_, qOverload<int>(&QSpinBox::valueChanged), this, &SerialConnectionWidget::saveSettings);

    repopulateParityOptions(parityCombo_);
    repopulateFlowControlOptions(flowControlCombo_);
    loadSettings();
    section_->setSettingsKey(settingsGroup_ + "/ui/connectionSettingsCollapsed");

    retranslateUi();
}

void SerialConnectionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (section_) {
        section_->setSettingsKey(settingsGroup_ + "/ui/connectionSettingsCollapsed");
    }
    loadSettings();
}

void SerialConnectionWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(baudCombo_);
    QSignalBlocker b2(dataBitsCombo_);
    QSignalBlocker b3(parityCombo_);
    QSignalBlocker b4(stopBitsCombo_);
    QSignalBlocker b5(autoReconnectCheck_);
    QSignalBlocker b6(reconnectDelaySpin_);
    QSignalBlocker b7(flowControlCombo_);

    const QString baudKey = settingsGroup_ + "/baudRate";
    const QString dataKey = settingsGroup_ + "/dataBits";
    const QString parityKey = settingsGroup_ + "/parity";
    const QString stopKey = settingsGroup_ + "/stopBits";
    const QString portKey = settingsGroup_ + "/portName";
    const QString flowControlKey = settingsGroup_ + "/flowControl";
    const QString autoReconnectKey = settingsGroup_ + "/autoReconnect";
    const QString reconnectDelayKey = settingsGroup_ + "/reconnectDelay";

    const int fallbackBaud = settingsService_->value("serial/baudRate").toInt();
    const int baudRate = settingsService_->contains(baudKey) ? settingsService_->value(baudKey).toInt() : fallbackBaud;
    const QString dataBits = settingsService_->value(dataKey).toString();
    const QString parity = settingsService_->value(parityKey).toString();
    const QString stopBits = settingsService_->value(stopKey).toString();
    const QString portName = settingsService_->value(portKey).toString();

    const int baudIndex = baudCombo_->findText(QString::number(baudRate));
    if (baudIndex >= 0) {
        baudCombo_->setCurrentIndex(baudIndex);
    }

    const int dataIndex = dataBitsCombo_->findText(dataBits);
    if (dataIndex >= 0) {
        dataBitsCombo_->setCurrentIndex(dataIndex);
    }

    int parityIndex = parityCombo_->findData(parity);
    if (parityIndex < 0) {
        parityIndex = parityCombo_->findText(parity);
    }
    if (parityIndex >= 0) {
        parityCombo_->setCurrentIndex(parityIndex);
    }

    const int stopIndex = stopBitsCombo_->findText(stopBits);
    if (stopIndex >= 0) {
        stopBitsCombo_->setCurrentIndex(stopIndex);
    }

    const QString flowControl = settingsService_->value(flowControlKey).toString();
    int flowIndex = flowControl.isEmpty() ? 0 : flowControlCombo_->findData(flowControl);
    if (flowIndex < 0 && !flowControl.isEmpty()) {
        flowIndex = flowControlCombo_->findText(flowControl);
    }
    if (flowIndex >= 0) {
        flowControlCombo_->setCurrentIndex(flowIndex);
    }

    if (!portName.isEmpty()) {
        const int portIndex = portCombo_->findData(portName);
        if (portIndex >= 0) {
            portCombo_->setCurrentIndex(portIndex);
        }
    }

    const bool autoReconnect = settingsService_->contains(autoReconnectKey)
        ? settingsService_->value(autoReconnectKey).toBool() : false;
    const int reconnectDelay = settingsService_->contains(reconnectDelayKey)
        ? settingsService_->value(reconnectDelayKey).toInt() : 3000;
    autoReconnectCheck_->setChecked(autoReconnect);
    reconnectDelaySpin_->setValue(reconnectDelay);
}

void SerialConnectionWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/baudRate", baudCombo_->currentText());
    settingsService_->setValue(settingsGroup_ + "/dataBits", dataBitsCombo_->currentText());
    settingsService_->setValue(settingsGroup_ + "/parity", parityCombo_->currentData().toString());
    settingsService_->setValue(settingsGroup_ + "/stopBits", stopBitsCombo_->currentText());
    settingsService_->setValue(settingsGroup_ + "/portName", portCombo_->currentData().toString());
    settingsService_->setValue(settingsGroup_ + "/flowControl", flowControlCombo_->currentData().toString());
    settingsService_->setValue(settingsGroup_ + "/autoReconnect", autoReconnectCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/reconnectDelay", reconnectDelaySpin_->value());
}

bool SerialConnectionWidget::autoReconnectEnabled() const {
    return autoReconnectCheck_ ? autoReconnectCheck_->isChecked() : false;
}

int SerialConnectionWidget::reconnectDelayMs() const {
    return reconnectDelaySpin_ ? reconnectDelaySpin_->value() : 3000;
}

void SerialConnectionWidget::retranslateUi() {
    if (section_) {
        section_->setTitle(tr("Connection Settings"));
    }
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
    repopulateParityOptions(parityCombo_);
    if (stopBitsLabel_) {
        stopBitsLabel_->setText(tr("Stop:"));
    }
    if (flowControlLabel_) {
        flowControlLabel_->setText(tr("Flow:"));
    }
    repopulateFlowControlOptions(flowControlCombo_);
    if (refreshBtn_) {
        refreshBtn_->setToolTip(tr("Refresh Ports"));
    }
    if (autoReconnectCheck_) {
        autoReconnectCheck_->setText(tr("Auto Reconnect"));
    }
    applyDisplayState();
}

void SerialConnectionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
