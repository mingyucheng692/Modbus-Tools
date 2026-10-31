// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusWorkbenchWidget.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFontDatabase>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedLayout>
#include <QVBoxLayout>

#include "common/ModbusDataHelper.h"
#include "infra/config/ISettingsService.h"
#include "core/common/SettingsKeys.h"
#include "modbus/base/ModbusCrc.h"
#include "modbus/base/ModbusLrc.h"

namespace ui::widgets::modbus {

ModbusWorkbenchWidget::ModbusWorkbenchWidget(infra::config::ISettingsService* settingsService,
                                             QWidget* parent)
    : QWidget(parent)
    , settingsService_(settingsService) {
    setMinimumWidth(360);
    guardState_.executeEnabled = true;
    guardState_.pollingEnabled = true;
    guardState_.rawSendEnabled = true;

    setupUi();
    loadSettings();
}

ModbusWorkbenchWidget::~ModbusWorkbenchWidget() noexcept {
    saveSettings();
}

void ModbusWorkbenchWidget::setExecutionGuards(const ::ui::application::modbus::ExecutionGuardState& guards) {
    guardState_ = guards;
    if (guards.pollingActive != isPolling_) {
        isPolling_ = guards.pollingActive;
        pollCheckBox_->blockSignals(true);
        pollCheckBox_->setChecked(isPolling_);
        pollCheckBox_->blockSignals(false);
        executeBtn_->setText(isPolling_ ? tr("Stop Polling (Esc)") : tr("Execute (F5)"));
        if (!isPolling_) {
            writePollWarningBadge_->hide();
            userConfirmedWritePoll_ = false;
        }
    }
    rawExecuteBtn_->setEnabled(guards.rawSendEnabled);
    validateInputs();
}

void ModbusWorkbenchWidget::setAddressBase(::modbus::address::AddressBase base) {
    addressBase_ = base;
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::setProtocolType(::modbus::parser::ProtocolType protocol) {
    protocolType_ = protocol;
    validateInputs();
}

void ModbusWorkbenchWidget::setConfirmWritePollHandler(std::function<bool(uint8_t fc)> handler) noexcept {
    confirmWritePollHandler_ = std::move(handler);
}

::ui::application::modbus::ModbusExecuteSpec ModbusWorkbenchWidget::currentSpec() const {
    ::ui::application::modbus::ModbusExecuteSpec spec;
    spec.slaveId = static_cast<uint8_t>(slaveIdSpin_->value());

    if (isRawMode_) {
        spec.isRawMode = true;
        spec.rawPduOrAdu = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
        return spec;
    }

    spec.isRawMode = false;
    spec.functionCode = static_cast<uint8_t>(fcCombo_->currentData().toUInt());
    spec.startAddress = pduAddress_;
    spec.quantity = static_cast<uint16_t>(quantitySpin_->value());

    if (spec.functionCode == 0x05) {
        spec.writeData = fc05IsOn_ ? QByteArray::fromHex("FF00") : QByteArray::fromHex("0000");
    } else if (spec.functionCode == 0x06) {
        bool ok = false;
        const int val = fc06ValueEdit_->text().toInt(&ok, radix_);
        spec.writeData.resize(2);
        spec.writeData[0] = static_cast<char>((val >> 8) & 0xFF);
        spec.writeData[1] = static_cast<char>(val & 0xFF);
    } else if (spec.functionCode == 0x0F) {
        auto res = ::ui::common::data_helper::packCoilBits(fc0fBitsEdit_->text(), spec.quantity);
        if (res.isOk()) {
            spec.writeData = res.value();
        }
    } else if (spec.functionCode == 0x10) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc10WordsEdit_->text(), radix_, spec.quantity);
        if (res.isOk()) {
            spec.writeData = res.value();
        }
    }
    return spec;
}

bool ModbusWorkbenchWidget::isRawMode() const noexcept { return isRawMode_; }
bool ModbusWorkbenchWidget::isPolling() const noexcept { return isPolling_; }
int ModbusWorkbenchWidget::pollingIntervalMs() const noexcept { return pollIntervalSpin_->value(); }
uint8_t ModbusWorkbenchWidget::slaveId() const noexcept { return static_cast<uint8_t>(slaveIdSpin_->value()); }
uint8_t ModbusWorkbenchWidget::functionCode() const noexcept { return static_cast<uint8_t>(fcCombo_->currentData().toUInt()); }
uint16_t ModbusWorkbenchWidget::startAddress() const noexcept { return pduAddress_; }
uint16_t ModbusWorkbenchWidget::quantity() const noexcept { return static_cast<uint16_t>(quantitySpin_->value()); }
int ModbusWorkbenchWidget::radix() const noexcept { return radix_; }
::modbus::address::AddressBase ModbusWorkbenchWidget::addressBase() const noexcept { return addressBase_; }

