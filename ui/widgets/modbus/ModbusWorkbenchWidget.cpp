// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusWorkbenchWidget.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFontDatabase>
#include <QFormLayout>
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
        if (!isPolling_) {
            writePollWarningBadge_->hide();
            userConfirmedWritePoll_ = false;
        }
    }
    rawExecuteBtn_->setEnabled(guards.rawSendEnabled);
    updateExecuteButtonAppearance();
    validateInputs();
    updateGuardAppearance();
}

void ModbusWorkbenchWidget::setAddressBase(::modbus::address::AddressBase base) {
    addressBase_ = base;
    if (addrBaseCombo_) {
        const int idx = (base == ::modbus::address::AddressBase::PlcAddress1Based) ? 1 : 0;
        if (addrBaseCombo_->currentIndex() != idx) {
            const QSignalBlocker blocker(addrBaseCombo_);
            addrBaseCombo_->setCurrentIndex(idx);
        }
    }
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
        auto res = ::ui::common::data_helper::parseRegisterWords(fc06ValueEdit_->text(), dataRadix_, 1);
        if (res.isOk()) {
            spec.writeData = res.value();
        }
    } else if (spec.functionCode == 0x0F) {
        auto res = ::ui::common::data_helper::packCoilBits(fc0fBitsEdit_->text(), spec.quantity);
        if (res.isOk()) {
            spec.writeData = res.value();
        }
    } else if (spec.functionCode == 0x10) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc10WordsEdit_->text(), dataRadix_, spec.quantity);
        if (res.isOk()) {
            spec.writeData = res.value();
        }
    }
    return spec;
}

int ModbusWorkbenchWidget::pollingIntervalMs() const noexcept { return pollIntervalSpin_->value(); }
uint8_t ModbusWorkbenchWidget::slaveId() const noexcept { return static_cast<uint8_t>(slaveIdSpin_->value()); }
uint8_t ModbusWorkbenchWidget::functionCode() const noexcept { return static_cast<uint8_t>(fcCombo_->currentData().toUInt()); }
uint16_t ModbusWorkbenchWidget::quantity() const noexcept { return static_cast<uint16_t>(quantitySpin_->value()); }

void ModbusWorkbenchWidget::setMode(int mode) {
    (mode == 0 ? standardModeBtn_ : rawModeBtn_)->setChecked(true);
    onModeChanged(mode == 0 ? 0 : 1);
}

void ModbusWorkbenchWidget::setSlaveId(uint8_t slaveId) { slaveIdSpin_->setValue(slaveId); }

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

void ModbusWorkbenchWidget::setQuantity(uint16_t qty) { quantitySpin_->setValue(qty); }

void ModbusWorkbenchWidget::setRadix(int radix) {
    (radix == 16 ? radixHexBtn_ : radixDecBtn_)->setChecked(true);
    onRadixChanged(radix == 16 ? 16 : 10);
}

void ModbusWorkbenchWidget::setDataRadix(int radix) {
    if ((radix != 10 && radix != 16 && radix != 2) || functionCode() == 0x05 || functionCode() == 0x0F) {
        return;
    }
    if (radix == 16) { dataRadixHexBtn_->setChecked(true); }
    else if (radix == 2) { dataRadixBinBtn_->setChecked(true); }
    else { dataRadixDecBtn_->setChecked(true); }
    onDataRadixChanged(radix);
}

void ModbusWorkbenchWidget::setPollingIntervalMs(int ms) { pollIntervalSpin_->setValue(ms); }

void ModbusWorkbenchWidget::setFc05Value(bool on) {
    fc05IsOn_ = on;
    (on ? fc05OnBtn_ : fc05OffBtn_)->setChecked(true);
}

