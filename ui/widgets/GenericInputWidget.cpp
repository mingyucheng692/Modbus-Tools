#include "GenericInputWidget.h"
#include "../common/ISettingsService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QSignalBlocker>

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
    
    // Auto Send
    controlLayout->addSpacing(20);
    autoSendCheck_ = new QCheckBox(this);
    controlLayout->addWidget(autoSendCheck_);
    
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(10, 3600000); // 10ms to 1h
    intervalSpin_->setValue(1000);
    intervalSpin_->setSuffix(" ms");
    intervalSpin_->setEnabled(false); // Enabled when checked
    controlLayout->addWidget(intervalSpin_);
    
    // Send File
    controlLayout->addStretch();
    sendFileBtn_ = new QPushButton(this);
    controlLayout->addWidget(sendFileBtn_);
    
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

    QSignalBlocker b1(hexRadio_);
    QSignalBlocker b2(asciiRadio_);
    QSignalBlocker b3(autoSendCheck_);
    QSignalBlocker b4(intervalSpin_);
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
}

void GenericInputWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    const QString format = hexRadio_->isChecked() ? "hex" : "ascii";
    settingsService_->setValue(settingsGroup_ + "/format", format);
    settingsService_->setValue(settingsGroup_ + "/autoSend", autoSendCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/intervalMs", intervalSpin_->value());
}

QByteArray GenericInputWidget::getData() const {
    QString text = inputEdit_->toPlainText();
    if (hexRadio_->isChecked()) {
        QString hex = text;
        hex.remove(' ');
        hex.remove('\n');
        hex.remove('\r');
        return QByteArray::fromHex(hex.toUtf8());
    } else {
        return text.toUtf8();
    }
}

void GenericInputWidget::onSendClicked() {
    QByteArray data = getData();
    if (!data.isEmpty()) {
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

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file"));
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (!data.isEmpty()) {
        emit sendRequested(data);
    }
}

void GenericInputWidget::appendData(const QByteArray& /*data*/) {
    // Optional: flash success or something?
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
}

void GenericInputWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
