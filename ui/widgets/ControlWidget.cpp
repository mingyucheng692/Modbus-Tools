/**
 * @file ControlWidget.cpp
 * @brief Implementation of ControlWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ControlWidget.h"
#include "AppConstants.h"
#include "../common/ISettingsService.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTimer>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include "modbus/base/ModbusDataHelper.h"

namespace ui::widgets {

ControlWidget::ControlWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
    
    pollTimer_ = new QTimer(this);
    connect(pollTimer_, &QTimer::timeout, this, &ControlWidget::onTimer);
    
    // Connect Enable Checkbox
    connect(enablePollCheck_, &QCheckBox::clicked, [this](bool checked) {
        if (checked) {
            bool ok = true;
            if (connectionValidator_) {
                ok = connectionValidator_();
            }
            if (!ok) {
                QSignalBlocker blocker(enablePollCheck_);
                enablePollCheck_->setChecked(false);
                return;
            }
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
    connect(addrEdit_, &QLineEdit::textChanged, this, &ControlWidget::saveSettings);
    connect(qtySpin_, qOverload<int>(&QSpinBox::valueChanged), this, &ControlWidget::saveSettings);
    connect(enablePollCheck_, &QCheckBox::toggled, this, &ControlWidget::saveSettings);
    
    connect(linkCheck_, &QCheckBox::toggled, this, &ControlWidget::linkToggled);
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
    
    bool ok = false;
    int addr = modbus::base::ModbusDataHelper::parseSmartInt(addrEdit_->text(), &ok);
    
    if (!ok || addr < app::constants::Values::Modbus::kMinAddress || addr > app::constants::Values::Modbus::kMaxAddress) {
        emit logMessageRequested(tr("Invalid Address format or range (0-65535): %1").arg(addrEdit_->text()), true);
        return;
    }

    // Address 0 Confirmation Logic
    if (addr == 0 && !skipAddrZeroWarning_) {
        // Pause timer to avoid multiple dialogs
        bool wasActive = pollTimer_->isActive();
        if (wasActive) pollTimer_->stop();

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Confirm Address"));
        msgBox.setText(tr("The polling address is set to 0. Are you sure you want to continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        QCheckBox* cb = new QCheckBox(tr("Do not show this again"), &msgBox);
        msgBox.setCheckBox(cb);

        int res = msgBox.exec();
        if (cb->isChecked()) {
            skipAddrZeroWarning_ = true;
            saveSettings();
        }

        if (res != QMessageBox::Yes) {
            // User cancelled, keep polling disabled or just skip this one
            if (enablePollCheck_) {
                QSignalBlocker blocker(enablePollCheck_);
                enablePollCheck_->setChecked(false);
            }
            return;
        }

        // Resume if it was active
        if (wasActive) pollTimer_->start();
    }
    
    emit pollRequested(fc, addr, qtySpin_->value());
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

void ControlWidget::setLinked(bool active) {
    if (linkCheck_) {
        QSignalBlocker blocker(linkCheck_);
        linkCheck_->setChecked(active);
    }
}

void ControlWidget::setPollingEnabled(bool enabled) {
    if (enablePollCheck_) {
        QSignalBlocker blocker(enablePollCheck_);
        enablePollCheck_->setChecked(enabled);
    }
    if (enabled) {
        pollTimer_->start(intervalSpin_->value());
    } else {
        pollTimer_->stop();
    }
}

void ControlWidget::setConnectionValidator(const std::function<bool()>& validator) {
    connectionValidator_ = validator;
}

int ControlWidget::pollingIntervalMs() const {
    return intervalSpin_ ? intervalSpin_->value() : app::constants::Values::Polling::kDefaultIntervalMs;
}

void ControlWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(intervalSpin_);
    QSignalBlocker b2(fcCombo_);
    QSignalBlocker b3(addrEdit_);
    QSignalBlocker b4(qtySpin_);
    QSignalBlocker b5(enablePollCheck_);
    QSignalBlocker b6(linkCheck_);

    const QString intervalKey = settingsGroup_ + "/intervalMs";
    const QString fcKey = settingsGroup_ + "/fcIndex";
    const QString addrKey = settingsGroup_ + "/addr";
    const QString addrStrKey = settingsGroup_ + "/pollAddrStr";
    const QString qtyKey = settingsGroup_ + "/qty";
    const QString skipKey = settingsGroup_ + "/skipAddrZeroPollWarning";

    auto getVal = [this](const QString& key, const QVariant& defaultVal) {
        QVariant v = settingsService_->value(key);
        return v.isValid() ? v : defaultVal;
    };

    const int interval = getVal(intervalKey, app::constants::Values::Polling::kDefaultIntervalMs).toInt();
    const int fcIndex = getVal(fcKey, app::constants::Values::Modbus::kDefaultControlFunctionIndex).toInt();
    const int qty = getVal(qtyKey, app::constants::Values::Modbus::kDefaultControlQuantity).toInt();
    
    // Load Address: Priority to pollAddrStr, fallback to addr (int)
    QString addrStr = getVal(addrStrKey, QString()).toString();
    if (addrStr.isEmpty()) {
        int oldAddr = getVal(addrKey, app::constants::Values::Modbus::kDefaultControlAddress).toInt();
        addrStr = QString::number(oldAddr);
    }

    skipAddrZeroWarning_ = getVal(skipKey, false).toBool();

    enablePollCheck_->setChecked(false); // Always OFF on startup
    linkCheck_->setChecked(false);       // Always OFF on startup
    intervalSpin_->setValue(interval);
    if (fcIndex >= 0 && fcIndex < fcCombo_->count()) {
        fcCombo_->setCurrentIndex(fcIndex);
    }
    addrEdit_->setText(addrStr);
    qtySpin_->setValue(qty);

    pollTimer_->stop();
}

void ControlWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/intervalMs", intervalSpin_->value());
    settingsService_->setValue(settingsGroup_ + "/fcIndex", fcCombo_->currentIndex());
    
    QString addrStr = addrEdit_->text();
    settingsService_->setValue(settingsGroup_ + "/pollAddrStr", addrStr);
    
    bool ok = false;
    int addr = modbus::base::ModbusDataHelper::parseSmartInt(addrStr, &ok);
    if (ok) {
        settingsService_->setValue(settingsGroup_ + "/addr", addr);
    }
    
    settingsService_->setValue(settingsGroup_ + "/qty", qtySpin_->value());
    settingsService_->setValue(settingsGroup_ + "/skipAddrZeroPollWarning", skipAddrZeroWarning_);
}

void ControlWidget::setupUi() {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 4, 2, 4);
    layout->setSpacing(2);
    
    // Poll Enable
    enablePollCheck_ = new QCheckBox(this);
    layout->addWidget(enablePollCheck_);
    
    // Link to Analyzer
    linkCheck_ = new QCheckBox(this);
    layout->addWidget(linkCheck_);
    
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
    addrEdit_ = new QLineEdit(this);
    addrEdit_->setFixedWidth(80);
    
    auto hexValidator = new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-FxXHh]*"), this);
    addrEdit_->setValidator(hexValidator);
    
    layout->addWidget(addrEdit_);
    
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
    if (linkCheck_) {
        linkCheck_->setText(tr("Link to Analyzer"));
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
    if (addrEdit_) {
        addrEdit_->setToolTip(tr("Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16)."));
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