void ModbusWorkbenchWidget::setWritePayload(const QString& text) {
    writePayloadDirty_ = true;
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

void ModbusWorkbenchWidget::setupModeToggle(QWidget* container, QVBoxLayout* rootLayout) {
    auto* topBarLayout = new QHBoxLayout();
    modeGroup_ = new QButtonGroup(this);
    standardModeBtn_ = new QPushButton(container);
    standardModeBtn_->setObjectName(QStringLiteral("standardModeBtn"));
    standardModeBtn_->setCheckable(true);
    standardModeBtn_->setChecked(true);
    rawModeBtn_ = new QPushButton(container);
    rawModeBtn_->setObjectName(QStringLiteral("rawModeBtn"));
    rawModeBtn_->setCheckable(true);
    modeGroup_->addButton(standardModeBtn_, 0);
    modeGroup_->addButton(rawModeBtn_, 1);
    connect(modeGroup_, &QButtonGroup::idClicked, this, &ModbusWorkbenchWidget::onModeChanged);
    topBarLayout->addWidget(standardModeBtn_);
    topBarLayout->addWidget(rawModeBtn_);
    rootLayout->addLayout(topBarLayout);
}

void ModbusWorkbenchWidget::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* container = new QWidget(scrollArea);
    container->setObjectName(QStringLiteral("workbenchContainer"));
    auto* rootLayout = new QVBoxLayout(container);
    rootLayout->setObjectName(QStringLiteral("workbenchRootLayout"));
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->setSpacing(6);

    setupModeToggle(container, rootLayout);

    auto* stackWidget = new QWidget(container);
    mainStackedLayout_ = new QStackedLayout(stackWidget);
    mainStackedLayout_->addWidget(createStandardWidget());
    mainStackedLayout_->addWidget(createRawWidget());
    rootLayout->addWidget(stackWidget);

    scrollArea->setWidget(container);
    outerLayout->addWidget(scrollArea);
    retranslateUi();
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
    addressingGroup_ = new QGroupBox(parent);
    addressingGroup_->setObjectName(QStringLiteral("workbenchAddressingGroup"));
    auto* gl = new QVBoxLayout(addressingGroup_);
    gl->setSpacing(6);

    setupFunctionCodeSelector(addressingGroup_, gl);
    setupAddressingForm(addressingGroup_, gl);

    addressWarningLabel_ = new QLabel(addressingGroup_);
    addressWarningLabel_->setObjectName(QStringLiteral("workbenchAddressWarning"));
    addressWarningLabel_->setStyleSheet(QStringLiteral("color: #d32f2f; font-weight: bold;"));
    addressWarningLabel_->setWordWrap(true);
    addressWarningLabel_->hide();
    gl->addWidget(addressWarningLabel_);

    layout->addWidget(addressingGroup_);
}

void ModbusWorkbenchWidget::setupFunctionCodeSelector(QGroupBox* group, QVBoxLayout* gl) {
    fcLabel_ = new QLabel(group);
    gl->addWidget(fcLabel_);
    fcCombo_ = new QComboBox(group);
    fcCombo_->setObjectName(QStringLiteral("workbenchFcCombo"));
    fcCombo_->addItem(QString(), 0x01);
    fcCombo_->addItem(QString(), 0x02);
    fcCombo_->addItem(QString(), 0x03);
    fcCombo_->addItem(QString(), 0x04);
    fcCombo_->addItem(QString(), 0x05);
    fcCombo_->addItem(QString(), 0x06);
    fcCombo_->addItem(QString(), 0x0F);
    fcCombo_->addItem(QString(), 0x10);
    fcCombo_->setCurrentIndex(2);
    connect(fcCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusWorkbenchWidget::onFunctionCodeChanged);
    gl->addWidget(fcCombo_);
}

void ModbusWorkbenchWidget::setupAddressingForm(QGroupBox* group, QVBoxLayout* gl) {
    auto* form = new QFormLayout();
    form->setObjectName(QStringLiteral("workbenchAddressingFormLayout"));
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setHorizontalSpacing(8);
    form->setVerticalSpacing(6);

    setupRadixRow(group, form);
    setupAddressBaseRow(group, form);
    setupSlaveIdRow(group, form);
    setupAddressAndQuantityRows(group, form);

    gl->addLayout(form);
}

void ModbusWorkbenchWidget::setupRadixRow(QGroupBox* group, QFormLayout* form) {
    radixLabel_ = new QLabel(group);
    auto* radixLayout = new QHBoxLayout();
    radixLayout->setContentsMargins(0, 0, 0, 0);
    radixGroup_ = new QButtonGroup(this);
    radixDecBtn_ = new QPushButton(group);
    radixDecBtn_->setObjectName(QStringLiteral("radixDecBtn"));
    radixDecBtn_->setCheckable(true);
    radixDecBtn_->setChecked(true);
    radixHexBtn_ = new QPushButton(group);
    radixHexBtn_->setObjectName(QStringLiteral("radixHexBtn"));
    radixHexBtn_->setCheckable(true);
    radixGroup_->addButton(radixDecBtn_, 10);
    radixGroup_->addButton(radixHexBtn_, 16);
    connect(radixGroup_, &QButtonGroup::idClicked, this, &ModbusWorkbenchWidget::onRadixChanged);
    radixLayout->addWidget(radixDecBtn_);
    radixLayout->addWidget(radixHexBtn_);
    radixLayout->addStretch(1);
    form->addRow(radixLabel_, radixLayout);
}

void ModbusWorkbenchWidget::setupAddressBaseRow(QGroupBox* group, QFormLayout* form) {
    addrBaseLabel_ = new QLabel(group);
    addrBaseCombo_ = new QComboBox(group);
    addrBaseCombo_->setObjectName(QStringLiteral("workbenchAddrBaseCombo"));
    addrBaseCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    addrBaseCombo_->addItem(QString(), static_cast<int>(::modbus::address::AddressBase::Offset0Based));
    addrBaseCombo_->addItem(QString(), static_cast<int>(::modbus::address::AddressBase::PlcAddress1Based));
    connect(addrBaseCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                const auto base = (idx == 1) ? ::modbus::address::AddressBase::PlcAddress1Based
                                             : ::modbus::address::AddressBase::Offset0Based;
                setAddressBase(base);
                emit addressBaseChanged(base);
            });
    form->addRow(addrBaseLabel_, addrBaseCombo_);
}

