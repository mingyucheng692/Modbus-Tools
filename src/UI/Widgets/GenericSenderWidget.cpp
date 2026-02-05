#include "GenericSenderWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QComboBox>
#include <QRegularExpression>

GenericSenderWidget::GenericSenderWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Input Area
    inputEdit_ = new QTextEdit(this);
    inputEdit_->setPlaceholderText("Enter data to send...");
    mainLayout->addWidget(inputEdit_);
    
    // Control Bar
    QHBoxLayout* ctrlLayout = new QHBoxLayout();
    
    formatCombo_ = new QComboBox(this);
    formatCombo_->addItem("Hex");
    formatCombo_->addItem("ASCII");
    
    autoSendCheck_ = new QCheckBox("Auto Send", this);
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(10, 60000);
    intervalSpin_->setValue(1000);
    intervalSpin_->setSuffix(" ms");
    
    sendBtn_ = new QPushButton("Send", this);
    
    ctrlLayout->addWidget(new QLabel("Format:"));
    ctrlLayout->addWidget(formatCombo_);
    ctrlLayout->addStretch();
    ctrlLayout->addWidget(autoSendCheck_);
    ctrlLayout->addWidget(intervalSpin_);
    ctrlLayout->addWidget(sendBtn_);
    
    mainLayout->addLayout(ctrlLayout);
    
    autoTimer_ = new QTimer(this);
    
    connect(sendBtn_, &QPushButton::clicked, this, &GenericSenderWidget::onSendClicked);
    connect(autoSendCheck_, &QCheckBox::toggled, this, &GenericSenderWidget::onAutoSendToggled);
    connect(autoTimer_, &QTimer::timeout, this, &GenericSenderWidget::onTimerTimeout);
}

void GenericSenderWidget::onSendClicked() {
    auto data = parseInput();
    if (!data.empty()) {
        emit sendRaw(data);
    }
}

void GenericSenderWidget::onAutoSendToggled(bool checked) {
    if (checked) {
        autoTimer_->start(intervalSpin_->value());
        inputEdit_->setReadOnly(true);
    } else {
        autoTimer_->stop();
        inputEdit_->setReadOnly(false);
    }
}

void GenericSenderWidget::onTimerTimeout() {
    onSendClicked();
}

std::vector<uint8_t> GenericSenderWidget::parseInput() {
    std::vector<uint8_t> data;
    QString text = inputEdit_->toPlainText();
    
    if (formatCombo_->currentText() == "Hex") {
        QStringList parts = text.split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);
        for (const QString& part : parts) {
            bool ok;
            uint8_t b = static_cast<uint8_t>(part.toUInt(&ok, 16));
            if (ok) data.push_back(b);
        }
    } else {
        QByteArray ba = text.toUtf8();
        data.assign(ba.begin(), ba.end());
    }
    return data;
}