void ModbusWorkbenchWidget::setMode(int mode) {
    if (mode == 0) {
        standardModeBtn_->setChecked(true);
        onModeChanged(0);
    } else {
        rawModeBtn_->setChecked(true);
        onModeChanged(1);
    }
}

void ModbusWorkbenchWidget::setSlaveId(uint8_t slaveId) {
    slaveIdSpin_->setValue(slaveId);
}

void ModbusWorkbenchWidget::setFunctionCode(uint8_t fc) {
    const int idx = fcCombo_->findData(fc);
    if (idx >= 0) {
        fcCombo_->setCurrentIndex(idx);
    }
}

void ModbusWorkbenchWidget::setStartAddress(uint16_t addr) {
    pduAddress_ = addr;
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::setQuantity(uint16_t qty) {
    quantitySpin_->setValue(qty);
}

void ModbusWorkbenchWidget::setRadix(int radix) {
    if (radix == 16) {
        radixHexBtn_->setChecked(true);
        onRadixChanged(16);
    } else {
        radixDecBtn_->setChecked(true);
        onRadixChanged(10);
    }
}

void ModbusWorkbenchWidget::setPollingIntervalMs(int ms) {
    pollIntervalSpin_->setValue(ms);
}

void ModbusWorkbenchWidget::setFc05Value(bool on) {
    fc05IsOn_ = on;
    if (on) {
        fc05OnBtn_->setChecked(true);
    } else {
        fc05OffBtn_->setChecked(true);
    }
}

void ModbusWorkbenchWidget::setWritePayload(const QString& text) {
    const uint8_t fc = functionCode();
    if (fc == 0x06) {
        fc06ValueEdit_->setText(text);
    } else if (fc == 0x0F) {
        fc0fBitsEdit_->setText(text);
    } else if (fc == 0x10) {
        fc10WordsEdit_->setText(text);
    }
    validateInputs();
}

void ModbusWorkbenchWidget::setRawText(const QString& text) {
    rawTextEdit_->setPlainText(text);
    validateInputs();
}

void ModbusWorkbenchWidget::setPollingChecked(bool checked) {
    if (checked && slaveId() == 0) {
        stopPollingInternal(false);
        return;
    }
    pollCheckBox_->setChecked(checked);
}

void ModbusWorkbenchWidget::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* container = new QWidget(scrollArea);
    auto* rootLayout = new QVBoxLayout(container);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    auto* topBarLayout = new QHBoxLayout();
    modeGroup_ = new QButtonGroup(this);
    standardModeBtn_ = new QPushButton(tr("Standard"), container);
    standardModeBtn_->setObjectName(QStringLiteral("standardModeBtn"));
    standardModeBtn_->setCheckable(true);
    standardModeBtn_->setChecked(true);
    rawModeBtn_ = new QPushButton(tr("Raw Frame"), container);
    rawModeBtn_->setObjectName(QStringLiteral("rawModeBtn"));
    rawModeBtn_->setCheckable(true);
    modeGroup_->addButton(standardModeBtn_, 0);
    modeGroup_->addButton(rawModeBtn_, 1);
    connect(modeGroup_, &QButtonGroup::idClicked, this, &ModbusWorkbenchWidget::onModeChanged);
    topBarLayout->addWidget(standardModeBtn_);
    topBarLayout->addWidget(rawModeBtn_);
    rootLayout->addLayout(topBarLayout);

    auto* stackWidget = new QWidget(container);
    mainStackedLayout_ = new QStackedLayout(stackWidget);
    mainStackedLayout_->addWidget(createStandardWidget());
    mainStackedLayout_->addWidget(createRawWidget());
    rootLayout->addWidget(stackWidget);

    scrollArea->setWidget(container);
    outerLayout->addWidget(scrollArea);
}

QWidget* ModbusWorkbenchWidget::createStandardWidget() {
    auto* w = new QWidget(this);
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(8);

    setupAddressingSection(w, l);
    setupWriteSection(w, l);
    setupExecutionSection(w, l);
    l->addStretch(1);
    return w;
}

void ModbusWorkbenchWidget::setupAddressingSection(QWidget* parent, QVBoxLayout* layout) {
    auto* group = new QGroupBox(tr("Addressing & Parameters"), parent);
    auto* gl = new QVBoxLayout(group);
    gl->setSpacing(6);

    setupRadixSelector(group, gl);
    setupParametersInputs(group, gl);

    addressWarningLabel_ = new QLabel(group);
    addressWarningLabel_->setObjectName(QStringLiteral("workbenchAddressWarning"));
    addressWarningLabel_->setStyleSheet(QStringLiteral("color: #d32f2f; font-weight: bold;"));
    addressWarningLabel_->setWordWrap(true);
    addressWarningLabel_->hide();
    gl->addWidget(addressWarningLabel_);

    layout->addWidget(group);
}