void ModbusWorkbenchWidget::setupSlaveIdRow(QGroupBox* group, QFormLayout* form) {
    slaveIdLabel_ = new QLabel(group);
    slaveIdSpin_ = new QSpinBox(group);
    slaveIdSpin_->setObjectName(QStringLiteral("workbenchSlaveIdSpin"));
    slaveIdSpin_->setRange(0, 247);
    slaveIdSpin_->setValue(1);
    connect(slaveIdSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusWorkbenchWidget::onSlaveIdChanged);
    form->addRow(slaveIdLabel_, slaveIdSpin_);
}

void ModbusWorkbenchWidget::setupAddressAndQuantityRows(QGroupBox* group, QFormLayout* form) {
    startAddrLabel_ = new QLabel(group);
    auto* addrContainer = new QWidget(group);
    auto* addrLayout = new QVBoxLayout(addrContainer);
    addrLayout->setContentsMargins(0, 0, 0, 0);
    addrLayout->setSpacing(2);

    addressEdit_ = new QLineEdit(addrContainer);
    addressEdit_->setObjectName(QStringLiteral("workbenchAddressEdit"));
    addressEdit_->setText(QStringLiteral("0"));
    connect(addressEdit_, &QLineEdit::textEdited, this, &ModbusWorkbenchWidget::onAddressEdited);
    addrLayout->addWidget(addressEdit_);

    addressPreviewLabel_ = new QLabel(addrContainer);
    addressPreviewLabel_->setObjectName(QStringLiteral("workbenchAddressPreview"));
    addressPreviewLabel_->setStyleSheet(QStringLiteral("color: #757575; font-size: 11px;"));
    addrLayout->addWidget(addressPreviewLabel_);
    form->addRow(startAddrLabel_, addrContainer);

    quantityLabel_ = new QLabel(group);
    quantitySpin_ = new QSpinBox(group);
    quantitySpin_->setObjectName(QStringLiteral("workbenchQuantitySpin"));
    quantitySpin_->setRange(1, 125);
    quantitySpin_->setValue(10);
    connect(quantitySpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusWorkbenchWidget::onQuantityChanged);
    form->addRow(quantityLabel_, quantitySpin_);
}

void ModbusWorkbenchWidget::setupWriteSection(QWidget* parent, QVBoxLayout* layout) {
    writeGroup_ = new QGroupBox(parent);
    writeGroup_->setObjectName(QStringLiteral("workbenchWriteGroup"));
    auto* wl = new QVBoxLayout(writeGroup_);
    wl->setSpacing(6);

    setupDataRadixRow(writeGroup_, wl);

    auto* writeStackWidget = new QWidget(writeGroup_);
    writeStackedLayout_ = new QStackedLayout(writeStackWidget);
    setupWriteStackPages(writeStackWidget);
    wl->addWidget(writeStackWidget);

    writeWarningLabel_ = new QLabel(writeGroup_);
    writeWarningLabel_->setObjectName(QStringLiteral("workbenchWriteWarning"));
    writeWarningLabel_->setStyleSheet(QStringLiteral("color: #d32f2f; font-weight: bold;"));
    writeWarningLabel_->setWordWrap(true);
    writeWarningLabel_->hide();
    wl->addWidget(writeWarningLabel_);

    layout->addWidget(writeGroup_);
}

void ModbusWorkbenchWidget::setupDataRadixRow(QGroupBox* group, QVBoxLayout* wl) {
    auto* rowLayout = new QHBoxLayout();
    rowLayout->setContentsMargins(0, 0, 0, 0);
    dataRadixLabel_ = new QLabel(group);
    dataRadixGroup_ = new QButtonGroup(this);
    dataRadixDecBtn_ = new QPushButton(group);
    dataRadixDecBtn_->setObjectName(QStringLiteral("workbenchDataRadixDecBtn"));
    dataRadixDecBtn_->setCheckable(true);
    dataRadixDecBtn_->setChecked(true);
    dataRadixHexBtn_ = new QPushButton(group);
    dataRadixHexBtn_->setObjectName(QStringLiteral("workbenchDataRadixHexBtn"));
    dataRadixHexBtn_->setCheckable(true);
    dataRadixBinBtn_ = new QPushButton(group);
    dataRadixBinBtn_->setObjectName(QStringLiteral("workbenchDataRadixBinBtn"));
    dataRadixBinBtn_->setCheckable(true);
    dataRadixGroup_->addButton(dataRadixDecBtn_, 10);
    dataRadixGroup_->addButton(dataRadixHexBtn_, 16);
    dataRadixGroup_->addButton(dataRadixBinBtn_, 2);
    connect(dataRadixGroup_, &QButtonGroup::idClicked, this, &ModbusWorkbenchWidget::onDataRadixChanged);
    rowLayout->addWidget(dataRadixLabel_);
    rowLayout->addWidget(dataRadixDecBtn_);
    rowLayout->addWidget(dataRadixHexBtn_);
    rowLayout->addWidget(dataRadixBinBtn_);
    rowLayout->addStretch(1);
    wl->addLayout(rowLayout);
}

