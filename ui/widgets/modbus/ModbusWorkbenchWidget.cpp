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
#include <QShortcut>
#include <QSpinBox>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QResizeEvent>

#include "Config.h"
#include "common/ModbusDataHelper.h"
#include "infra/config/ISettingsService.h"
#include "core/common/SettingsKeys.h"
#include "modbus/base/ModbusAduBuilder.h"
#include "modbus/base/ModbusCrc.h"
#include "modbus/base/ModbusLrc.h"

namespace ui::widgets::modbus {

namespace {

class WorkbenchScrollArea : public QScrollArea {
public:
    explicit WorkbenchScrollArea(QWidget* parent = nullptr) : QScrollArea(parent) {}
    [[nodiscard]] QSize sizeHint() const override { return widget() ? widget()->sizeHint() : QScrollArea::sizeHint(); }
    [[nodiscard]] QSize minimumSizeHint() const override { return QSize(0, 0); }
};

class DynamicStackedLayout : public QStackedLayout {
public:
    using QStackedLayout::QStackedLayout;
    [[nodiscard]] bool hasHeightForWidth() const override {
        return currentWidget() && currentWidget()->hasHeightForWidth();
    }
    [[nodiscard]] int heightForWidth(int width) const override {
        return (currentWidget() && currentWidget()->hasHeightForWidth())
            ? currentWidget()->heightForWidth(width)
            : (currentWidget() ? currentWidget()->sizeHint().height() : 0);
    }
    [[nodiscard]] QSize sizeHint() const override {
        return currentWidget() ? currentWidget()->sizeHint() : QStackedLayout::sizeHint();
    }
    [[nodiscard]] QSize minimumSize() const override {
        return currentWidget() ? currentWidget()->minimumSizeHint() : QStackedLayout::minimumSize();
    }
};

class FramePreviewEdit : public QPlainTextEdit {
public:
    using QPlainTextEdit::QPlainTextEdit;
    [[nodiscard]] QSize sizeHint() const override { return QSize(100, 28); }
    [[nodiscard]] QSize minimumSizeHint() const override { return QSize(0, 24); }
};

} // namespace

ModbusWorkbenchWidget::ModbusWorkbenchWidget(infra::config::ISettingsService* settingsService,
                                             QWidget* parent)
    : QWidget(parent)
    , settingsService_(settingsService) {
    setMinimumWidth(280);
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
    isPolling_ = guards.pollingActive;
    if (pollCheckBox_->isChecked() != isPolling_) {
        const QSignalBlocker blocker(pollCheckBox_);
        pollCheckBox_->setChecked(isPolling_);
    }
    if (!isPolling_) {
        writePollWarningBadge_->hide();
        userConfirmedWritePoll_ = false;
    }
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

QByteArray ModbusWorkbenchWidget::currentWriteData(uint8_t fc, uint16_t qty) const {
    if (fc == 0x05) {
        return fc05IsOn_ ? QByteArray::fromHex("FF00") : QByteArray::fromHex("0000");
    }
    if (fc == 0x06) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc06ValueEdit_->text(), dataRadix_, 1);
        return res.isOk() ? res.value() : QByteArray();
    }
    if (fc == 0x0F) {
        auto res = ::ui::common::data_helper::packCoilBits(fc0fBitsEdit_->text(), qty);
        return res.isOk() ? res.value() : QByteArray();
    }
    if (fc == 0x10) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc10WordsEdit_->text(), dataRadix_, qty);
        return res.isOk() ? res.value() : QByteArray();
    }
    return {};
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
    spec.writeData = currentWriteData(spec.functionCode, spec.quantity);
    spec.intervalMs = pollIntervalSpin_->value();
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
    if (const int idx = fcCombo_->findData(fc); idx >= 0) fcCombo_->setCurrentIndex(idx);
}

