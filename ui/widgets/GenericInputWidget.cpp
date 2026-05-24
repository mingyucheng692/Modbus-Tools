/**
 * @file GenericInputWidget.cpp
 * @brief Implementation of GenericInputWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericInputWidget.h"
#include "AppConstants.h"
#include "../common/ISettingsService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDebug>
#include <QEvent>
#include <QSignalBlocker>
#include <QStringConverter>
#include <QVariant>

namespace ui::widgets {

GenericInputWidget::GenericInputWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService)
{
    setupUi();
    autoSendTimer_ = new QTimer(this);
    connect(autoSendTimer_, &QTimer::timeout, this, &GenericInputWidget::onTimerTimeout);
}

GenericInputWidget::~GenericInputWidget() = default;

void GenericInputWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. Input Area
    inputEdit_ = new QTextEdit(this);
    mainLayout->addWidget(inputEdit_);

    // 2. Control Area
    auto controlLayout = new QHBoxLayout();
    
    // Format Selection
    auto formatGroup = new QButtonGroup(this);
    hexRadio_ = new QRadioButton(this);
    asciiRadio_ = new QRadioButton(this);
    hexRadio_->setChecked(true); // Default Hex
    formatGroup->addButton(hexRadio_);
    formatGroup->addButton(asciiRadio_);
    
    controlLayout->addWidget(hexRadio_);
    controlLayout->addWidget(asciiRadio_);
    
    controlLayout->addSpacing(8);
    lineEndingCombo_ = new QComboBox(this);
    lineEndingCombo_->addItem(tr("No LF"), QVariant(0));
    lineEndingCombo_->addItem(tr("CR (\\r)"), QVariant(1));
    lineEndingCombo_->addItem(tr("LF (\\n)"), QVariant(2));
    lineEndingCombo_->addItem(tr("CRLF (\\r\\n)"), QVariant(3));
    controlLayout->addWidget(lineEndingCombo_);
    
    encodingCombo_ = new QComboBox(this);
    encodingCombo_->addItem(QStringLiteral("UTF-8"), QVariant(0));
    encodingCombo_->addItem(QStringLiteral("Latin-1"), QVariant(1));
    encodingCombo_->addItem(QStringLiteral("GBK"), QVariant(2));
    encodingCombo_->addItem(QStringLiteral("ASCII"), QVariant(3));
    controlLayout->addWidget(encodingCombo_);
    
    // Auto Send
    controlLayout->addSpacing(20);
    autoSendCheck_ = new QCheckBox(this);
    controlLayout->addWidget(autoSendCheck_);
    
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(app::constants::Values::GenericIo::kMinInputIntervalMs,
                            app::constants::Values::GenericIo::kMaxInputIntervalMs);
    intervalSpin_->setValue(app::constants::Values::GenericIo::kDefaultInputIntervalMs);
    intervalSpin_->setSuffix(" ms");
    intervalSpin_->setEnabled(false); // Enabled when checked
    controlLayout->addWidget(intervalSpin_);
    
    // Send File
    controlLayout->addStretch();
    sendFileBtn_ = new QPushButton(this);
    controlLayout->addWidget(sendFileBtn_);

    // Send History
    sendHistoryCombo_ = new QComboBox(this);
    sendHistoryCombo_->setEditable(true);
    sendHistoryCombo_->setMinimumWidth(80);
    sendHistoryCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    controlLayout->addWidget(sendHistoryCombo_);

    // Send Button
    sendBtn_ = new QPushButton(this);
    sendBtn_->setMinimumWidth(64);
    controlLayout->addWidget(sendBtn_);
    
    mainLayout->addLayout(controlLayout);

    // Connections
    connect(hexRadio_, &QRadioButton::toggled, [this](bool checked){
        if (checked) onFormatChanged();
    });
    connect(asciiRadio_, &QRadioButton::toggled, [this](bool checked){
        if (checked) onFormatChanged();
    });
    connect(hexRadio_, &QRadioButton::toggled, this, &GenericInputWidget::saveSettings);
    connect(asciiRadio_, &QRadioButton::toggled, this, &GenericInputWidget::saveSettings);
    
    connect(autoSendCheck_, &QCheckBox::toggled, this, &GenericInputWidget::onAutoSendToggled);
    connect(autoSendCheck_, &QCheckBox::toggled, this, &GenericInputWidget::saveSettings);
    connect(intervalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &GenericInputWidget::saveSettings);
    connect(sendBtn_, &QPushButton::clicked, this, &GenericInputWidget::onSendClicked);
    connect(sendFileBtn_, &QPushButton::clicked, this, &GenericInputWidget::onSendFileClicked);

    connect(lineEndingCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GenericInputWidget::saveSettings);
    connect(encodingCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GenericInputWidget::saveSettings);

    connect(sendHistoryCombo_, QOverload<int>::of(&QComboBox::activated),
            this, &GenericInputWidget::onHistoryActivated);

    retranslateUi();
}

void GenericInputWidget::onFormatChanged() {
    QString text = inputEdit_->toPlainText();
    if (text.isEmpty()) return;
    
    // Prevent recursive signal if setText triggers change? No, QRadioButton toggled triggers this.
    // Changing text doesn't trigger format change.
    
    if (hexRadio_->isChecked()) {
        // Was Ascii, now Hex
        // Convert text (treated as Ascii) to Hex
        // But wait, if the user typed "Hello", we want "48 65 6C 6C 6F"
        // If the user was in ASCII mode, text is ASCII.
        QByteArray data = text.toUtf8();
        inputEdit_->setText(data.toHex(' '));
    } else {
        // Was Hex, now Ascii
        // Convert Hex string to Ascii
        QString hex = text;
        hex.remove(' ');
        QByteArray data = QByteArray::fromHex(hex.toUtf8());
        // Check for non-printable chars?
        // QTextEdit handles them somewhat, but null bytes might be an issue.
        // For now, just set as Utf8
        inputEdit_->setText(QString::fromUtf8(data));
    }
}

void GenericInputWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    loadSettings();
}

void GenericInputWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    const QString key = settingsGroup_ + "/format";
    QString format = settingsService_->value(key).toString().toLower();
    const bool autoSend = settingsService_->value(settingsGroup_ + "/autoSend").toBool();
    const int intervalMs = settingsService_->value(settingsGroup_ + "/intervalMs").toInt();
    const int lineEnding = settingsService_->contains(settingsGroup_ + "/lineEnding")
        ? settingsService_->value(settingsGroup_ + "/lineEnding").toInt() : 0;
    const int encoding = settingsService_->contains(settingsGroup_ + "/encoding")
        ? settingsService_->value(settingsGroup_ + "/encoding").toInt() : 0;

    QSignalBlocker b1(hexRadio_);
    QSignalBlocker b2(asciiRadio_);
    QSignalBlocker b3(autoSendCheck_);
    QSignalBlocker b4(intervalSpin_);
    QSignalBlocker b5(lineEndingCombo_);
    QSignalBlocker b6(encodingCombo_);
    if (format == "ascii") {
        asciiRadio_->setChecked(true);
    } else {
        hexRadio_->setChecked(true);
    }
    autoSendCheck_->setChecked(autoSend);
    intervalSpin_->setValue(intervalMs);
    intervalSpin_->setEnabled(autoSend);
    if (autoSend) {
        autoSendTimer_->start(intervalMs);
    } else {
        autoSendTimer_->stop();
    }
    lineEndingCombo_->setCurrentIndex(qBound(0, lineEnding, 3));
    encodingCombo_->setCurrentIndex(qBound(0, encoding, 3));

    loadHistory();
}

void GenericInputWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    const QString format = hexRadio_->isChecked() ? "hex" : "ascii";
    settingsService_->setValue(settingsGroup_ + "/format", format);
    settingsService_->setValue(settingsGroup_ + "/autoSend", autoSendCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/intervalMs", intervalSpin_->value());
    settingsService_->setValue(settingsGroup_ + "/lineEnding", lineEndingCombo_->currentData().toInt());
    settingsService_->setValue(settingsGroup_ + "/encoding", encodingCombo_->currentData().toInt());
}

QByteArray GenericInputWidget::getData() const {
    QString text = inputEdit_->toPlainText();
    QByteArray data;
    if (hexRadio_->isChecked()) {
        QString hex = text;
        hex.remove(' ');
        hex.remove('\n');
        hex.remove('\r');
        data = QByteArray::fromHex(hex.toUtf8());
    } else {
        const int enc = encodingCombo_->currentData().toInt();
        switch (enc) {
        case 1: { // Latin-1
            data.reserve(text.size());
            for (const QChar& ch : text) {
                if (ch.unicode() <= 0xFF)
                    data.append(static_cast<char>(ch.unicode()));
                else
                    data.append('?');
            }
            break;
        }
        case 2: { // GBK (via GB18030 superset)
            QStringEncoder encoder(QStringLiteral("GB18030"));
            data = encoder.encode(text);
            break;
        }
        case 3: { // ASCII
            data.reserve(text.size());
            for (const QChar& ch : text) {
                data.append(ch.unicode() <= 0x7F
                    ? static_cast<char>(ch.unicode()) : '?');
            }
            break;
        }
        default: data = text.toUtf8(); break;
        }
    }

    const int ending = lineEndingCombo_->currentData().toInt();
    switch (ending) {
    case 1: data.append('\r'); break;
    case 2: data.append('\n'); break;
    case 3: data.append("\r\n"); break;
    default: break;
    }

    return data;
}

void GenericInputWidget::onSendClicked() {
    const QString text = inputEdit_->toPlainText();
    QByteArray data = getData();
    if (!data.isEmpty()) {
        addToHistory(text);
        emit sendRequested(data);
    }
}

void GenericInputWidget::onAutoSendToggled(bool checked) {
    if (checked) {
        autoSendTimer_->start(intervalSpin_->value());
    } else {
        autoSendTimer_->stop();
    }
    intervalSpin_->setEnabled(checked);
    sendBtn_->setEnabled(true);
}

void GenericInputWidget::onTimerTimeout() {
    if (!autoSendCheck_->isChecked()) {
        autoSendTimer_->stop();
        return;
    }
    
    // Restart timer if interval changed
    if (autoSendTimer_->interval() != intervalSpin_->value()) {
        autoSendTimer_->setInterval(intervalSpin_->value());
    }
    
    onSendClicked();
}

void GenericInputWidget::onSendFileClicked() {
    QString path = QFileDialog::getOpenFileName(this, tr("Select File to Send"));
    if (path.isEmpty()) return;
    emit fileSendRequested(path);
}

void GenericInputWidget::onHistoryActivated(int index) {
    if (index < 0 || !sendHistoryCombo_) return;
    const QString text = sendHistoryCombo_->itemText(index);
    if (!text.isEmpty()) {
        inputEdit_->setText(text);
    }
}

void GenericInputWidget::addToHistory(const QString& text) {
    if (text.isEmpty() || !sendHistoryCombo_) return;

    const int existingIndex = sendHistoryCombo_->findText(text);
    if (existingIndex >= 0) {
        sendHistoryCombo_->removeItem(existingIndex);
    }

    sendHistoryCombo_->insertItem(0, text);
    sendHistoryCombo_->setCurrentIndex(-1);

    while (sendHistoryCombo_->count() > 20) {
        sendHistoryCombo_->removeItem(sendHistoryCombo_->count() - 1);
    }

    saveHistory();
}

void GenericInputWidget::saveHistory() {
    if (!sendHistoryCombo_) return;
    if (settingsGroup_.isEmpty() || !settingsService_) return;

    QStringList items;
    items.reserve(sendHistoryCombo_->count());
    for (int i = 0; i < sendHistoryCombo_->count(); ++i) {
        items.append(sendHistoryCombo_->itemText(i));
    }
    settingsService_->setValue(settingsGroup_ + "/sendHistory", items);
}

void GenericInputWidget::loadHistory() {
    if (!sendHistoryCombo_) return;
    if (settingsGroup_.isEmpty() || !settingsService_) return;

    const QString key = settingsGroup_ + "/sendHistory";
    if (!settingsService_->contains(key)) return;

    const QStringList items = settingsService_->value(key).toStringList();
    sendHistoryCombo_->clear();
    for (const QString& item : items) {
        sendHistoryCombo_->addItem(item);
    }
    sendHistoryCombo_->setCurrentIndex(-1);
}

void GenericInputWidget::retranslateUi() {
    if (inputEdit_) {
        inputEdit_->setPlaceholderText(tr("Enter data to send..."));
    }
    if (hexRadio_) {
        hexRadio_->setText(tr("HEX"));
    }
    if (asciiRadio_) {
        asciiRadio_->setText(tr("ASCII"));
    }
    if (autoSendCheck_) {
        autoSendCheck_->setText(tr("Auto Send"));
    }
    if (intervalSpin_) {
        intervalSpin_->setSuffix(tr(" ms"));
    }
    if (sendFileBtn_) {
        sendFileBtn_->setText(tr("Send File"));
    }
    if (sendBtn_) {
        sendBtn_->setText(tr("Send"));
    }
    if (lineEndingCombo_) {
        QSignalBlocker b(lineEndingCombo_);
        const int current = lineEndingCombo_->currentIndex();
        lineEndingCombo_->clear();
        lineEndingCombo_->addItem(tr("No LF"), QVariant(0));
        lineEndingCombo_->addItem(tr("CR (\\r)"), QVariant(1));
        lineEndingCombo_->addItem(tr("LF (\\n)"), QVariant(2));
        lineEndingCombo_->addItem(tr("CRLF (\\r\\n)"), QVariant(3));
        lineEndingCombo_->setCurrentIndex(current);
    }
}

void GenericInputWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