void ModbusWorkbenchWidget::setupWriteStackPages(QWidget* stackWidget) {
    auto* pRead = new QWidget(stackWidget);
    auto* lRead = new QVBoxLayout(pRead);
    lRead->setContentsMargins(0, 4, 0, 4);
    readPlaceholderLabel_ = new QLabel(pRead);
    readPlaceholderLabel_->setObjectName(QStringLiteral("workbenchReadPlaceholder"));
    readPlaceholderLabel_->setStyleSheet(QStringLiteral("color: #757575; font-size: 11px; font-style: italic;"));
    readPlaceholderLabel_->setWordWrap(true);
    lRead->addWidget(readPlaceholderLabel_);
    writeStackedLayout_->addWidget(pRead);

    auto* p0 = new QWidget(stackWidget);
    auto* l0 = new QHBoxLayout(p0);
    fc05Group_ = new QButtonGroup(this);
    fc05OnBtn_ = new QPushButton(p0);
    fc05OnBtn_->setObjectName(QStringLiteral("workbenchFc05OnBtn"));
    fc05OnBtn_->setCheckable(true);
    fc05OnBtn_->setChecked(true);
    fc05OffBtn_ = new QPushButton(p0);
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

    auto addPage = [&](QLabel*& lbl, QLineEdit*& edt, const QString& objName, const QString& initText = {}) {
        auto* p = new QWidget(stackWidget);
        auto* l = new QVBoxLayout(p);
        lbl = new QLabel(p);
        l->addWidget(lbl);
        edt = new QLineEdit(p);
        edt->setObjectName(objName);
        if (!initText.isEmpty()) {
            edt->setText(initText);
        }
        connect(edt, &QLineEdit::textEdited, this, [this] {
            writePayloadDirty_ = true;
            validateInputs();
        });
        l->addWidget(edt);
        writeStackedLayout_->addWidget(p);
    };
    addPage(fc06ValueLabel_, fc06ValueEdit_, QStringLiteral("workbenchFc06ValueEdit"), QStringLiteral("0"));
    addPage(fc0fHintLabel_, fc0fBitsEdit_, QStringLiteral("workbenchFc0fBitsEdit"));
    addPage(fc10HintLabel_, fc10WordsEdit_, QStringLiteral("workbenchFc10WordsEdit"));
}

void ModbusWorkbenchWidget::setupPollControls(QVBoxLayout* gl) {
    auto* pollLayout = new QHBoxLayout();
    pollCheckBox_ = new QCheckBox(executionGroup_);
    pollCheckBox_->setObjectName(QStringLiteral("workbenchPollCheck"));
    connect(pollCheckBox_, &QCheckBox::toggled, this, &ModbusWorkbenchWidget::onPollCheckBoxToggled);
    pollLayout->addWidget(pollCheckBox_);

    pollIntervalLabel_ = new QLabel(executionGroup_);
    pollLayout->addWidget(pollIntervalLabel_);
    pollIntervalSpin_ = new QSpinBox(executionGroup_);
    pollIntervalSpin_->setObjectName(QStringLiteral("workbenchPollIntervalSpin"));
    pollIntervalSpin_->setRange(50, 60000);
    pollIntervalSpin_->setSingleStep(100);
    pollIntervalSpin_->setValue(1000);
    pollIntervalSpin_->setSuffix(QStringLiteral(" ms"));
    connect(pollIntervalSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusWorkbenchWidget::onPollIntervalChanged);
    pollLayout->addWidget(pollIntervalSpin_);
    gl->addLayout(pollLayout);
}

void ModbusWorkbenchWidget::setupExecutionSection(QWidget* parent, QVBoxLayout* layout) {
    executionGroup_ = new QGroupBox(parent);
    executionGroup_->setObjectName(QStringLiteral("workbenchExecutionGroup"));
    auto* gl = new QVBoxLayout(executionGroup_);
    gl->setSpacing(6);

    executeBtn_ = new QPushButton(executionGroup_);
    executeBtn_->setObjectName(QStringLiteral("workbenchExecuteBtn"));
    connect(executeBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onExecuteClicked);
    gl->addWidget(executeBtn_);

    setupPollControls(gl);

    writePollWarningBadge_ = new QLabel(executionGroup_);
    writePollWarningBadge_->setObjectName(QStringLiteral("workbenchWritePollBadge"));
    writePollWarningBadge_->setStyleSheet(QStringLiteral("color: #e65100; font-weight: bold;"));
    writePollWarningBadge_->hide();
    gl->addWidget(writePollWarningBadge_);

    guardHintLabel_ = new QLabel(executionGroup_);
    guardHintLabel_->setObjectName(QStringLiteral("workbenchGuardHint"));
    guardHintLabel_->setStyleSheet(QStringLiteral("color: #757575; font-size: 11px;"));
    guardHintLabel_->setWordWrap(true);
    guardHintLabel_->hide();
    gl->addWidget(guardHintLabel_);

    updateExecuteButtonAppearance();
    layout->addWidget(executionGroup_);
}