void ModbusWorkbenchWidget::setupRadixSelector(QGroupBox* group, QVBoxLayout* gl) {
    auto* radixLayout = new QHBoxLayout();
    radixLayout->addWidget(new QLabel(tr("Radix:"), group));
    radixGroup_ = new QButtonGroup(this);
    radixDecBtn_ = new QPushButton(tr("DEC"), group);
    radixDecBtn_->setObjectName(QStringLiteral("radixDecBtn"));
    radixDecBtn_->setCheckable(true);
    radixDecBtn_->setChecked(true);
    radixHexBtn_ = new QPushButton(tr("HEX"), group);
    radixHexBtn_->setObjectName(QStringLiteral("radixHexBtn"));
    radixHexBtn_->setCheckable(true);
    radixGroup_->addButton(radixDecBtn_, 10);
    radixGroup_->addButton(radixHexBtn_, 16);
    connect(radixGroup_, &QButtonGroup::idClicked, this, &ModbusWorkbenchWidget::onRadixChanged);
    radixLayout->addWidget(radixDecBtn_);
    radixLayout->addWidget(radixHexBtn_);
    radixLayout->addStretch(1);
    gl->addLayout(radixLayout);
}

void ModbusWorkbenchWidget::setupParametersInputs(QGroupBox* group, QVBoxLayout* gl) {
    gl->addWidget(new QLabel(tr("Function Code:"), group));
    fcCombo_ = new QComboBox(group);
    fcCombo_->setObjectName(QStringLiteral("workbenchFcCombo"));
    fcCombo_->addItem(tr("0x01 Read Coils"), 0x01);
    fcCombo_->addItem(tr("0x02 Read Discrete Inputs"), 0x02);
    fcCombo_->addItem(tr("0x03 Read Holding Registers"), 0x03);
    fcCombo_->addItem(tr("0x04 Read Input Registers"), 0x04);
    fcCombo_->addItem(tr("0x05 Write Single Coil"), 0x05);
    fcCombo_->addItem(tr("0x06 Write Single Register"), 0x06);
    fcCombo_->addItem(tr("0x0F Write Multiple Coils"), 0x0F);
    fcCombo_->addItem(tr("0x10 Write Multiple Registers"), 0x10);
    fcCombo_->setCurrentIndex(2);
    connect(fcCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusWorkbenchWidget::onFunctionCodeChanged);
    gl->addWidget(fcCombo_);

    auto* slaveLayout = new QHBoxLayout();
    slaveLayout->addWidget(new QLabel(tr("Slave ID (0-247):"), group));
    slaveIdSpin_ = new QSpinBox(group);
    slaveIdSpin_->setObjectName(QStringLiteral("workbenchSlaveIdSpin"));
    slaveIdSpin_->setRange(0, 247);
    slaveIdSpin_->setValue(1);
    connect(slaveIdSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusWorkbenchWidget::onSlaveIdChanged);
    slaveLayout->addWidget(slaveIdSpin_);
    gl->addLayout(slaveLayout);

    setupAddressAndQuantityInputs(group, gl);
}

void ModbusWorkbenchWidget::setupAddressAndQuantityInputs(QGroupBox* group, QVBoxLayout* gl) {
    auto* addrLayout = new QHBoxLayout();
    addrLayout->addWidget(new QLabel(tr("Start Address:"), group));
    addressEdit_ = new QLineEdit(group);
    addressEdit_->setObjectName(QStringLiteral("workbenchAddressEdit"));
    addressEdit_->setText(QStringLiteral("0"));
    connect(addressEdit_, &QLineEdit::textEdited, this, &ModbusWorkbenchWidget::onAddressEdited);
    addrLayout->addWidget(addressEdit_);
    gl->addLayout(addrLayout);

    auto* qtyLayout = new QHBoxLayout();
    qtyLayout->addWidget(new QLabel(tr("Quantity:"), group));
    quantitySpin_ = new QSpinBox(group);
    quantitySpin_->setObjectName(QStringLiteral("workbenchQuantitySpin"));
    quantitySpin_->setRange(1, 125);
    quantitySpin_->setValue(10);
    connect(quantitySpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusWorkbenchWidget::onQuantityChanged);
    qtyLayout->addWidget(quantitySpin_);
    gl->addLayout(qtyLayout);
}

void ModbusWorkbenchWidget::setupWriteSection(QWidget* parent, QVBoxLayout* layout) {
    writeGroup_ = new QGroupBox(tr("Write Data"), parent);
    auto* wl = new QVBoxLayout(writeGroup_);
    auto* writeStackWidget = new QWidget(writeGroup_);
    writeStackedLayout_ = new QStackedLayout(writeStackWidget);

    setupWriteStackPages(writeStackWidget);

    wl->addWidget(writeStackWidget);
    writeGroup_->hide();
    layout->addWidget(writeGroup_);
}

