// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SerialConnectionWidget.h"
#include "Config.h"
#include "CollapsibleSection.h"
#include "common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include "../../core/common/TrContext.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QSerialPortInfo>

#include <QSignalBlocker>

namespace ui::widgets {

using namespace core::common::settings_keys;

namespace {

constexpr auto kParityNone = "none";
constexpr auto kParityEven = "even";
constexpr auto kParityOdd = "odd";
constexpr auto kParitySpace = "space";
constexpr auto kParityMark = "mark";
constexpr auto kFlowNone = "none";
constexpr auto kFlowRtsCts = "rts_cts";
constexpr auto kFlowXonXoff = "xon_xoff";
constexpr char kSerialContext[] = "ui::widgets::SerialConnectionWidget";
constexpr auto kNoneText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "None");
constexpr auto kEvenText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Even");
constexpr auto kOddText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Odd");
constexpr auto kSpaceText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Space");
constexpr auto kMarkText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "Mark");
constexpr auto kRtsCtsText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "RTS/CTS");
constexpr auto kXonXoffText = QT_TRANSLATE_NOOP("ui::widgets::SerialConnectionWidget", "XON/XOFF");

void repopulateParityOptions(QComboBox* combo)
{
    if (!combo) {
        return;
    }

    const QString currentValue = combo->currentData().toString();
    QSignalBlocker blocker(combo);
    combo->clear();
    combo->addItem(TrContext<kSerialContext>::tr(kNoneText), QString::fromLatin1(kParityNone));
    combo->addItem(TrContext<kSerialContext>::tr(kEvenText), QString::fromLatin1(kParityEven));
    combo->addItem(TrContext<kSerialContext>::tr(kOddText), QString::fromLatin1(kParityOdd));
    combo->addItem(TrContext<kSerialContext>::tr(kSpaceText), QString::fromLatin1(kParitySpace));
    combo->addItem(TrContext<kSerialContext>::tr(kMarkText), QString::fromLatin1(kParityMark));

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
    combo->addItem(TrContext<kSerialContext>::tr(kNoneText), QString::fromLatin1(kFlowNone));
    combo->addItem(TrContext<kSerialContext>::tr(kRtsCtsText), QString::fromLatin1(kFlowRtsCts));
    combo->addItem(TrContext<kSerialContext>::tr(kXonXoffText), QString::fromLatin1(kFlowXonXoff));

    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

bool usesDisconnectAction(SerialConnectionWidget::DisplayState state)
{
    switch (state) {
    case SerialConnectionWidget::DisplayState::Connecting:
    case SerialConnectionWidget::DisplayState::Connected:
        return true;
    case SerialConnectionWidget::DisplayState::Disconnected:
    case SerialConnectionWidget::DisplayState::Disconnecting:
        return false;
    default:
        return false;
    }
}

} // namespace

SerialConnectionWidget::SerialConnectionWidget(infra::config::ISettingsService* settingsService, QWidget *parent)
    : BaseConnectionWidget(settingsService, parent) {
    suppressSettingsSave_ = true;
    setupUi();
    refreshPorts();
    suppressSettingsSave_ = false;
}

SerialConnectionWidget::~SerialConnectionWidget() = default;