QWidget* ModbusWorkbenchWidget::createRawWidget() {
    auto* w = new QWidget(this);
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(8);

    rawGroup_ = new QGroupBox(w);
    rawGroup_->setObjectName(QStringLiteral("workbenchRawGroup"));
    auto* el = new QVBoxLayout(rawGroup_);

    rawTextEdit_ = new QPlainTextEdit(rawGroup_);
    rawTextEdit_->setObjectName(QStringLiteral("workbenchRawTextEdit"));
    rawTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(rawTextEdit_, &QPlainTextEdit::textChanged, this, &ModbusWorkbenchWidget::validateInputs);
    el->addWidget(rawTextEdit_);

    setupRawButtons(rawGroup_, el);

    l->addWidget(rawGroup_);
    l->addStretch(1);
    return w;
}

void ModbusWorkbenchWidget::setupRawButtons(QGroupBox* editGroup, QVBoxLayout* el) {
    auto* btnLayout = new QHBoxLayout();
    addMbapBtn_ = new QPushButton(editGroup);
    addMbapBtn_->setObjectName(QStringLiteral("workbenchAddMbapBtn"));
    connect(addMbapBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onAddMbapClicked);

    addCrcBtn_ = new QPushButton(editGroup);
    addCrcBtn_->setObjectName(QStringLiteral("workbenchAddCrcBtn"));
    connect(addCrcBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onAddCrcClicked);

    addLrcBtn_ = new QPushButton(editGroup);
    addLrcBtn_->setObjectName(QStringLiteral("workbenchAddLrcBtn"));
    connect(addLrcBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onAddLrcClicked);

    cleanHexBtn_ = new QPushButton(editGroup);
    cleanHexBtn_->setObjectName(QStringLiteral("workbenchCleanHexBtn"));
    connect(cleanHexBtn_, &QPushButton::clicked, this, &ModbusWorkbenchWidget::onCleanHexClicked);

    btnLayout->addWidget(addMbapBtn_);
    btnLayout->addWidget(addCrcBtn_);
    btnLayout->addWidget(addLrcBtn_);
    btnLayout->addWidget(cleanHexBtn_);
    el->addLayout(btnLayout);

    rawExecuteBtn_ = new QPushButton(editGroup);
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
    radix_ = id;
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::onDataRadixChanged(int id) {
    if (id != 10 && id != 16 && id != 2) {
        return;
    }
    const int oldRadix = dataRadix_;
    dataRadix_ = id;
    const uint8_t fc = functionCode();
    if (fc == 0x06 || fc == 0x10) {
        registerDataRadix_ = id;
        if (fc == 0x06) {
            const QString oldText = fc06ValueEdit_->text().trimmed();
            if (!oldText.isEmpty()) {
                fc06ValueEdit_->setText(::ui::common::data_helper::convertRadix(oldText, oldRadix, dataRadix_));
            }
        } else if (fc == 0x10) {
            const QString oldText = fc10WordsEdit_->text().trimmed();
            if (!oldText.isEmpty()) {
                fc10WordsEdit_->setText(::ui::common::data_helper::convertRadix(oldText, oldRadix, dataRadix_));
            }
        }
    }
    updateDataRadixHints();
    validateInputs();
}

void ModbusWorkbenchWidget::updateRequestDataState(uint8_t fc) {
    const bool isRead = ::modbus::base::function_traits::isReadOperation(fc);
    const bool isCoil = (fc == 0x05 || fc == 0x0F);
    const bool isRegister = (fc == 0x06 || fc == 0x10);

    if (isRead) {
        writeStackedLayout_->setCurrentIndex(0);
        dataRadixDecBtn_->setEnabled(false);
        dataRadixHexBtn_->setEnabled(false);
        dataRadixBinBtn_->setEnabled(false);
    } else if (isCoil) {
        writeStackedLayout_->setCurrentIndex(fc == 0x05 ? 1 : 3);
        dataRadix_ = 2;
        dataRadixBinBtn_->setChecked(true);
        dataRadixDecBtn_->setEnabled(false);
        dataRadixHexBtn_->setEnabled(false);
        dataRadixBinBtn_->setEnabled(true);
    } else if (isRegister) {
        writeStackedLayout_->setCurrentIndex(fc == 0x06 ? 2 : 4);
        dataRadixDecBtn_->setEnabled(true);
        dataRadixHexBtn_->setEnabled(true);
        dataRadixBinBtn_->setEnabled(true);
        dataRadix_ = registerDataRadix_;
        if (dataRadix_ == 16) {
            dataRadixHexBtn_->setChecked(true);
        } else if (dataRadix_ == 2) {
            dataRadixBinBtn_->setChecked(true);
        } else {
            dataRadix_ = 10;
            dataRadixDecBtn_->setChecked(true);
        }
    }
    updateDataRadixHints();
}

void ModbusWorkbenchWidget::updateDataRadixHints() {
    const uint16_t qty = quantity();
    if (fc0fHintLabel_) {
        fc0fHintLabel_->setText(tr("Coil Bits (LSB-first: left = start address, %1 bits):").arg(qty));
    }
    if (fc10HintLabel_) {
        fc10HintLabel_->setText(tr("Register Values (required: %1 words):").arg(qty));
    }
    if (fc06ValueLabel_) {
        if (dataRadix_ == 16) {
            fc06ValueLabel_->setText(tr("Register Value (0x0000 ~ 0xFFFF):"));
            fc06ValueEdit_->setPlaceholderText(QStringLiteral("0x0000"));
        } else if (dataRadix_ == 2) {
            fc06ValueLabel_->setText(tr("Register Value (16-bit 0/1):"));
            fc06ValueEdit_->setPlaceholderText(QStringLiteral("0000000000000000"));
        } else {
            fc06ValueLabel_->setText(tr("Register Value (0 ~ 65535):"));
            fc06ValueEdit_->setPlaceholderText(QStringLiteral("0"));
        }
    }
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

    writePayloadDirty_ = false;
    updateRequestDataState(fc);
    updateExecuteButtonAppearance();
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

void ModbusWorkbenchWidget::onQuantityChanged(int /*val*/) { validateInputs(); }
void ModbusWorkbenchWidget::onFc05Toggled(bool on) { fc05IsOn_ = on; }

void ModbusWorkbenchWidget::onExecuteClicked() {
    if (isPolling_) {
        onPollCheckBoxToggled(false);
        return;
    }
    writePayloadDirty_ = true;
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
        writePayloadDirty_ = true;
        validateInputs();
        if (addressWarningLabel_->isVisible() || writeWarningLabel_->isVisible()) {
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
        updateExecuteButtonAppearance();
        emit pollToggleRequested(true, currentSpec(), userConfirmedWritePoll_);
    } else {
        stopPollingInternal(true);
    }
}

void ModbusWorkbenchWidget::stopPollingInternal(bool emitSignal) {
    isPolling_ = false;
    userConfirmedWritePoll_ = false;
    writePollWarningBadge_->hide();
    updateExecuteButtonAppearance();
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

void ModbusWorkbenchWidget::onPollIntervalChanged(int val) { emit pollIntervalChanged(val); }

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
    if (addressPreviewLabel_) {
        const auto hexStr = QString::number(pduAddress_, 16).rightJustified(4, QLatin1Char('0')).toUpper();
        addressPreviewLabel_->setText(tr("PDU: 0x%1 (%2)").arg(hexStr).arg(pduAddress_));
    }
}

void ModbusWorkbenchWidget::validateAddressRange(bool& valid, QString& errorMsg) {
    auto addrRes = ::modbus::address::toPduAddress(addressEdit_->text(), addressBase_);
    if (!addrRes.isValid) {
        valid = false;
        errorMsg = addrRes.errorMessage;
        if (addressPreviewLabel_) {
            addressPreviewLabel_->setText(tr("PDU: --"));
        }
    } else {
        pduAddress_ = addrRes.pduAddress;
        if (addressPreviewLabel_) {
            const auto hexStr = QString::number(pduAddress_, 16).rightJustified(4, QLatin1Char('0')).toUpper();
            addressPreviewLabel_->setText(tr("PDU: 0x%1 (%2)").arg(hexStr).arg(pduAddress_));
        }
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
        auto res = ::ui::common::data_helper::parseRegisterWords(fc06ValueEdit_->text(), dataRadix_, 1);
        if (res.isErr()) {
            valid = false;
            errorMsg = res.error();
        }
    } else if (fc == 0x0F) {
        auto res = ::ui::common::data_helper::packCoilBits(fc0fBitsEdit_->text(), qty);
        if (res.isErr()) {
            valid = false;
            errorMsg = res.error();
        }
    } else if (fc == 0x10) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc10WordsEdit_->text(), dataRadix_, qty);
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

    bool addrValid = true;
    QString addrError;
    const uint8_t fc = functionCode();
    const uint8_t sid = slaveId();

    if (sid == 0 && ::modbus::base::function_traits::isReadOperation(fc)) {
        addrValid = false;
        addrError = tr("Broadcast read (Slave ID 0) is forbidden per IEC 61158");
    }

    if (addrValid) {
        validateAddressRange(addrValid, addrError);
    }

    bool writeValid = true;
    QString writeError;
    if (::modbus::base::function_traits::isWriteOperation(fc)) {
        validateWriteData(writeValid, writeError);
    }

    updateDataRadixHints();
    updateWarningAndActionState(addrValid, addrError, writeValid, writeError, sid);
}

void ModbusWorkbenchWidget::updateWarningAndActionState(
    bool addrValid, const QString& addrError, bool writeValid, const QString& writeError, uint8_t sid) {
    if (!addrValid) {
        addressWarningLabel_->setText(addrError);
        addressWarningLabel_->show();
        addressEdit_->setStyleSheet(QStringLiteral("border: 1px solid #d32f2f;"));
    } else {
        addressWarningLabel_->hide();
        addressEdit_->setStyleSheet(QString());
    }

    if (!writeValid && writePayloadDirty_) {
        writeWarningLabel_->setText(writeError);
        writeWarningLabel_->show();
    } else {
        writeWarningLabel_->hide();
    }

    const bool overallValid = addrValid && writeValid;
    executeBtn_->setEnabled(overallValid && guardState_.executeEnabled);

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
        pollCheckBox_->setEnabled(overallValid && guardState_.pollingEnabled);
        pollCheckBox_->setToolTip(!guardState_.pollingEnabled ? guardState_.tooltipMessage : QString());
    }
    updateGuardAppearance();
}