void ModbusWorkbenchWidget::setupWriteStackPages(QWidget* stackWidget) {
    auto* p0 = new QWidget(stackWidget);
    auto* l0 = new QHBoxLayout(p0);
    fc05Group_ = new QButtonGroup(this);
    fc05OnBtn_ = new QPushButton(tr("ON (0xFF00)"), p0);
    fc05OnBtn_->setObjectName(QStringLiteral("workbenchFc05OnBtn"));
    fc05OnBtn_->setCheckable(true);
    fc05OnBtn_->setChecked(true);
    fc05OffBtn_ = new QPushButton(tr("OFF (0x0000)"), p0);
    fc05OffBtn_->setObjectName(QStringLiteral("workbenchFc05OffBtn"));
    fc05OffBtn_->setCheckable(true);
    fc05Group_->addButton(fc05OnBtn_, 1);
    fc05Group_->addButton(fc05OffBtn_, 0);
    connect(fc05Group_, &QButtonGroup::idClicked, this, [this](int id) {
        onFc05Toggled(id == 1);
    });
    l0->addWidget(fc05OnBtn_);
    l0->addWidget(fc05OffBtn_);
    writeStackedLayout_->addWidget(p0);

    auto* p1 = new QWidget(stackWidget);
    auto* l1 = new QVBoxLayout(p1);
    l1->addWidget(new QLabel(tr("Register Value (0-65535):"), p1));
    fc06ValueEdit_ = new QLineEdit(p1);
    fc06ValueEdit_->setObjectName(QStringLiteral("workbenchFc06ValueEdit"));
    fc06ValueEdit_->setText(QStringLiteral("0"));
    connect(fc06ValueEdit_, &QLineEdit::textEdited, this, &ModbusWorkbenchWidget::validateInputs);
    l1->addWidget(fc06ValueEdit_);
    writeStackedLayout_->addWidget(p1);

    setupBitAndWordPages(stackWidget);
}

void ModbusWorkbenchWidget::setupBitAndWordPages(QWidget* stackWidget) {
    auto* p2 = new QWidget(stackWidget);
    auto* l2 = new QVBoxLayout(p2);
    fc0fHintLabel_ = new QLabel(tr("Bit Sequence (e.g. 10110):"), p2);
    l2->addWidget(fc0fHintLabel_);
    fc0fBitsEdit_ = new QLineEdit(p2);
    fc0fBitsEdit_->setObjectName(QStringLiteral("workbenchFc0fBitsEdit"));
    connect(fc0fBitsEdit_, &QLineEdit::textEdited, this, &ModbusWorkbenchWidget::validateInputs);
    l2->addWidget(fc0fBitsEdit_);
    writeStackedLayout_->addWidget(p2);

    auto* p3 = new QWidget(stackWidget);
    auto* l3 = new QVBoxLayout(p3);
    fc10HintLabel_ = new QLabel(tr("Register Values (space separated):"), p3);
    l3->addWidget(fc10HintLabel_);
    fc10WordsEdit_ = new QLineEdit(p3);
    fc10WordsEdit_->setObjectName(QStringLiteral("workbenchFc10WordsEdit"));
    connect(fc10WordsEdit_, &QLineEdit::textEdited, this, &ModbusWorkbenchWidget::validateInputs);
    l3->addWidget(fc10WordsEdit_);
    writeStackedLayout_->addWidget(p3);
}

void ModbusWorkbenchWidget::setupExecutionSection(QWidget* parent, QVBoxLayout* layout) {
    auto* group = new QGroupBox(tr("Execution & Polling"), parent);
    auto* gl = new QVBoxLayout(group);
    gl->setSpacing(6);

    executeBtn_ = new QPushButton(tr("Execute (F5)"), group);
    executeBtn_->setObjectName(QStringLiteral("workbenchExecuteBtn"));
    executeBtn_->setShortcut(QKeySequence(Qt::Key_F5));
    connect(executeBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onExecuteClicked);
    gl->addWidget(executeBtn_);

    auto* pollLayout = new QHBoxLayout();
    pollCheckBox_ = new QCheckBox(tr("Periodic Polling"), group);
    pollCheckBox_->setObjectName(QStringLiteral("workbenchPollCheck"));
    connect(pollCheckBox_, &QCheckBox::toggled, this, &ModbusWorkbenchWidget::onPollCheckBoxToggled);
    pollLayout->addWidget(pollCheckBox_);

    pollLayout->addWidget(new QLabel(tr("Interval:"), group));
    pollIntervalSpin_ = new QSpinBox(group);
    pollIntervalSpin_->setObjectName(QStringLiteral("workbenchPollIntervalSpin"));
    pollIntervalSpin_->setRange(50, 60000);
    pollIntervalSpin_->setSingleStep(100);
    pollIntervalSpin_->setValue(1000);
    pollIntervalSpin_->setSuffix(QStringLiteral(" ms"));
    connect(pollIntervalSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusWorkbenchWidget::onPollIntervalChanged);
    pollLayout->addWidget(pollIntervalSpin_);
    gl->addLayout(pollLayout);

    writePollWarningBadge_ = new QLabel(tr("[!] Write Polling Active"), group);
    writePollWarningBadge_->setObjectName(QStringLiteral("workbenchWritePollBadge"));
    writePollWarningBadge_->setStyleSheet(QStringLiteral("color: #e65100; font-weight: bold;"));
    writePollWarningBadge_->hide();
    gl->addWidget(writePollWarningBadge_);

    layout->addWidget(group);
}