void ModbusWorkbenchWidget::setStartAddress(uint16_t addr) {
    pduAddress_ = addr;
    stopPollingOnParameterChange(tr("Polling automatically stopped due to address change"));
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::setQuantity(uint16_t qty) { quantitySpin_->setValue(qty); }

void ModbusWorkbenchWidget::setRadix(int radix) {
    (radix == 16 ? radixHexBtn_ : radixDecBtn_)->setChecked(true);
    onRadixChanged(radix == 16 ? 16 : 10);
}

void ModbusWorkbenchWidget::setDataRadix(int radix) {
    if ((radix != 10 && radix != 16 && radix != 2) || functionCode() == 0x05 || functionCode() == 0x0F) return;
    (radix == 16 ? dataRadixHexBtn_ : (radix == 2 ? dataRadixBinBtn_ : dataRadixDecBtn_))->setChecked(true);
    onDataRadixChanged(radix);
}

void ModbusWorkbenchWidget::setPollingIntervalMs(int ms) { pollIntervalSpin_->setValue(ms); }

void ModbusWorkbenchWidget::setFc05Value(bool on) {
    fc05IsOn_ = on;
    (on ? fc05OnBtn_ : fc05OffBtn_)->setChecked(true);
    validateInputs();
}

void ModbusWorkbenchWidget::setWritePayload(const QString& text) {
    writePayloadDirty_ = true;
    const uint8_t fc = functionCode();
    if (fc == 0x06) fc06ValueEdit_->setText(text);
    else if (fc == 0x0F) fc0fBitsEdit_->setText(text);
    else if (fc == 0x10) fc10WordsEdit_->setText(text);
    validateInputs();
}

void ModbusWorkbenchWidget::setRawText(const QString& text) {
    rawTextEdit_->setPlainText(text);
    validateInputs();
}

void ModbusWorkbenchWidget::setPollingChecked(bool checked) {
    if (checked && slaveId() == 0) { stopPollingInternal(false); return; }
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
    outerLayout_ = new QVBoxLayout(this);
    outerLayout_->setObjectName(QStringLiteral("workbenchRootLayout"));
    outerLayout_->setContentsMargins(4, 4, 4, 4);
    outerLayout_->setSpacing(6);
    setupModeToggle(this, outerLayout_);

    scrollArea_ = new WorkbenchScrollArea(this);
    scrollArea_->setObjectName(QStringLiteral("workbenchScrollArea"));
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* container = new QWidget(scrollArea_);
    container->setObjectName(QStringLiteral("workbenchContainer"));
    container->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(6);

    auto* stackWidget = new QWidget(container);
    stackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainStackedLayout_ = new DynamicStackedLayout(stackWidget);
    mainStackedLayout_->addWidget(createStandardWidget());
    mainStackedLayout_->addWidget(createRawWidget());
    containerLayout->addWidget(stackWidget, 1);
    scrollArea_->setWidget(container);
    outerLayout_->addWidget(scrollArea_, 1);
    setupExecutionSection(this, outerLayout_);

    stopPollShortcut_ = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    stopPollShortcut_->setObjectName(QStringLiteral("workbenchStopPollShortcut"));
    stopPollShortcut_->setEnabled(false);
    connect(stopPollShortcut_, &QShortcut::activated, this, [this]() { if (isPolling_) onExecuteClicked(); });
    updateRequestDataState(functionCode());
    retranslateUi();
}

QWidget* ModbusWorkbenchWidget::createStandardWidget() {
    auto* w = new QWidget(this);
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(8);

    setupAddressingSection(w, l);
    setupWriteSection(w, l);
    return w;
}

void ModbusWorkbenchWidget::setupAddressingSection(QWidget* parent, QVBoxLayout* layout) {
    addressingGroup_ = new QGroupBox(parent);
    addressingGroup_->setObjectName(QStringLiteral("workbenchAddressingGroup"));
    addressingGroup_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
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

    layout->addWidget(addressingGroup_, 0);
}

void ModbusWorkbenchWidget::setupFunctionCodeSelector(QGroupBox* group, QVBoxLayout* gl) {
    fcLabel_ = new QLabel(group);
    gl->addWidget(fcLabel_);
    fcCombo_ = new QComboBox(group);
    fcCombo_->setObjectName(QStringLiteral("workbenchFcCombo"));
    fcCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    fcCombo_->setMinimumContentsLength(12);
    static constexpr uint8_t kFcCodes[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0F, 0x10};
    for (uint8_t fc : kFcCodes) {
        fcCombo_->addItem(QString(), fc);
    }
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
    radixLayout->setSpacing(4);
    radixGroup_ = new QButtonGroup(this);
    auto makeBtn = [group, this](const QString& name, int id, bool checked) {
        auto* btn = new QPushButton(group);
        btn->setObjectName(name);
        btn->setCheckable(true);
        btn->setChecked(checked);
        btn->setMinimumWidth(36);
        radixGroup_->addButton(btn, id);
        return btn;
    };
    radixDecBtn_ = makeBtn(QStringLiteral("radixDecBtn"), 10, true);
    radixHexBtn_ = makeBtn(QStringLiteral("radixHexBtn"), 16, false);
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
    writeGroup_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* wl = new QVBoxLayout(writeGroup_);
    wl->setSpacing(6);

    setupDataRadixRow(writeGroup_, wl);

    auto* writeStackWidget = new QWidget(writeGroup_);
    writeStackedLayout_ = new DynamicStackedLayout(writeStackWidget);
    setupWriteStackPages(writeStackWidget);
    wl->addWidget(writeStackWidget);

    writeWarningLabel_ = new QLabel(writeGroup_);
    writeWarningLabel_->setObjectName(QStringLiteral("workbenchWriteWarning"));
    writeWarningLabel_->setStyleSheet(QStringLiteral("color: #d32f2f; font-weight: bold;"));
    writeWarningLabel_->setWordWrap(true);
    writeWarningLabel_->hide();
    wl->addWidget(writeWarningLabel_);

    framePreviewLabel_ = new QLabel(writeGroup_);
    framePreviewLabel_->setObjectName(QStringLiteral("workbenchFramePreviewLabel"));
    wl->addWidget(framePreviewLabel_);

    framePreviewEdit_ = new FramePreviewEdit(writeGroup_);
    framePreviewEdit_->setObjectName(QStringLiteral("workbenchFramePreviewEdit"));
    framePreviewEdit_->setReadOnly(true);
    framePreviewEdit_->setMinimumHeight(24);
    framePreviewEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    framePreviewEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    wl->addWidget(framePreviewEdit_, 1);

    layout->addWidget(writeGroup_, 1);
}

void ModbusWorkbenchWidget::setupDataRadixRow(QGroupBox* group, QVBoxLayout* wl) {
    auto* rowLayout = new QHBoxLayout();
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(4);
    dataRadixLabel_ = new QLabel(group);
    dataRadixGroup_ = new QButtonGroup(this);
    auto makeBtn = [group, this](const QString& name, int id, bool checked) {
        auto* btn = new QPushButton(group);
        btn->setObjectName(name);
        btn->setCheckable(true);
        btn->setChecked(checked);
        btn->setMinimumWidth(36);
        dataRadixGroup_->addButton(btn, id);
        return btn;
    };
    dataRadixDecBtn_ = makeBtn(QStringLiteral("workbenchDataRadixDecBtn"), 10, true);
    dataRadixHexBtn_ = makeBtn(QStringLiteral("workbenchDataRadixHexBtn"), 16, false);
    dataRadixBinBtn_ = makeBtn(QStringLiteral("workbenchDataRadixBinBtn"), 2, false);
    connect(dataRadixGroup_, &QButtonGroup::idClicked, this, &ModbusWorkbenchWidget::onDataRadixChanged);
    rowLayout->addWidget(dataRadixLabel_);
    rowLayout->addWidget(dataRadixDecBtn_);
    rowLayout->addWidget(dataRadixHexBtn_);
    rowLayout->addWidget(dataRadixBinBtn_);
    rowLayout->addStretch(1);
    wl->addLayout(rowLayout);
}

void ModbusWorkbenchWidget::setupReadPlaceholderPage(QWidget* stackWidget) {
    auto* page = new QWidget(stackWidget);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    readPlaceholderLabel_ = new QLabel(page);
    readPlaceholderLabel_->setObjectName(QStringLiteral("workbenchReadPlaceholder"));
    readPlaceholderLabel_->setStyleSheet(QStringLiteral("color: #757575; font-size: 11px; font-style: italic;"));
    readPlaceholderLabel_->setWordWrap(true);
    layout->addWidget(readPlaceholderLabel_);
    writeStackedLayout_->addWidget(page);
}

void ModbusWorkbenchWidget::setupFc05Page(QWidget* stackWidget) {
    auto* page = new QWidget(stackWidget);
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    fc05Group_ = new QButtonGroup(this);
    auto makeBtn = [page, this](const QString& name, int id, bool checked) {
        auto* btn = new QPushButton(page);
        btn->setObjectName(name);
        btn->setCheckable(true);
        btn->setChecked(checked);
        fc05Group_->addButton(btn, id);
        return btn;
    };
    fc05OnBtn_ = makeBtn(QStringLiteral("workbenchFc05OnBtn"), 1, true);
    fc05OffBtn_ = makeBtn(QStringLiteral("workbenchFc05OffBtn"), 0, false);
    connect(fc05Group_, &QButtonGroup::idClicked, this, [this](int id) { onFc05Toggled(id == 1); });
    layout->addWidget(fc05OnBtn_);
    layout->addWidget(fc05OffBtn_);
    writeStackedLayout_->addWidget(page);
}

void ModbusWorkbenchWidget::addWriteLinePage(QWidget* stackWidget, QLabel*& lbl, QLineEdit*& edt,
                                             const QString& objName, const QString& initText) {
    auto* page = new QWidget(stackWidget);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    lbl = new QLabel(page);
    lbl->setWordWrap(true);
    layout->addWidget(lbl);
    edt = new QLineEdit(page);
    edt->setObjectName(objName);
    if (!initText.isEmpty()) {
        edt->setText(initText);
    }
    connect(edt, &QLineEdit::textEdited, this, [this]() {
        stopPollingOnParameterChange(tr("Polling automatically stopped due to payload change"));
    });
    connect(edt, &QLineEdit::textChanged, this, [this]() {
        writePayloadDirty_ = true;
        validateInputs();
    });
    layout->addWidget(edt);
    writeStackedLayout_->addWidget(page);
}

void ModbusWorkbenchWidget::setupWriteStackPages(QWidget* stackWidget) {
    setupReadPlaceholderPage(stackWidget);
    setupFc05Page(stackWidget);
    addWriteLinePage(stackWidget, fc06ValueLabel_, fc06ValueEdit_,
                     QStringLiteral("workbenchFc06ValueEdit"), QStringLiteral("0"));
    addWriteLinePage(stackWidget, fc0fHintLabel_, fc0fBitsEdit_,
                     QStringLiteral("workbenchFc0fBitsEdit"));
    addWriteLinePage(stackWidget, fc10HintLabel_, fc10WordsEdit_,
                     QStringLiteral("workbenchFc10WordsEdit"));
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
    pollIntervalSpin_->setRange(config::Polling::kMinIntervalMs, config::Polling::kMaxIntervalMs);
    pollIntervalSpin_->setSingleStep(config::Polling::kIntervalStepMs);
    pollIntervalSpin_->setValue(config::Polling::kDefaultIntervalMs);
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
    rawGroup_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* el = new QVBoxLayout(rawGroup_);

    rawTextEdit_ = new QPlainTextEdit(rawGroup_);
    rawTextEdit_->setObjectName(QStringLiteral("workbenchRawTextEdit"));
    rawTextEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rawTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(rawTextEdit_, &QPlainTextEdit::textChanged, this, &ModbusWorkbenchWidget::validateInputs);
    el->addWidget(rawTextEdit_, 1);

    setupRawButtons(rawGroup_, el);

    l->addWidget(rawGroup_, 1);
    return w;
}

void ModbusWorkbenchWidget::setupRawButtons(QGroupBox* editGroup, QVBoxLayout* el) {
    auto* btnLayout = new QHBoxLayout();
    auto makeBtn = [editGroup, btnLayout](const QString& name, auto slot, auto* receiver) {
        auto* btn = new QPushButton(editGroup);
        btn->setObjectName(name);
        connect(btn, &QPushButton::clicked, receiver, slot);
        btnLayout->addWidget(btn);
        return btn;
    };
    addMbapBtn_ = makeBtn(QStringLiteral("workbenchAddMbapBtn"), &ModbusWorkbenchWidget::onAddMbapClicked, this);
    addCrcBtn_ = makeBtn(QStringLiteral("workbenchAddCrcBtn"), &ModbusWorkbenchWidget::onAddCrcClicked, this);
    addLrcBtn_ = makeBtn(QStringLiteral("workbenchAddLrcBtn"), &ModbusWorkbenchWidget::onAddLrcClicked, this);
    cleanHexBtn_ = makeBtn(QStringLiteral("workbenchCleanHexBtn"), &ModbusWorkbenchWidget::onCleanHexClicked, this);
    el->addLayout(btnLayout);
}

void ModbusWorkbenchWidget::onModeChanged(int id) {
    isRawMode_ = (id == 1);
    mainStackedLayout_->setCurrentIndex(isRawMode_ ? 1 : 0);
    const bool showPolling = !isRawMode_;
    if (pollCheckBox_) pollCheckBox_->setVisible(showPolling);
    if (pollIntervalLabel_) pollIntervalLabel_->setVisible(showPolling);
    if (pollIntervalSpin_) pollIntervalSpin_->setVisible(showPolling);
    if (writePollWarningBadge_) {
        writePollWarningBadge_->setVisible(showPolling && isPolling_ && ::modbus::base::function_traits::isWriteOperation(functionCode()));
    }
    if (scrollArea_) {
        scrollArea_->updateGeometry();
    }
    updateExecuteButtonAppearance();
    validateInputs();
}

void ModbusWorkbenchWidget::onRadixChanged(int id) {
    radix_ = id;
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::onDataRadixChanged(int id) {
    if (id != 10 && id != 16 && id != 2) return;
    const int oldRadix = dataRadix_;
    dataRadix_ = id;
    const uint8_t fc = functionCode();
    if (fc == 0x06 || fc == 0x10) {
        registerDataRadix_ = id;
        auto* edt = (fc == 0x06) ? fc06ValueEdit_ : fc10WordsEdit_;
        const QString oldText = edt->text().trimmed();
        if (!oldText.isEmpty()) {
            edt->setText(::ui::common::data_helper::convertRadix(oldText, oldRadix, dataRadix_));
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
        (dataRadix_ == 16 ? dataRadixHexBtn_ : (dataRadix_ == 2 ? dataRadixBinBtn_ : dataRadixDecBtn_))->setChecked(true);
    }
    updateDataRadixHints();
    if (scrollArea_) scrollArea_->updateGeometry();
}

void ModbusWorkbenchWidget::updateDataRadixHints() {
    const uint16_t qty = quantity();
    if (fc0fHintLabel_) fc0fHintLabel_->setText(tr("Coil Bits (LSB-first: left = start address, %1 bits):").arg(qty));
    if (fc10HintLabel_) fc10HintLabel_->setText(tr("Register Values (required: %1 words):").arg(qty));
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

    if (isPolling_) {
        const bool isWrite = ::modbus::base::function_traits::isWriteOperation(fc);
        stopPollingOnParameterChange(
            isWrite ? QString() : tr("Polling automatically stopped due to function code change"));
        if (isWrite) {
            emit logMessageRequested(tr("Polling automatically stopped due to write operation switch"), true);
        }
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

void ModbusWorkbenchWidget::onSlaveIdChanged(int /*val*/) {
    stopPollingOnParameterChange(tr("Polling automatically stopped due to Slave ID change"));
    validateInputs();
}

void ModbusWorkbenchWidget::onAddressEdited() {
    if (auto res = ::modbus::address::toPduAddress(addressEdit_->text(), addressBase_); res.isValid) {
        pduAddress_ = res.pduAddress;
    }
    stopPollingOnParameterChange(tr("Polling automatically stopped due to address change"));
    validateInputs();
}

void ModbusWorkbenchWidget::onQuantityChanged(int /*val*/) {
    stopPollingOnParameterChange(tr("Polling automatically stopped due to quantity change"));
    validateInputs();
}

void ModbusWorkbenchWidget::onFc05Toggled(bool on) {
    fc05IsOn_ = on;
    stopPollingOnParameterChange(tr("Polling automatically stopped due to payload change"));
    validateInputs();
}

void ModbusWorkbenchWidget::onExecuteClicked() {
    if (isPolling_) { onPollCheckBoxToggled(false); return; }
    writePayloadDirty_ = true;
    validateInputs();
    if (executeBtn_->isEnabled()) emit executeRequested(currentSpec());
}

void ModbusWorkbenchWidget::onPollCheckBoxToggled(bool checked) {
    if (!checked) { stopPollingInternal(true); return; }
    if (slaveId() == 0) { stopPollingInternal(false); return; }
    writePayloadDirty_ = true;
    if (!validateInputs()) { stopPollingInternal(false); return; }

    const uint8_t fc = functionCode();
    if (::modbus::base::function_traits::isWriteOperation(fc)) {
        if (!confirmWritePoll(fc)) { stopPollingInternal(false); return; }
        userConfirmedWritePoll_ = true;
        writePollWarningBadge_->show();
    } else {
        userConfirmedWritePoll_ = false;
        writePollWarningBadge_->hide();
    }
    emit pollToggleRequested(true, currentSpec(), userConfirmedWritePoll_);
}

void ModbusWorkbenchWidget::stopPollingInternal(bool emitSignal) {
    isPolling_ = false;
    userConfirmedWritePoll_ = false;
    writePollWarningBadge_->hide();
    updateExecuteButtonAppearance();
    {
        const QSignalBlocker blocker(pollCheckBox_);
        pollCheckBox_->setChecked(false);
    }
    validateInputs();
    if (emitSignal) emit pollToggleRequested(false, currentSpec(), false);
}

void ModbusWorkbenchWidget::stopPollingOnParameterChange(const QString& reason) {
    if (!isPolling_) return;
    stopPollingInternal(true);
    if (!reason.isEmpty()) emit logMessageRequested(reason, false);
}

bool ModbusWorkbenchWidget::confirmWritePoll(uint8_t fc) {
    if (confirmWritePollHandler_) return confirmWritePollHandler_(fc);
    return QMessageBox::warning(this, tr("High-Risk Operation Warning"),
        tr("Warning: The current function code is a write operation [FC 0x%1]. "
           "Periodic polling will continuously write physical data to the slave, "
           "which may cause frequent slave state changes or hardware damage. "
           "Are you sure you want to start write polling?")
            .arg(fc, 2, 16, QLatin1Char('0')),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

void ModbusWorkbenchWidget::onPollIntervalChanged(int val) { emit pollIntervalChanged(val); }

void ModbusWorkbenchWidget::onAddMbapClicked() {
    const QByteArray pdu = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
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
        if (addressPreviewLabel_) addressPreviewLabel_->setText(tr("PDU: --"));
        return;
    }
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

void ModbusWorkbenchWidget::validateWriteData(bool& valid, QString& errorMsg) {
    const uint8_t fc = functionCode();
    const uint16_t qty = quantity();
    if (fc == 0x06) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc06ValueEdit_->text(), dataRadix_, 1);
        if (res.isErr()) { valid = false; errorMsg = res.error(); }
    } else if (fc == 0x0F) {
        auto res = ::ui::common::data_helper::packCoilBits(fc0fBitsEdit_->text(), qty);
        if (res.isErr()) { valid = false; errorMsg = res.error(); }
    } else if (fc == 0x10) {
        auto res = ::ui::common::data_helper::parseRegisterWords(fc10WordsEdit_->text(), dataRadix_, qty);
        if (res.isErr()) { valid = false; errorMsg = res.error(); }
    }
}

bool ModbusWorkbenchWidget::validateInputs() {
    if (isRawMode_) {
        const QByteArray raw = ::ui::common::data_helper::parseHex(rawTextEdit_->toPlainText());
        const bool rawValid = !raw.isEmpty();
        if (executeBtn_) executeBtn_->setEnabled(isPolling_ || (rawValid && guardState_.rawSendEnabled));
        lastValidationHint_ = rawValid ? QString() : tr("Raw frame hex data is required.");
        updateGuardAppearance();
        return rawValid;
    }

    bool addrValid = true;
    QString addrError;
    const uint8_t fc = functionCode();
    const uint8_t sid = slaveId();

    if (sid == 0 && ::modbus::base::function_traits::isReadOperation(fc)) {
        addrValid = false;
        addrError = tr("Broadcast read (Slave ID 0) is forbidden per IEC 61158");
    } else {
        validateAddressRange(addrValid, addrError);
    }

    bool writeValid = true;
    QString writeError;
    if (::modbus::base::function_traits::isWriteOperation(fc)) {
        validateWriteData(writeValid, writeError);
    }

    updateDataRadixHints();
    updateWarningAndActionState(addrValid, addrError, writeValid, writeError, sid);
    updateFramePreview();
    return addrValid && writeValid;
}

void ModbusWorkbenchWidget::updateWarningAndActionState(
    bool addrValid, const QString& addrError, bool writeValid, const QString& writeError, uint8_t sid) {
    addressWarningLabel_->setVisible(!addrValid);
    if (!addrValid) addressWarningLabel_->setText(addrError);
    addressEdit_->setStyleSheet(!addrValid ? QStringLiteral("border: 1px solid #d32f2f;") : QString());

    const bool showWriteErr = !writeValid && writePayloadDirty_;
    writeWarningLabel_->setVisible(showWriteErr);
    if (showWriteErr) writeWarningLabel_->setText(writeError);

    lastValidationHint_ = !addrValid ? addrError : (!writeValid ? writeError : QString());
    const bool overallValid = addrValid && writeValid;
    executeBtn_->setEnabled(isPolling_ || (overallValid && guardState_.executeEnabled));

    if (sid == 0) {
        pollCheckBox_->setEnabled(false);
        if (isPolling_) {
            stopPollingInternal(true);
        } else {
            const QSignalBlocker blocker(pollCheckBox_);
            pollCheckBox_->setChecked(false);
        }
        pollCheckBox_->setToolTip(tr("Broadcast (Slave ID 0) does not support periodic polling"));
    } else {
        pollCheckBox_->setEnabled(overallValid && guardState_.pollingEnabled);
        pollCheckBox_->setToolTip(!guardState_.pollingEnabled ? guardState_.tooltipMessage : QString());
    }
    updateGuardAppearance();
}

void ModbusWorkbenchWidget::updateFramePreview() {
    if (!framePreviewEdit_ || isRawMode_) {
        return;
    }
    const uint8_t fc = functionCode();
    const uint8_t sid = slaveId();
    const uint16_t qty = quantity();
    const QByteArray writeData = currentWriteData(fc, qty);

    QString error;
    const QByteArray adu = ::modbus::base::buildRequestFramePreview(
        protocolType_, sid, fc, pduAddress_, qty, writeData, &error);

    if (!adu.isEmpty()) {
        framePreviewEdit_->setPlainText(protocolType_ == ::modbus::parser::ProtocolType::Ascii
            ? QString::fromUtf8(adu.trimmed()) : adu.toHex(' ').toUpper());
    } else {
        framePreviewEdit_->setPlainText(error.isEmpty() ? QStringLiteral("--") : tr("[!] %1").arg(error));
    }
}

void ModbusWorkbenchWidget::loadSettings() {
    if (!settingsService_) { updateAddressDisplay(); validateInputs(); return; }
    using namespace core::common::settings_keys;
    auto* s = settingsService_;
    auto apply = [s](const QString& k, auto setter) { if (s->contains(k)) setter(s->value(k)); };

    uint8_t fc = 0x03;
    if (s->contains(kModbusWorkbenchFunctionCode)) {
        const auto v = static_cast<uint8_t>(s->value(kModbusWorkbenchFunctionCode).toUInt());
        if (!::modbus::base::function_traits::isWriteOperation(v)) fc = v;
    }
    setFunctionCode(fc);
    if (s->contains(kModbusWorkbenchRadix)) {
        radix_ = (s->value(kModbusWorkbenchRadix).toInt() == 16) ? 16 : 10;
        (radix_ == 16 ? radixHexBtn_ : radixDecBtn_)->setChecked(true);
    }
    registerDataRadix_ = s->contains(kModbusWorkbenchDataRadix) ? s->value(kModbusWorkbenchDataRadix).toInt() : radix_;
    if (registerDataRadix_ != 16 && registerDataRadix_ != 2) registerDataRadix_ = 10;
    dataRadix_ = registerDataRadix_;

    apply(kModbusWorkbenchSlaveId, [this](const QVariant& v) { slaveIdSpin_->setValue(v.toInt()); });
    apply(kModbusWorkbenchStartAddress, [this](const QVariant& v) { pduAddress_ = static_cast<uint16_t>(v.toUInt()); });
    apply(kModbusWorkbenchQuantity, [this](const QVariant& v) { quantitySpin_->setValue(v.toInt()); });
    apply(kModbusWorkbenchPollIntervalMs, [this](const QVariant& v) { pollIntervalSpin_->setValue(v.toInt()); });
    apply(kModbusWorkbenchFc05Value, [this](const QVariant& v) { setFc05Value(v.toBool()); });
    apply(kModbusWorkbenchFc06Value, [this](const QVariant& v) { fc06ValueEdit_->setText(v.toString()); });
    apply(kModbusWorkbenchFc0fValue, [this](const QVariant& v) { fc0fBitsEdit_->setText(v.toString()); });
    apply(kModbusWorkbenchFc10Value, [this](const QVariant& v) { fc10WordsEdit_->setText(v.toString()); });
    apply(kModbusWorkbenchRawText, [this](const QVariant& v) { rawTextEdit_->setPlainText(v.toString()); });
    apply(kModbusWorkbenchMode, [this](const QVariant& v) { setMode(v.toInt()); });
    updateAddressDisplay();
    validateInputs();
}

void ModbusWorkbenchWidget::saveSettings() {
    if (!settingsService_) return;
    using namespace core::common::settings_keys;
    auto* s = settingsService_;
    s->setValue(kModbusWorkbenchMode, isRawMode_ ? 1 : 0);
    s->setValue(kModbusWorkbenchSlaveId, slaveIdSpin_->value());
    s->setValue(kModbusWorkbenchFunctionCode, functionCode());
    s->setValue(kModbusWorkbenchStartAddress, pduAddress_);
    s->setValue(kModbusWorkbenchQuantity, quantitySpin_->value());
    s->setValue(kModbusWorkbenchRadix, radix_);
    s->setValue(kModbusWorkbenchDataRadix, registerDataRadix_);
    s->setValue(kModbusWorkbenchPollIntervalMs, pollIntervalSpin_->value());
    s->setValue(kModbusWorkbenchFc05Value, fc05IsOn_);
    s->setValue(kModbusWorkbenchFc06Value, fc06ValueEdit_->text());
    s->setValue(kModbusWorkbenchFc0fValue, fc0fBitsEdit_->text());
    s->setValue(kModbusWorkbenchFc10Value, fc10WordsEdit_->text());
    s->setValue(kModbusWorkbenchRawText, rawTextEdit_->toPlainText());
}

void ModbusWorkbenchWidget::retranslateAddressingUi() {
    standardModeBtn_->setText(tr("Standard"));
    rawModeBtn_->setText(tr("Raw Frame"));
    addressingGroup_->setTitle(tr("Addressing & Parameters").replace(QLatin1Char('&'), QStringLiteral("&&")));
    radixLabel_->setText(tr("Address Radix:"));
    radixDecBtn_->setText(tr("DEC"));
    radixDecBtn_->setToolTip(tr("Decimal (DEC)"));
    radixHexBtn_->setText(tr("HEX"));
    radixHexBtn_->setToolTip(tr("Hexadecimal (HEX)"));
    if (addrBaseLabel_) addrBaseLabel_->setText(tr("Address Base:"));
    if (addrBaseCombo_) {
        const QSignalBlocker blocker(addrBaseCombo_);
        addrBaseCombo_->setItemText(0, tr("0-Based"));
        addrBaseCombo_->setItemText(1, tr("1-Based"));
    }
    fcLabel_->setText(tr("Function Code:"));
    const int currentFcIdx = fcCombo_->currentIndex();
    {
        const QSignalBlocker blocker(fcCombo_);
        static constexpr const char* kFcCanonicalNames[] = {
            QT_TR_NOOP("0x01 Read Coils"), QT_TR_NOOP("0x02 Read Discrete Inputs"),
            QT_TR_NOOP("0x03 Read Holding Registers"), QT_TR_NOOP("0x04 Read Input Registers"),
            QT_TR_NOOP("0x05 Write Single Coil"), QT_TR_NOOP("0x06 Write Single Register"),
            QT_TR_NOOP("0x0F Write Multiple Coils"), QT_TR_NOOP("0x10 Write Multiple Registers")
        };
        for (int i = 0; i < 8; ++i) {
            fcCombo_->setItemText(i, tr(kFcCanonicalNames[i]));
            fcCombo_->setItemData(i, QString::fromLatin1(kFcCanonicalNames[i]), Qt::ToolTipRole);
        }
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
    dataRadixDecBtn_->setToolTip(tr("Decimal (DEC)"));
    dataRadixHexBtn_->setText(tr("HEX"));
    dataRadixHexBtn_->setToolTip(tr("Hexadecimal (HEX)"));
    dataRadixBinBtn_->setText(tr("BIN"));
    dataRadixBinBtn_->setToolTip(tr("Binary (BIN)"));
    if (readPlaceholderLabel_) readPlaceholderLabel_->setText(tr("Read operation requires no request payload"));
    if (framePreviewLabel_) framePreviewLabel_->setText(tr("Frame Preview (HEX):"));
    fc05OnBtn_->setText(tr("ON (0xFF00)"));
    fc05OffBtn_->setText(tr("OFF (0x0000)"));
    updateDataRadixHints();
    rawGroup_->setTitle(tr("Raw Frame Dispatch (HEX)"));
    rawTextEdit_->setPlaceholderText(tr("Enter HEX bytes (e.g. 01 03 00 00 00 0A)"));
    addMbapBtn_->setText(tr("+ MBAP"));
    addCrcBtn_->setText(tr("+ CRC16"));
    addLrcBtn_->setText(tr("+ LRC"));
    cleanHexBtn_->setText(tr("Format"));
}

void ModbusWorkbenchWidget::retranslateExecutionUi() {
    executionGroup_->setTitle(tr("Execution & Polling").replace(QLatin1Char('&'), QStringLiteral("&&")));
    updateExecuteButtonAppearance();
    pollCheckBox_->setText(tr("Periodic Polling"));
    pollIntervalLabel_->setText(tr("Interval:"));
    if (writePollWarningBadge_) writePollWarningBadge_->setText(tr("[!] Write Polling Active"));
    updateGuardAppearance();
}

void ModbusWorkbenchWidget::updateExecuteButtonAppearance() {
    if (!executeBtn_) return;
    if (stopPollShortcut_) stopPollShortcut_->setEnabled(isPolling_ && isRawMode_);
    if (isPolling_) {
        executeBtn_->setText(tr("Stop Polling (Esc)"));
        executeBtn_->setShortcut(QKeySequence(Qt::Key_Escape));
        executeBtn_->setStyleSheet(QString());
    } else if (isRawMode_) {
        executeBtn_->setText(tr("Send Raw Frame (F5)"));
        executeBtn_->setShortcut(QKeySequence(Qt::Key_F5));
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
    const bool showGuardMsg = isRawMode_
        ? (!guardState_.rawSendEnabled && !guardState_.tooltipMessage.isEmpty())
        : (!guardState_.executeEnabled && !guardState_.tooltipMessage.isEmpty());
    const bool showValidationMsg = !showGuardMsg
        && (isRawMode_ ? guardState_.rawSendEnabled : guardState_.executeEnabled)
        && !isPolling_ && !lastValidationHint_.isEmpty();
    const QString tip = showGuardMsg ? guardState_.tooltipMessage : (showValidationMsg ? lastValidationHint_ : QString());

    if (guardHintLabel_) {
        guardHintLabel_->setVisible(!tip.isEmpty());
        if (!tip.isEmpty()) guardHintLabel_->setText(tip);
    }
    if (executeBtn_) executeBtn_->setToolTip(tip);
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

void ModbusWorkbenchWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (scrollArea_) scrollArea_->updateGeometry();
}

} // namespace ui::widgets::modbus