void ModbusWorkbenchWidget::loadAddressingSettings() {
    using namespace core::common::settings_keys;
    if (settingsService_->contains(kModbusWorkbenchRadix)) {
        radix_ = settingsService_->value(kModbusWorkbenchRadix).toInt();
        if (radix_ != 16) { radix_ = 10; }
        (radix_ == 16 ? radixHexBtn_ : radixDecBtn_)->setChecked(true);
    }
    if (settingsService_->contains(kModbusWorkbenchDataRadix)) {
        registerDataRadix_ = settingsService_->value(kModbusWorkbenchDataRadix).toInt();
        if (registerDataRadix_ != 16 && registerDataRadix_ != 2) { registerDataRadix_ = 10; }
    } else if (settingsService_->contains(kModbusWorkbenchRadix)) {
        registerDataRadix_ = settingsService_->value(kModbusWorkbenchRadix).toInt();
        if (registerDataRadix_ != 16) { registerDataRadix_ = 10; }
    }
    dataRadix_ = registerDataRadix_;
    if (settingsService_->contains(kModbusWorkbenchSlaveId)) {
        slaveIdSpin_->setValue(settingsService_->value(kModbusWorkbenchSlaveId).toInt());
    }
    if (settingsService_->contains(kModbusWorkbenchStartAddress)) {
        pduAddress_ = static_cast<uint16_t>(settingsService_->value(kModbusWorkbenchStartAddress).toUInt());
    }
    if (settingsService_->contains(kModbusWorkbenchQuantity)) {
        quantitySpin_->setValue(settingsService_->value(kModbusWorkbenchQuantity).toInt());
    }
}