QWidget* ModbusWorkbenchWidget::createRawWidget() {
    auto* w = new QWidget(this);
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(8);

    auto* editGroup = new QGroupBox(tr("Raw Frame Dispatch (HEX)"), w);
    auto* el = new QVBoxLayout(editGroup);

    rawTextEdit_ = new QPlainTextEdit(editGroup);
    rawTextEdit_->setObjectName(QStringLiteral("workbenchRawTextEdit"));
    rawTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    rawTextEdit_->setPlaceholderText(tr("Enter HEX bytes (e.g. 01 03 00 00 00 0A)"));
    connect(rawTextEdit_, &QPlainTextEdit::textChanged, this, &ModbusWorkbenchWidget::validateInputs);
    el->addWidget(rawTextEdit_);

    setupRawButtons(editGroup, el);

    l->addWidget(editGroup);
    l->addStretch(1);
    return w;
}

void ModbusWorkbenchWidget::setupRawButtons(QGroupBox* editGroup, QVBoxLayout* el) {
    auto* btnLayout = new QHBoxLayout();
    addMbapBtn_ = new QPushButton(tr("+ MBAP"), editGroup);
    addMbapBtn_->setObjectName(QStringLiteral("workbenchAddMbapBtn"));
    connect(addMbapBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onAddMbapClicked);

    addCrcBtn_ = new QPushButton(tr("+ CRC16"), editGroup);
    addCrcBtn_->setObjectName(QStringLiteral("workbenchAddCrcBtn"));
    connect(addCrcBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onAddCrcClicked);

    addLrcBtn_ = new QPushButton(tr("+ LRC"), editGroup);
    addLrcBtn_->setObjectName(QStringLiteral("workbenchAddLrcBtn"));
    connect(addLrcBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onAddLrcClicked);

    cleanHexBtn_ = new QPushButton(tr("Format"), editGroup);
    cleanHexBtn_->setObjectName(QStringLiteral("workbenchCleanHexBtn"));
    connect(cleanHexBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onCleanHexClicked);

    btnLayout->addWidget(addMbapBtn_);
    btnLayout->addWidget(addCrcBtn_);
    btnLayout->addWidget(addLrcBtn_);
    btnLayout->addWidget(cleanHexBtn_);
    el->addLayout(btnLayout);

    rawExecuteBtn_ = new QPushButton(tr("Send Raw Frame"), editGroup);
    rawExecuteBtn_->setObjectName(QStringLiteral("workbenchRawExecuteBtn"));
    connect(rawExecuteBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onExecuteClicked);
    el->addWidget(rawExecuteBtn_);
}

void ModbusWorkbenchWidget::onModeChanged(int id) {
    isRawMode_ = (id == 1);
    mainStackedLayout_->setCurrentIndex(isRawMode_ ? 1 : 0);
    validateInputs();
}

void ModbusWorkbenchWidget::onRadixChanged(int id) {
    const int oldRadix = radix_;
    radix_ = id;
    updateAddressDisplay();

    if (fc06ValueEdit_) {
        bool ok = false;
        const int val = fc06ValueEdit_->text().toInt(&ok, oldRadix);
        if (ok) {
            fc06ValueEdit_->setText(QString::number(val, radix_).toUpper());
        }
    }
    if (fc10WordsEdit_ && !fc10WordsEdit_->text().trimmed().isEmpty()) {
        fc10WordsEdit_->setText(::ui::common::data_helper::convertRadix(
            fc10WordsEdit_->text(), oldRadix, radix_));
    }
    validateInputs();
}