io::SerialConfig SerialConnectionWidget::getConfig() const {
    io::SerialConfig config;
    config.portName = portCombo_->currentData().toString();
    if (config.portName.isEmpty()) {
        config.portName = portCombo_->currentText();
    }
    
    bool ok;
    config.baudRate = baudCombo_->currentText().toInt(&ok);
    if (!ok) config.baudRate = config::Serial::kDefaultBaudRate;
    config.dataBits = dataBitsCombo_->currentText().toInt();
    
    // Stop Bits
    QString stopStr = stopBitsCombo_->currentText();
    if (stopStr == QStringLiteral("1")) config.stopBits = QSerialPort::OneStop;
    else if (stopStr == QStringLiteral("1.5")) config.stopBits = QSerialPort::OneAndHalfStop;
    else if (stopStr == QStringLiteral("2")) config.stopBits = QSerialPort::TwoStop;
    
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

StateDisplayInfo SerialConnectionWidget::getStateDisplayInfo(DisplayState state) const {
    switch (state) {
    case DisplayState::Disconnected:
        return {tr("Connect"), tr("Disconnected"), QStringLiteral("color: red; font-weight: bold;")};
    case DisplayState::Connecting:
        return {tr("Disconnect"), tr("Connecting"), QStringLiteral("color: orange; font-weight: bold;")};
    case DisplayState::Connected:
        return {tr("Disconnect"), tr("Connected"), QStringLiteral("color: green; font-weight: bold;")};
    case DisplayState::Disconnecting:
        return {tr("Disconnecting"), tr("Disconnecting"), QStringLiteral("color: orange; font-weight: bold;")};
    default:
        return {};
    }
}

void SerialConnectionWidget::applyInputWidgetsState(bool enabled) {
    // Serial-specific input widgets only; connectBtn_ is handled by the base skeleton.
    portCombo_->setEnabled(enabled);
    baudCombo_->setEnabled(enabled);
    dataBitsCombo_->setEnabled(enabled);
    parityCombo_->setEnabled(enabled);
    stopBitsCombo_->setEnabled(enabled);
    flowControlCombo_->setEnabled(enabled);
    refreshBtn_->setEnabled(enabled);
}

void SerialConnectionWidget::stripOfflinePortEntries() {
    if (!portCombo_) return;
    for (int i = portCombo_->count() - 1; i >= 0; --i) {
        if (portCombo_->itemData(i, Qt::UserRole + 1).toBool()) {
            portCombo_->removeItem(i);
        }
    }
}

void SerialConnectionWidget::ensureOfflinePortEntry(const QString& portName) {
    if (!portCombo_ || portName.isEmpty()) return;
    const int existing = portCombo_->findData(portName);
    if (existing >= 0) {
        portCombo_->setCurrentIndex(existing);
        return;
    }
    portCombo_->addItem(tr("%1 (Offline)").arg(portName), portName);
    const int addedIndex = portCombo_->count() - 1;
    portCombo_->setItemData(addedIndex, true, Qt::UserRole + 1);
    portCombo_->setCurrentIndex(addedIndex);
}

void SerialConnectionWidget::refreshPorts() {
    QSignalBlocker blocker(portCombo_);
    QString currentPortName = portCombo_->currentData().toString();
    if (currentPortName.isEmpty()) {
        currentPortName = portCombo_->currentText();
    }
    const QString savedPortName = (!settingsGroup_.isEmpty() && settingsService_)
        ? settingsService_->value(settingsGroup_ + QStringLiteral("/portName")).toString()
        : QString();

    portCombo_->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QString itemText = info.portName();
        if (!info.description().isEmpty()) {
            itemText = info.portName() + QStringLiteral(" (") + info.description() + QStringLiteral(")");
        }
        portCombo_->addItem(itemText, info.portName());
    }

    const QString preferredPort = !currentPortName.isEmpty() ? currentPortName : savedPortName;
    const int index = preferredPort.isEmpty() ? -1 : portCombo_->findData(preferredPort);
    if (index >= 0) {
        portCombo_->setCurrentIndex(index);
    } else if (!savedPortName.isEmpty()) {
        // Keep unplugged configured port selected without overwriting saved settings.
        ensureOfflinePortEntry(savedPortName);
        return;
    } else if (portCombo_->count() > 0) {
        portCombo_->setCurrentIndex(0);
    }
    saveSettings();
}

QSize SerialConnectionWidget::minimumSizeHint() const {
    if (section_) {
        return QSize(360, section_->minimumSizeHint().height());
    }
    return BaseConnectionWidget::minimumSizeHint();
}