void ModbusWorkbenchWidget::loadExecutionAndPayloadSettings() {
    using namespace core::common::settings_keys;
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
    loadAddressingSettings();
    loadExecutionAndPayloadSettings();

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
    settingsService_->setValue(kModbusWorkbenchDataRadix, registerDataRadix_);
    settingsService_->setValue(kModbusWorkbenchPollIntervalMs, pollIntervalSpin_->value());
    settingsService_->setValue(kModbusWorkbenchFc05Value, fc05IsOn_);
    settingsService_->setValue(kModbusWorkbenchFc06Value, fc06ValueEdit_->text());
    settingsService_->setValue(kModbusWorkbenchFc0fValue, fc0fBitsEdit_->text());
    settingsService_->setValue(kModbusWorkbenchFc10Value, fc10WordsEdit_->text());
    settingsService_->setValue(kModbusWorkbenchRawText, rawTextEdit_->toPlainText());
    settingsService_->sync();
}

void ModbusWorkbenchWidget::retranslateAddressingUi() {
    standardModeBtn_->setText(tr("Standard"));
    rawModeBtn_->setText(tr("Raw Frame"));
    addressingGroup_->setTitle(tr("Addressing & Parameters").replace(QLatin1Char('&'), QStringLiteral("&&")));
    radixLabel_->setText(tr("Address Radix:"));
    radixDecBtn_->setText(tr("DEC"));
    radixHexBtn_->setText(tr("HEX"));
    if (addrBaseLabel_) {
        addrBaseLabel_->setText(tr("Address Base:"));
    }
    if (addrBaseCombo_) {
        const QSignalBlocker blocker(addrBaseCombo_);
        addrBaseCombo_->setItemText(0, tr("0-Based"));
        addrBaseCombo_->setItemText(1, tr("1-Based"));
    }
    fcLabel_->setText(tr("Function Code:"));
    const int currentFcIdx = fcCombo_->currentIndex();
    {
        const QSignalBlocker blocker(fcCombo_);
        fcCombo_->setItemText(0, tr("0x01 Read Coils"));
        fcCombo_->setItemText(1, tr("0x02 Read Discrete Inputs"));
        fcCombo_->setItemText(2, tr("0x03 Read Holding Registers"));
        fcCombo_->setItemText(3, tr("0x04 Read Input Registers"));
        fcCombo_->setItemText(4, tr("0x05 Write Single Coil"));
        fcCombo_->setItemText(5, tr("0x06 Write Single Register"));
        fcCombo_->setItemText(6, tr("0x0F Write Multiple Coils"));
        fcCombo_->setItemText(7, tr("0x10 Write Multiple Registers"));
        fcCombo_->setCurrentIndex(currentFcIdx);
    }
    slaveIdLabel_->setText(tr("Slave ID (0-247):"));
    startAddrLabel_->setText(tr("Start Address:"));
    quantityLabel_->setText(tr("Quantity:"));
}