void ModbusWorkbenchWidget::onFunctionCodeChanged(int /*index*/) {
    const uint8_t fc = functionCode();

    if (isPolling_ && ::modbus::base::function_traits::isWriteOperation(fc)) {
        stopPollingInternal(true);
        emit logMessageRequested(tr("Polling automatically stopped due to write operation switch"), true);
    }

    const uint16_t maxQty = ::modbus::base::function_traits::maxAllowedQuantity(fc);
    if (fc == 0x05 || fc == 0x06) {
        quantitySpin_->setValue(1);
        quantitySpin_->setEnabled(false);
    } else {
        quantitySpin_->setEnabled(true);
        quantitySpin_->setMaximum(maxQty);
        if (quantitySpin_->value() > maxQty) {
            quantitySpin_->setValue(maxQty);
        }
    }

    if (::modbus::base::function_traits::isReadOperation(fc)) {
        writeGroup_->hide();
    } else {
        writeGroup_->show();
        if (fc == 0x05) writeStackedLayout_->setCurrentIndex(0);
        else if (fc == 0x06) writeStackedLayout_->setCurrentIndex(1);
        else if (fc == 0x0F) writeStackedLayout_->setCurrentIndex(2);
        else if (fc == 0x10) writeStackedLayout_->setCurrentIndex(3);
    }
    validateInputs();
}

void ModbusWorkbenchWidget::onSlaveIdChanged(int val) {
    if (val == 0 && isPolling_) {
        stopPollingInternal(true);
    }
    validateInputs();
}

void ModbusWorkbenchWidget::onAddressEdited() {
    auto res = ::modbus::address::toPduAddress(addressEdit_->text(), addressBase_);
    if (res.isValid) {
        pduAddress_ = res.pduAddress;
    }
    validateInputs();
}

void ModbusWorkbenchWidget::onQuantityChanged(int /*val*/) {
    validateInputs();
}

void ModbusWorkbenchWidget::onFc05Toggled(bool on) {
    fc05IsOn_ = on;
}

void ModbusWorkbenchWidget::onExecuteClicked() {
    if (isPolling_) {
        onPollCheckBoxToggled(false);
        return;
    }
    validateInputs();
    if (executeBtn_->isEnabled()) {
        emit executeRequested(currentSpec());
    }
}

void ModbusWorkbenchWidget::onPollCheckBoxToggled(bool checked) {
    if (checked) {
        if (slaveId() == 0) {
            stopPollingInternal(false);
            return;
        }
        validateInputs();
        if (addressWarningLabel_->isVisible()) {
            stopPollingInternal(false);
            return;
        }

        const uint8_t fc = functionCode();
        if (::modbus::base::function_traits::isWriteOperation(fc)) {
            if (!confirmWritePoll(fc)) {
                stopPollingInternal(false);
                return;
            }
            userConfirmedWritePoll_ = true;
            writePollWarningBadge_->show();
        } else {
            userConfirmedWritePoll_ = false;
            writePollWarningBadge_->hide();
        }

        isPolling_ = true;
        executeBtn_->setText(tr("Stop Polling (Esc)"));
        emit pollToggleRequested(true, currentSpec(), userConfirmedWritePoll_);
    } else {
        stopPollingInternal(true);
    }
}

void ModbusWorkbenchWidget::stopPollingInternal(bool emitSignal) {
    isPolling_ = false;
    userConfirmedWritePoll_ = false;
    writePollWarningBadge_->hide();
    executeBtn_->setText(tr("Execute (F5)"));
    pollCheckBox_->blockSignals(true);
    pollCheckBox_->setChecked(false);
    pollCheckBox_->blockSignals(false);
    if (emitSignal) {
        emit pollToggleRequested(false, currentSpec(), false);
    }
}

