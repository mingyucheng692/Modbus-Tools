#include "ControlWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimer>

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
            totalRtt_ += rttMs;
        }
    }
    updateStatsLabel();
}

void ControlWidget::resetStats() {
    txCount_ = 0;
    rxCount_ = 0;
    errorCount_ = 0;
    totalRtt_ = 0;
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
    double avgRtt = (rxCount_ > 0) ? (double)totalRtt_ / rxCount_ : 0.0;
    statsLabel_->setText(QString("TX: %1 | RX: %2 | Avg RTT: %3 ms")
                        .arg(txCount_)
                        .arg(rxCount_)
                        .arg(avgRtt, 0, 'f', 1));
}

void ControlWidget::setupUi() {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Poll Enable
    enablePollCheck_ = new QCheckBox("Enable Polling", this);
    layout->addWidget(enablePollCheck_);
    
    // Interval
    layout->addWidget(new QLabel("Interval(ms):", this));
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(10, 60000);
    intervalSpin_->setValue(1000);
    intervalSpin_->setSingleStep(100);
    layout->addWidget(intervalSpin_);
    
    // Function Code
    layout->addWidget(new QLabel("FC:", this));
    fcCombo_ = new QComboBox(this);
    fcCombo_->addItems({
        "01-Read Coils", 
        "02-Read Discrete", 
        "03-Read Holding", 
        "04-Read Input"
    });
    fcCombo_->setCurrentIndex(2); // Default to 03
    layout->addWidget(fcCombo_);
    
    // Address
    layout->addWidget(new QLabel("Addr:", this));
    addrSpin_ = new QSpinBox(this);
    addrSpin_->setRange(0, 65535);
    addrSpin_->setValue(0);
    layout->addWidget(addrSpin_);
    
    // Quantity
    layout->addWidget(new QLabel("Qty:", this));
    qtySpin_ = new QSpinBox(this);
    qtySpin_->setRange(1, 125);
    qtySpin_->setValue(1);
    layout->addWidget(qtySpin_);
    
    layout->addStretch();
    
    // Stats
    statsLabel_ = new QLabel("TX: 0 | RX: 0 | Err: 0 | Avg RTT: 0.0 ms", this);
    layout->addWidget(statsLabel_);
}

} // namespace ui::widgets
