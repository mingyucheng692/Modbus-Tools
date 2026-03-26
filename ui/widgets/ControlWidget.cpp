#include "ControlWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimer>
#include <QEvent>
#include <QSettings>
#include <QApplication>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

ControlWidget::ControlWidget(QWidget *parent)
    : QWidget(parent) {
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
}

ControlWidget::~ControlWidget() = default;

void ControlWidget::updateStats(bool isTx, int rttMs, bool isError) {
    if (isError) {
        errorCount_++;
    } else if (isTx) {
        txCount_++;
    } else {
        rxCount_++;
        if (rttMs >= 0) {
            lastRtt_ = rttMs;
        }
    }
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
    statsLabel_->setText(tr("TX: %1 | RX: %2 | FAIL: %3 | RTT: %4 ms")
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
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QSignalBlocker b1(intervalSpin_);
    QSignalBlocker b2(fcCombo_);
    QSignalBlocker b3(addrSpin_);
    QSignalBlocker b4(qtySpin_);

    const QString intervalKey = settingsGroup_ + "/intervalMs";
    const QString fcKey = settingsGroup_ + "/fcIndex";
    const QString addrKey = settingsGroup_ + "/addr";
    const QString qtyKey = settingsGroup_ + "/qty";

    const int interval = settings.value(intervalKey, intervalSpin_->value()).toInt();
    const int fcIndex = settings.value(fcKey, fcCombo_->currentIndex()).toInt();
    const int addr = settings.value(addrKey, addrSpin_->value()).toInt();
    const int qty = settings.value(qtyKey, qtySpin_->value()).toInt();

    intervalSpin_->setValue(interval);
    if (fcIndex >= 0 && fcIndex < fcCombo_->count()) {
        fcCombo_->setCurrentIndex(fcIndex);
    }
    addrSpin_->setValue(addr);
    qtySpin_->setValue(qty);

    if (!settings.contains(intervalKey)) settings.setValue(intervalKey, interval);
    if (!settings.contains(fcKey)) settings.setValue(fcKey, fcCombo_->currentIndex());
    if (!settings.contains(addrKey)) settings.setValue(addrKey, addr);
    if (!settings.contains(qtyKey)) settings.setValue(qtyKey, qty);
}

void ControlWidget::saveSettings() {
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue(settingsGroup_ + "/intervalMs", intervalSpin_->value());
    settings.setValue(settingsGroup_ + "/fcIndex", fcCombo_->currentIndex());
    settings.setValue(settingsGroup_ + "/addr", addrSpin_->value());
    settings.setValue(settingsGroup_ + "/qty", qtySpin_->value());
}

void ControlWidget::setupUi() {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    // Poll Enable
    enablePollCheck_ = new QCheckBox(this);
    layout->addWidget(enablePollCheck_);
    
    // Interval
    intervalLabel_ = new QLabel(this);
    layout->addWidget(intervalLabel_);
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(10, 60000);
    intervalSpin_->setValue(1000);
    intervalSpin_->setSingleStep(100);
    intervalSpin_->setFixedWidth(84);
    layout->addWidget(intervalSpin_);
    
    // Function Code
    fcLabel_ = new QLabel(this);
    layout->addWidget(fcLabel_);
    fcCombo_ = new QComboBox(this);
    fcCombo_->addItems({"", "", "", ""});
    fcCombo_->setCurrentIndex(2); // Default to 03
    fcCombo_->setMinimumContentsLength(12);
    fcCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    layout->addWidget(fcCombo_);
    
    // Address
    addrLabel_ = new QLabel(this);
    layout->addWidget(addrLabel_);
    addrSpin_ = new QSpinBox(this);
    addrSpin_->setRange(0, 65535);
    addrSpin_->setValue(0);
    addrSpin_->setFixedWidth(72);
    layout->addWidget(addrSpin_);
    
    // Quantity
    qtyLabel_ = new QLabel(this);
    layout->addWidget(qtyLabel_);
    qtySpin_ = new QSpinBox(this);
    qtySpin_->setRange(1, 125);
    qtySpin_->setValue(1);
    qtySpin_->setFixedWidth(64);
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