bool ModbusWorkbenchWidget::confirmWritePoll(uint8_t fc) {
    if (confirmWritePollHandler_) {
        return confirmWritePollHandler_(fc);
    }
    const auto btn = QMessageBox::warning(this, tr("High-Risk Operation Warning"),
        tr("Warning: The current function code is a write operation [FC 0x%1]. "
           "Periodic polling will continuously write physical data to the slave, "
           "which may cause frequent slave state changes or hardware damage. "
           "Are you sure you want to start write polling?")
            .arg(fc, 2, 16, QLatin1Char('0')),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return btn == QMessageBox::Yes;
}

void ModbusWorkbenchWidget::onPollIntervalChanged(int val) {
    emit pollIntervalChanged(val);
}

void ModbusWorkbenchWidget::onAddMbapClicked() {
    QByteArray pdu = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
    QByteArray adu;
    adu.resize(6);
    adu[0] = 0x00;
    adu[1] = 0x01;
    adu[2] = 0x00;
    adu[3] = 0x00;
    const uint16_t length = static_cast<uint16_t>(pdu.size() + 1);
    adu[4] = static_cast<char>((length >> 8) & 0xFF);
    adu[5] = static_cast<char>(length & 0xFF);
    adu.append(static_cast<char>(slaveIdSpin_->value()));
    adu.append(pdu);
    rawTextEdit_->setPlainText(adu.toHex(' ').toUpper());
}

void ModbusWorkbenchWidget::onAddCrcClicked() {
    QByteArray adu = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
    ::modbus::base::appendModbusRtuCrc(adu);
    rawTextEdit_->setPlainText(adu.toHex(' ').toUpper());
}

void ModbusWorkbenchWidget::onAddLrcClicked() {
    QByteArray adu = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
    ::modbus::base::appendModbusAsciiLrc(adu);
    rawTextEdit_->setPlainText(adu.toHex(' ').toUpper());
}

void ModbusWorkbenchWidget::onCleanHexClicked() {
    QByteArray raw = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
    rawTextEdit_->setPlainText(raw.toHex(' ').toUpper());
}

void ModbusWorkbenchWidget::updateAddressDisplay() {
    addressEdit_->setText(::modbus::address::toDisplayAddress(
        pduAddress_, addressBase_, radix_ == 16));
}

void ModbusWorkbenchWidget::validateAddressRange(bool& valid, QString& errorMsg) {
    auto addrRes = ::modbus::address::toPduAddress(addressEdit_->text(), addressBase_);
    if (!addrRes.isValid) {
        valid = false;
        errorMsg = addrRes.errorMessage;
    } else {
        pduAddress_ = addrRes.pduAddress;
        if (static_cast<uint32_t>(pduAddress_) + quantity() > 65536U) {
            valid = false;
            errorMsg = tr("Address overflow: Start (%1) + Qty (%2) > 65536").arg(pduAddress_).arg(quantity());
        }
    }
}

void ModbusWorkbenchWidget::validateWriteData(bool& valid, QString& errorMsg) {
    const uint8_t fc = functionCode();
    const uint16_t qty = quantity();

    if (fc == 0x06) {
        bool ok = false;
        const int val = fc06ValueEdit_->text().toInt(&ok, radix_);
        if (!ok || val < 0 || val > 65535) {
            valid = false;
            errorMsg = tr("Invalid register value (must be 0 ~ 65535)");
        }
    } else if (fc == 0x0F) {
        auto res = ::ui::common::data_helper::packCoilBits(fc0fBitsEdit_->text(), qty);
        if (res.isErr()) {
            valid = false;
            errorMsg = res.error();
        }
    } else if (fc == 0x10) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc10WordsEdit_->text(), radix_, qty);
        if (res.isErr()) {
            valid = false;
            errorMsg = res.error();
        }
    }
}

void ModbusWorkbenchWidget::validateInputs() {
    if (isRawMode_) {
        const QByteArray raw = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
        rawExecuteBtn_->setEnabled(!raw.isEmpty() && guardState_.rawSendEnabled);
        return;
    }

    bool valid = true;
    QString errorMsg;
    const uint8_t fc = functionCode();
    const uint8_t sid = slaveId();
    const uint16_t qty = quantity();

    if (sid == 0 && ::modbus::base::function_traits::isReadOperation(fc)) {
        valid = false;
        errorMsg = tr("Broadcast read (Slave ID 0) is forbidden per IEC 61158");
    }

    if (valid) {
        validateAddressRange(valid, errorMsg);
    }
    if (valid && ::modbus::base::function_traits::isWriteOperation(fc)) {
        validateWriteData(valid, errorMsg);
    }

    if (fc0fHintLabel_) {
        fc0fHintLabel_->setText(tr("Bit Sequence (required: %1 bits):").arg(qty));
    }
    if (fc10HintLabel_) {
        fc10HintLabel_->setText(tr("Register Values (required: %1 words):").arg(qty));
    }

    updateWarningAndActionState(valid, errorMsg, sid);
}

void ModbusWorkbenchWidget::updateWarningAndActionState(bool valid, const QString& errorMsg, uint8_t sid) {
    if (!valid) {
        addressWarningLabel_->setText(errorMsg);
        addressWarningLabel_->show();
        executeBtn_->setEnabled(false);
        addressEdit_->setStyleSheet(QStringLiteral("border: 1px solid #d32f2f;"));
    } else {
        addressWarningLabel_->hide();
        addressEdit_->setStyleSheet(QString());
        executeBtn_->setEnabled(guardState_.executeEnabled);
    }

    if (sid == 0) {
        pollCheckBox_->setEnabled(false);
        if (isPolling_) {
            stopPollingInternal(true);
        } else {
            pollCheckBox_->blockSignals(true);
            pollCheckBox_->setChecked(false);
            pollCheckBox_->blockSignals(false);
        }
        pollCheckBox_->setToolTip(tr("Broadcast (Slave ID 0) does not support periodic polling"));
    } else {
        pollCheckBox_->setEnabled(valid && guardState_.pollingEnabled);
        pollCheckBox_->setToolTip(QString());
    }
}