void SerialConnectionWidget::setTwoRowLayout(bool twoRow) {
    if (isTwoRow_ == twoRow) return;
    applyLayoutMode(twoRow);
    updateGeometry();
    if (section_) section_->updateGeometry();
}

void SerialConnectionWidget::applyLayoutMode(bool twoRow) {
    isTwoRow_ = twoRow;
    if (!gridLayout_) return;

    if (twoRow) {
        // Row 0: Port, Refresh, Baud, Data
        gridLayout_->addWidget(portLabel_, 0, 0);
        gridLayout_->addWidget(portCombo_, 0, 1);
        gridLayout_->addWidget(refreshBtn_, 0, 2);
        gridLayout_->addWidget(baudLabel_, 0, 3);
        gridLayout_->addWidget(baudCombo_, 0, 4);
        gridLayout_->addWidget(dataBitsLabel_, 0, 5);
        gridLayout_->addWidget(dataBitsCombo_, 0, 6);

        // Row 1: Stop, Parity, Flow, Connect, Status
        gridLayout_->addWidget(stopBitsLabel_, 1, 0);
        gridLayout_->addWidget(stopBitsCombo_, 1, 1);
        gridLayout_->addWidget(parityLabel_, 1, 3);
        gridLayout_->addWidget(parityCombo_, 1, 4);
        gridLayout_->addWidget(flowControlLabel_, 1, 5);
        gridLayout_->addWidget(flowControlCombo_, 1, 6);
        gridLayout_->addWidget(connectBtn_, 1, 7);
        gridLayout_->addWidget(statusLabel_, 1, 8);

        gridLayout_->setColumnStretch(9, 1);
        gridLayout_->setColumnStretch(15, 0);
    } else {
        // Single row: all in Row 0
        gridLayout_->addWidget(portLabel_, 0, 0);
        gridLayout_->addWidget(portCombo_, 0, 1);
        gridLayout_->addWidget(refreshBtn_, 0, 2);
        gridLayout_->addWidget(baudLabel_, 0, 3);
        gridLayout_->addWidget(baudCombo_, 0, 4);
        gridLayout_->addWidget(dataBitsLabel_, 0, 5);
        gridLayout_->addWidget(dataBitsCombo_, 0, 6);
        gridLayout_->addWidget(parityLabel_, 0, 7);
        gridLayout_->addWidget(parityCombo_, 0, 8);
        gridLayout_->addWidget(stopBitsLabel_, 0, 9);
        gridLayout_->addWidget(stopBitsCombo_, 0, 10);
        gridLayout_->addWidget(flowControlLabel_, 0, 11);
        gridLayout_->addWidget(flowControlCombo_, 0, 12);
        gridLayout_->addWidget(connectBtn_, 0, 13);
        gridLayout_->addWidget(statusLabel_, 0, 14);

        gridLayout_->setColumnStretch(9, 0);
        gridLayout_->setColumnStretch(15, 1);
    }
}

void SerialConnectionWidget::updateResponsiveLayout(int availableWidth) {
    if (availableWidth <= 0) return;
    static constexpr int kCollapseThreshold = 630;
    static constexpr int kExpandThreshold = 650;

    if (isTwoRow_ && availableWidth >= kExpandThreshold) {
        applyLayoutMode(false);
        updateGeometry();
        if (section_) section_->updateGeometry();
    } else if (!isTwoRow_ && availableWidth < kCollapseThreshold) {
        applyLayoutMode(true);
        updateGeometry();
        if (section_) section_->updateGeometry();
    }
}

void SerialConnectionWidget::resizeEvent(QResizeEvent* event) {
    BaseConnectionWidget::resizeEvent(event);
    if (event) {
        updateResponsiveLayout(event->size().width());
    }
}

