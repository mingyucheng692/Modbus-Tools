#include "ControlWidget.h"
#include "AppConstants.h"
#include "../common/ISettingsService.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimer>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

ControlWidget::ControlWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
    
    pollTimer_ = new QTimer(this);
    connect(pollTimer_, &QTimer::timeout, this, &ControlWidget::onTimer);
    
    // Connect Enable Checkbox
    connect(enablePollCheck_, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            pollTimer_->start(intervalSpin_->value());
        } else {
            pollTimer_->stop();
        }
    });

    // Connect Interval Change
    connect(intervalSpin_, &QSpinBox::valueChanged, [this](int val) {
        if (pollTimer_->isActive()) {
            pollTimer_->setInterval(val);
        }
    });

    connect(intervalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &ControlWidget::saveSettings);
    connect(fcCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &ControlWidget::saveSettings);
    connect(addrSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &ControlWidget::saveSettings);
    connect(qtySpin_, qOverload<int>(&QSpinBox::valueChanged), this, &ControlWidget::saveSettings);
    connect(enablePollCheck_, &QCheckBox::toggled, this, &ControlWidget::saveSettings);
}

ControlWidget::~ControlWidget() = default;

void ControlWidget::recordTx() {
    txCount_++;
    updateStatsLabel();
}

void ControlWidget::recordRx(int rttMs) {
    rxCount_++;
    if (rttMs >= 0) {
        lastRtt_ = rttMs;
    }
    updateStatsLabel();
}

void ControlWidget::recordError() {
    errorCount_++;
    updateStatsLabel();
}

void ControlWidget::resetStats() {
    txCount_ = 0;
    rxCount_ = 0;
    errorCount_ = 0;
    lastRtt_ = 0;
    updateStatsLabel();
}

void ControlWidget::onTimer() {
    int fcIndex = fcCombo_->currentIndex();
    uint8_t fc = 0x03; // Default
    
    // Map index to FC (01, 02, 03, 04)
    switch(fcIndex) {
        case 0: fc = 0x01; break; // Coils
        case 1: fc = 0x02; break; // Discrete Inputs
        case 2: fc = 0x03; break; // Holding Registers
        case 3: fc = 0x04; break; // Input Registers
        default: fc = 0x03; break;
    }
    
    emit pollRequested(fc, addrSpin_->value(), qtySpin_->value());
}

void ControlWidget::updateStatsLabel() {
    statsLabel_->setText(QStringLiteral("TX: %1 | RX: %2 | Err: %3 | RTT: %4 ms")
                             .arg(txCount_)
                             .arg(rxCount_)
                             .arg(errorCount_)
                             .arg(lastRtt_));
}

void ControlWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    loadSettings();
}

void ControlWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(intervalSpin_);
    QSignalBlocker b2(fcCombo_);
    QSignalBlocker b3(addrSpin_);
    QSignalBlocker b4(qtySpin_);

    const QString intervalKey = settingsGroup_ + "/intervalMs";
    const QString fcKey = settingsGroup_ + "/fcIndex";
    const QString addrKey = settingsGroup_ + "/addr";
    const QString qtyKey = settingsGroup_ + "/qty";

    const bool enablePoll = settingsService_->value(settingsGroup_ + "/enablePoll").toBool();
    const int interval = settingsService_->value(intervalKey).toInt();
    const int fcIndex = settingsService_->value(fcKey).toInt();
    const int addr = settingsService_->value(addrKey).toInt();
    const int qty = settingsService_->value(qtyKey).toInt();

    enablePollCheck_->setChecked(enablePoll);
    intervalSpin_->setValue(interval);
    if (fcIndex >= 0 && fcIndex < fcCombo_->count()) {
        fcCombo_->setCurrentIndex(fcIndex);
    }
    addrSpin_->setValue(addr);
    qtySpin_->setValue(qty);

    if (enablePollCheck_->isChecked()) {
        pollTimer_->start(intervalSpin_->value());
    } else {
        pollTimer_->stop();
    }
}

void ControlWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/enablePoll", enablePollCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/intervalMs", intervalSpin_->value());
    settingsService_->setValue(settingsGroup_ + "/fcIndex", fcCombo_->currentIndex());
    settingsService_->setValue(settingsGroup_ + "/addr", addrSpin_->value());
    settingsService_->setValue(settingsGroup_ + "/qty", qtySpin_->value());
}

void ControlWidget::setupUi() {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 4, 2, 4);
    layout->setSpacing(2);
    
    // Poll Enable
    enablePollCheck_ = new QCheckBox(this);
    layout->addWidget(enablePollCheck_);
    
    // Interval
    intervalLabel_ = new QLabel(this);
    layout->addWidget(intervalLabel_);
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(app::constants::Values::Polling::kMinIntervalMs,
                            app::constants::Values::Polling::kMaxIntervalMs);
    intervalSpin_->setValue(app::constants::Values::Polling::kDefaultIntervalMs);
    intervalSpin_->setSingleStep(app::constants::Values::Polling::kIntervalStepMs);
    intervalSpin_->setFixedWidth(78);
    layout->addWidget(intervalSpin_);
    
    // Function Code
    fcLabel_ = new QLabel(this);
    layout->addWidget(fcLabel_);
    fcCombo_ = new QComboBox(this);
    fcCombo_->addItems({"", "", "", ""});
    fcCombo_->setCurrentIndex(app::constants::Values::Modbus::kDefaultControlFunctionIndex);
    fcCombo_->setMinimumContentsLength(11);
    fcCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    layout->addWidget(fcCombo_);
    
    // Address
    addrLabel_ = new QLabel(this);
    layout->addWidget(addrLabel_);
    addrSpin_ = new QSpinBox(this);
    addrSpin_->setRange(app::constants::Values::Modbus::kMinAddress,
                        app::constants::Values::Modbus::kMaxAddress);
    addrSpin_->setValue(app::constants::Values::Modbus::kDefaultControlAddress);
    addrSpin_->setFixedWidth(68);
    layout->addWidget(addrSpin_);
    
    // Quantity
    qtyLabel_ = new QLabel(this);
    layout->addWidget(qtyLabel_);
    qtySpin_ = new QSpinBox(this);
    qtySpin_->setRange(app::constants::Values::Modbus::kMinQuantity,
                       app::constants::Values::Modbus::kMaxReadQuantity);
    qtySpin_->setValue(app::constants::Values::Modbus::kDefaultControlQuantity);
    qtySpin_->setFixedWidth(60);
    layout->addWidget(qtySpin_);
    
    layout->addStretch();
    
    // Stats
    statsLabel_ = new QLabel(this);
    statsLabel_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    layout->addWidget(statsLabel_);

    retranslateUi();
}

void ControlWidget::retranslateUi() {
    if (enablePollCheck_) {
        enablePollCheck_->setText(tr("Enable Polling"));
    }
    if (intervalLabel_) {
        intervalLabel_->setText(tr("Interval(ms):"));
    }
    if (fcLabel_) {
        fcLabel_->setText(tr("FC:"));
    }
    if (fcCombo_) {
        fcCombo_->setItemText(0, tr("01-Read Coils"));
        fcCombo_->setItemText(1, tr("02-Read Discrete"));
        fcCombo_->setItemText(2, tr("03-Read Holding"));
        fcCombo_->setItemText(3, tr("04-Read Input"));
    }
    if (addrLabel_) {
        addrLabel_->setText(tr("Addr:"));
    }
    if (qtyLabel_) {
        qtyLabel_->setText(tr("Qty:"));
    }
    updateStatsLabel();
}

void ControlWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