void ModbusWorkbenchWidget::retranslateWriteAndRawUi() {
    writeGroup_->setTitle(tr("Request Data"));
    dataRadixLabel_->setText(tr("Data Radix:"));
    dataRadixDecBtn_->setText(tr("DEC"));
    dataRadixHexBtn_->setText(tr("HEX"));
    dataRadixBinBtn_->setText(tr("BIN"));
    if (readPlaceholderLabel_) {
        readPlaceholderLabel_->setText(tr("Read operation requires no request payload"));
    }
    fc05OnBtn_->setText(tr("ON (0xFF00)"));
    fc05OffBtn_->setText(tr("OFF (0x0000)"));
    updateDataRadixHints();
    rawGroup_->setTitle(tr("Raw Frame Dispatch (HEX)"));
    rawTextEdit_->setPlaceholderText(tr("Enter HEX bytes (e.g. 01 03 00 00 00 0A)"));
    addMbapBtn_->setText(tr("+ MBAP"));
    addCrcBtn_->setText(tr("+ CRC16"));
    addLrcBtn_->setText(tr("+ LRC"));
    cleanHexBtn_->setText(tr("Format"));
    rawExecuteBtn_->setText(tr("Send Raw Frame"));
}

void ModbusWorkbenchWidget::retranslateExecutionUi() {
    executionGroup_->setTitle(tr("Execution & Polling").replace(QLatin1Char('&'), QStringLiteral("&&")));
    updateExecuteButtonAppearance();
    pollCheckBox_->setText(tr("Periodic Polling"));
    pollIntervalLabel_->setText(tr("Interval:"));
    if (writePollWarningBadge_) {
        writePollWarningBadge_->setText(tr("[!] Write Polling Active"));
    }
    updateGuardAppearance();
}

void ModbusWorkbenchWidget::updateExecuteButtonAppearance() {
    if (!executeBtn_) {
        return;
    }
    if (isPolling_) {
        executeBtn_->setText(tr("Stop Polling (Esc)"));
        executeBtn_->setShortcut(QKeySequence(Qt::Key_Escape));
        executeBtn_->setStyleSheet(QString());
    } else if (::modbus::base::function_traits::isWriteOperation(functionCode())) {
        executeBtn_->setText(tr("Write (F5)"));
        executeBtn_->setShortcut(QKeySequence(Qt::Key_F5));
        executeBtn_->setStyleSheet(QStringLiteral("border: 1px solid #e65100; font-weight: bold;"));
    } else {
        executeBtn_->setText(tr("Execute (F5)"));
        executeBtn_->setShortcut(QKeySequence(Qt::Key_F5));
        executeBtn_->setStyleSheet(QString());
    }
}

void ModbusWorkbenchWidget::updateGuardAppearance() {
    if (guardHintLabel_) {
        const bool showHint = !guardState_.executeEnabled && !guardState_.tooltipMessage.isEmpty();
        if (showHint) {
            guardHintLabel_->setText(guardState_.tooltipMessage);
            guardHintLabel_->show();
        } else {
            guardHintLabel_->hide();
        }
    }
    const QString tip = (!guardState_.executeEnabled) ? guardState_.tooltipMessage : QString();
    if (executeBtn_) {
        executeBtn_->setToolTip(tip);
    }
    if (rawExecuteBtn_) {
        rawExecuteBtn_->setToolTip((!guardState_.rawSendEnabled) ? guardState_.tooltipMessage : QString());
    }
    if (pollCheckBox_ && slaveId() != 0) {
        pollCheckBox_->setToolTip((!guardState_.pollingEnabled) ? guardState_.tooltipMessage : QString());
    }
}

void ModbusWorkbenchWidget::retranslateUi() {
    retranslateAddressingUi();
    retranslateWriteAndRawUi();
    retranslateExecutionUi();
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