void ModbusWorkbenchWidget::loadWidgetValues() {
    using namespace core::common::settings_keys;
    if (settingsService_->contains(kModbusWorkbenchRadix)) {
        radix_ = settingsService_->value(kModbusWorkbenchRadix).toInt();
        if (radix_ != 16) { radix_ = 10; }
        (radix_ == 16 ? radixHexBtn_ : radixDecBtn_)->setChecked(true);
    }
    if (settingsService_->contains(kModbusWorkbenchSlaveId)) {
        slaveIdSpin_->setValue(settingsService_->value(kModbusWorkbenchSlaveId).toInt());
    }
    if (settingsService_->contains(kModbusWorkbenchStartAddress)) {
        pduAddress_ = static_cast<uint16_t>(settingsService_->value(kModbusWorkbenchStartAddress).toUInt());
    }
    if (settingsService_->contains(kModbusWorkbenchQuantity)) {
        quantitySpin_->setValue(settingsService_->value(kModbusWorkbenchQuantity).toInt());
    }
    if (settingsService_->contains(kModbusWorkbenchPollIntervalMs)) {
        pollIntervalSpin_->setValue(settingsService_->value(kModbusWorkbenchPollIntervalMs).toInt());
    }
    if (settingsService_->contains(kModbusWorkbenchFc05Value)) {
        setFc05Value(settingsService_->value(kModbusWorkbenchFc05Value).toBool());
    }
    if (settingsService_->contains(kModbusWorkbenchFc06Value)) {
        fc06ValueEdit_->setText(settingsService_->value(kModbusWorkbenchFc06Value).toString());
    }
    if (settingsService_->contains(kModbusWorkbenchFc0fValue)) {
        fc0fBitsEdit_->setText(settingsService_->value(kModbusWorkbenchFc0fValue).toString());
    }
    if (settingsService_->contains(kModbusWorkbenchFc10Value)) {
        fc10WordsEdit_->setText(settingsService_->value(kModbusWorkbenchFc10Value).toString());
    }
    if (settingsService_->contains(kModbusWorkbenchRawText)) {
        rawTextEdit_->setPlainText(settingsService_->value(kModbusWorkbenchRawText).toString());
    }
}

void ModbusWorkbenchWidget::loadSettings() {
    if (!settingsService_) {
        updateAddressDisplay();
        validateInputs();
        return;
    }
    using namespace core::common::settings_keys;
    loadWidgetValues();

    uint8_t fc = 0x03;
    if (settingsService_->contains(kModbusWorkbenchFunctionCode)) {
        const auto savedFc = static_cast<uint8_t>(settingsService_->value(kModbusWorkbenchFunctionCode).toUInt());
        if (!::modbus::base::function_traits::isWriteOperation(savedFc)) {
            fc = savedFc;
        }
    }
    setFunctionCode(fc);

    if (settingsService_->contains(kModbusWorkbenchMode)) {
        setMode(settingsService_->value(kModbusWorkbenchMode).toInt());
    }
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::saveSettings() {
    if (!settingsService_) {
        return;
    }
    using namespace core::common::settings_keys;
    settingsService_->setValue(kModbusWorkbenchMode, isRawMode_ ? 1 : 0);
    settingsService_->setValue(kModbusWorkbenchSlaveId, slaveIdSpin_->value());
    settingsService_->setValue(kModbusWorkbenchFunctionCode, functionCode());
    settingsService_->setValue(kModbusWorkbenchStartAddress, pduAddress_);
    settingsService_->setValue(kModbusWorkbenchQuantity, quantitySpin_->value());
    settingsService_->setValue(kModbusWorkbenchRadix, radix_);
    settingsService_->setValue(kModbusWorkbenchPollIntervalMs, pollIntervalSpin_->value());
    settingsService_->setValue(kModbusWorkbenchFc05Value, fc05IsOn_);
    settingsService_->setValue(kModbusWorkbenchFc06Value, fc06ValueEdit_->text());
    settingsService_->setValue(kModbusWorkbenchFc0fValue, fc0fBitsEdit_->text());
    settingsService_->setValue(kModbusWorkbenchFc10Value, fc10WordsEdit_->text());
    settingsService_->setValue(kModbusWorkbenchRawText, rawTextEdit_->toPlainText());
    settingsService_->sync();
}

void ModbusWorkbenchWidget::retranslateUi() {
    standardModeBtn_->setText(tr("Standard"));
    rawModeBtn_->setText(tr("Raw Frame"));
    executeBtn_->setText(isPolling_ ? tr("Stop Polling (Esc)") : tr("Execute (F5)"));
    rawExecuteBtn_->setText(tr("Send Raw Frame"));
    pollCheckBox_->setText(tr("Periodic Polling"));
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets::modbus