void SerialConnectionWidget::setupUi() {
    gridLayout_ = new QGridLayout();
    gridLayout_->setSizeConstraint(QLayout::SetNoConstraint);
    gridLayout_->setContentsMargins(4, 0, 4, 0);
    gridLayout_->setHorizontalSpacing(4);
    gridLayout_->setVerticalSpacing(2);
    setupBaseUiWithLayout(gridLayout_);

    // Port
    portLabel_ = new QLabel(this);
    portCombo_ = new QComboBox(this);
    portCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    portCombo_->setMinimumContentsLength(5);
    portCombo_->setMinimumWidth(50);

    refreshBtn_ = new QPushButton(QStringLiteral("R"), this);
    refreshBtn_->setFixedWidth(28);
    refreshBtn_->setToolTip(tr("Refresh Ports"));
    connect(refreshBtn_, &QPushButton::clicked, this, &SerialConnectionWidget::refreshPorts);

    // Baud
    baudLabel_ = new QLabel(this);
    baudCombo_ = new QComboBox(this);
    baudCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    baudCombo_->setMinimumContentsLength(6);
    baudCombo_->addItems({QString::fromLatin1(config::Serial::kDefaultBaudRateText),
                          QStringLiteral("19200"),
                          QStringLiteral("38400"),
                          QStringLiteral("57600"),
                          QStringLiteral("115200")});
    baudCombo_->setCurrentText(QString::fromLatin1(config::Serial::kDefaultBaudRateText));

    // Data Bits
    dataBitsLabel_ = new QLabel(this);
    dataBitsCombo_ = new QComboBox(this);
    dataBitsCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    dataBitsCombo_->setMinimumContentsLength(2);
    dataBitsCombo_->addItems({QString::fromLatin1(config::Serial::kDefaultDataBitsText), QStringLiteral("7")});
    dataBitsCombo_->setCurrentText(QString::fromLatin1(config::Serial::kDefaultDataBitsText));
    dataBitsCombo_->setMinimumWidth(36);

    // Parity
    parityLabel_ = new QLabel(this);
    parityCombo_ = new QComboBox(this);
    parityCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    parityCombo_->setMinimumContentsLength(4);
    parityCombo_->setMinimumWidth(48);

    // Stop Bits
    stopBitsLabel_ = new QLabel(this);
    stopBitsCombo_ = new QComboBox(this);
    stopBitsCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    stopBitsCombo_->setMinimumContentsLength(3);
    stopBitsCombo_->addItems({QString::fromLatin1(config::Serial::kDefaultStopBitsText),
                              QStringLiteral("1.5"),
                              QStringLiteral("2")});
    stopBitsCombo_->setMinimumWidth(36);

    // Flow Control
    flowControlLabel_ = new QLabel(this);
    flowControlCombo_ = new QComboBox(this);
    flowControlCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    flowControlCombo_->setMinimumContentsLength(4);
    flowControlCombo_->setMinimumWidth(56);

    // Create common widgets (connectBtn_, statusLabel_)
    createCommonWidgets(section_->contentWidget());

    // Initially start in 2-row layout to ensure low initial minimumSizeHint
    isTwoRow_ = true;
    applyLayoutMode(true);

    auto* mainLayout = qobject_cast<QHBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
        mainLayout->addWidget(section_);
    }

    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (usesDisconnectAction(displayState_)) {
            emit disconnectClicked();
        } else {
            const auto config = getConfig();
            saveSettings();
            emit connectClicked(config);
        }
    });

    connect(portCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &SerialConnectionWidget::saveSettings);
    connect(baudCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(dataBitsCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(parityCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(stopBitsCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);
    connect(flowControlCombo_, &QComboBox::currentTextChanged, this, &SerialConnectionWidget::saveSettings);

    setupCommonConnections();
    repopulateParityOptions(parityCombo_);
    repopulateFlowControlOptions(flowControlCombo_);
    if (section_) {
        const QString collapseKey = settingsGroup_.isEmpty()
            ? QString::fromLatin1(core::common::settings_keys::kSerialDebuggerConnectionCollapsed)
            : (settingsGroup_ + QStringLiteral("/ui/connectionSettingsCollapsed"));
        section_->setSettingsKey(collapseKey);
    }
    loadSettings();

    retranslateUi();
}

void SerialConnectionWidget::loadSettings() {
    loadCommonSettings();
    if (!settingsService_ || settingsGroup_.isEmpty()) return;
    QSignalBlocker b0(portCombo_);
    QSignalBlocker b1(baudCombo_);
    QSignalBlocker b2(dataBitsCombo_);
    QSignalBlocker b3(parityCombo_);
    QSignalBlocker b4(stopBitsCombo_);
    QSignalBlocker b5(flowControlCombo_);

    const QString group = settingsGroup_;
    const QString baudKey = group + QStringLiteral("/baudRate");
    const QString dataBitsKey = group + QStringLiteral("/dataBits");
    const QString parityKey = group + QStringLiteral("/parity");
    const QString stopBitsKey = group + QStringLiteral("/stopBits");
    const QString portKey = group + QStringLiteral("/portName");
    const QString flowKey = group + QStringLiteral("/flowControl");

    // The legacy "serial/baudRate" fallback read was removed:
    // the in-flight migration now carries that legacy value into
    // modbus/rtu/serial/baudRate at load time, and pages without a saved
    // baudRate simply keep their combo-box default.
    const int baudRate = settingsService_->contains(baudKey)
        ? settingsService_->value(baudKey).toInt() : 0;
    const QString dataBits = settingsService_->value(dataBitsKey).toString();
    const QString parity = settingsService_->value(parityKey).toString();
    const QString stopBits = settingsService_->value(stopBitsKey).toString();
    const QString portName = settingsService_->value(portKey).toString();
    const QString flowControl = settingsService_->value(flowKey).toString();

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

    int flowIndex = flowControl.isEmpty() ? 0 : flowControlCombo_->findData(flowControl);
    if (flowIndex < 0 && !flowControl.isEmpty()) {
        flowIndex = flowControlCombo_->findText(flowControl);
    }
    if (flowIndex >= 0) {
        flowControlCombo_->setCurrentIndex(flowIndex);
    }

    stripOfflinePortEntries();
    if (!portName.isEmpty()) {
        const int portIndex = portCombo_->findData(portName);
        if (portIndex >= 0) {
            portCombo_->setCurrentIndex(portIndex);
        } else {
            ensureOfflinePortEntry(portName);
        }
    }
}

void SerialConnectionWidget::saveSettings() {
    saveCommonSettings();
    if (suppressSettingsSave_ || !settingsService_ || settingsGroup_.isEmpty()) return;

    const QString group = settingsGroup_;
    const QString baudKey = group + QStringLiteral("/baudRate");
    const QString dataBitsKey = group + QStringLiteral("/dataBits");
    const QString parityKey = group + QStringLiteral("/parity");
    const QString stopBitsKey = group + QStringLiteral("/stopBits");
    const QString portKey = group + QStringLiteral("/portName");
    const QString flowKey = group + QStringLiteral("/flowControl");

    settingsService_->setValue(baudKey, baudCombo_->currentText());
    settingsService_->setValue(dataBitsKey, dataBitsCombo_->currentText());
    settingsService_->setValue(parityKey, parityCombo_->currentData().toString());
    settingsService_->setValue(stopBitsKey, stopBitsCombo_->currentText());
    settingsService_->setValue(portKey, portCombo_->currentData().toString());
    settingsService_->setValue(flowKey, flowControlCombo_->currentData().toString());
}

void SerialConnectionWidget::retranslateUi() {
    BaseConnectionWidget::retranslateUi();
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
    if (portCombo_) {
        QSignalBlocker blocker(portCombo_);
        for (int i = 0; i < portCombo_->count(); ++i) {
            if (portCombo_->itemData(i, Qt::UserRole + 1).toBool()) {
                portCombo_->setItemText(
                    i, tr("%1 (Offline)").arg(portCombo_->itemData(i).toString()));
            }
        }
    }
    updateResponsiveLayout(width());
}

} // namespace ui::widgets
