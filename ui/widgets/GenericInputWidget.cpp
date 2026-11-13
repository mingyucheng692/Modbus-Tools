// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "GenericInputWidget.h"
#include "Config.h"
#include "infra/config/ISettingsService.h"
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
#include <QDebug>
#include <QEvent>
#include <QSignalBlocker>
#include <QStringConverter>
#include <QVariant>

namespace {

QByteArray encodeTextWithEncoding(const QString& text, int encoding) {
    switch (encoding) {
    case 1: { // Latin-1
        QByteArray data;
        data.reserve(text.size());
        for (const QChar& ch : text) {
            data.append(ch.unicode() <= 0xFF ? static_cast<char>(ch.unicode()) : '?');
        }
        return data;
    }
    case 3: { // ASCII
        QByteArray data;
        data.reserve(text.size());
        for (const QChar& ch : text) {
            data.append(ch.unicode() <= 0x7F ? static_cast<char>(ch.unicode()) : '?');
        }
        return data;
    }
    default:
        return text.toUtf8();
    }
}

QString decodeBytesWithEncoding(const QByteArray& data, int encoding) {
    if (encoding == 1) {
        return QString::fromLatin1(data);
    }
    if (encoding == 3) {
        QString decoded;
        decoded.reserve(data.size());
        for (const unsigned char byte : data) {
            decoded.append(byte <= 0x7F ? QChar::fromLatin1(static_cast<char>(byte)) : QLatin1Char('?'));
        }
        return decoded;
    }
    return QString::fromUtf8(data);
}

bool tryParseCleanHex(const QString& text, QByteArray* out) {
    QString clean;
    clean.reserve(text.size());
    for (const QChar& ch : text) {
        if (ch.isSpace()) continue;
        const bool isHex = (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')) ||
                           (ch >= QLatin1Char('a') && ch <= QLatin1Char('f')) ||
                           (ch >= QLatin1Char('A') && ch <= QLatin1Char('F'));
        if (!isHex) return false;
        clean.append(ch);
    }
    if (clean.size() % 2 != 0) return false;
    if (out) *out = QByteArray::fromHex(clean.toLatin1());
    return true;
}

} // namespace

namespace ui::widgets {

GenericInputWidget::GenericInputWidget(infra::config::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService)
{
    setupUi();
}

GenericInputWidget::~GenericInputWidget() = default;

void GenericInputWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. Input Area
    inputEdit_ = new QTextEdit(this);
    inputEdit_->setLineWrapMode(QTextEdit::WidgetWidth);
    connect(inputEdit_, &QTextEdit::textChanged, this, [this]() {
        if (!inputEdit_->styleSheet().isEmpty()) {
            inputEdit_->setStyleSheet(QString());
            inputEdit_->setToolTip(QString());
        }
    });
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
    encodingCombo_->addItem(QStringLiteral("ASCII"), QVariant(3));
    controlLayout->addWidget(encodingCombo_);
    
    // Auto Send
    controlLayout->addSpacing(20);
    autoSendCheck_ = new QCheckBox(this);
    controlLayout->addWidget(autoSendCheck_);
    
    intervalSpin_ = new QSpinBox(this);
    intervalSpin_->setRange(config::GenericIo::kMinInputIntervalMs,
                            config::GenericIo::kMaxInputIntervalMs);
    intervalSpin_->setValue(config::GenericIo::kDefaultInputIntervalMs);
    intervalSpin_->setSuffix(" ms");
    intervalSpin_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    intervalSpin_->setAlignment(Qt::AlignCenter);
    intervalSpin_->setFixedWidth(64);
    intervalSpin_->setEnabled(false); // Enabled when checked
    controlLayout->addWidget(intervalSpin_);

    controlLayout->addStretch();

    // Send History
    sendHistoryCombo_ = new QComboBox(this);
    sendHistoryCombo_->setEditable(true);
    sendHistoryCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    sendHistoryCombo_->setMinimumContentsLength(8);
    sendHistoryCombo_->setMinimumWidth(80);
    sendHistoryCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    controlLayout->addWidget(sendHistoryCombo_, 1);

    // Send Button
    sendBtn_ = new QPushButton(this);
    sendBtn_->setMinimumWidth(64);
    controlLayout->addWidget(sendBtn_);

    mainLayout->addLayout(controlLayout);

    // Connections
    connect(hexRadio_, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) onFormatChanged();
    });
    connect(asciiRadio_, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) onFormatChanged();
    });
    connect(hexRadio_, &QRadioButton::toggled, this, &GenericInputWidget::saveSettings);
    connect(asciiRadio_, &QRadioButton::toggled, this, &GenericInputWidget::saveSettings);

    connect(autoSendCheck_, &QCheckBox::toggled, this, &GenericInputWidget::onAutoSendToggled);
    connect(autoSendCheck_, &QCheckBox::toggled, this, &GenericInputWidget::saveSettings);
    connect(intervalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int val) {
        if (autoSendCheck_ && autoSendCheck_->isChecked()) {
            emit autoSendIntervalChanged(val);
        }
    });
    connect(intervalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &GenericInputWidget::saveSettings);
    connect(sendBtn_, &QPushButton::clicked, this, &GenericInputWidget::onSendClicked);

    connect(lineEndingCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GenericInputWidget::saveSettings);
    connect(encodingCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GenericInputWidget::saveSettings);

    connect(sendHistoryCombo_, QOverload<int>::of(&QComboBox::activated),
            this, &GenericInputWidget::onHistoryActivated);

    retranslateUi();
}

void GenericInputWidget::onFormatChanged() {
    const QString text = inputEdit_->toPlainText();
    inputEdit_->setStyleSheet(QString());
    inputEdit_->setToolTip(QString());
    if (text.isEmpty()) return;

    const int encoding = encodingCombo_ ? encodingCombo_->currentData().toInt() : 0;
    if (hexRadio_->isChecked()) {
        const QByteArray data = encodeTextWithEncoding(text, encoding);
        inputEdit_->setText(QString::fromLatin1(data.toHex(' ').toUpper()));
        return;
    }

    QByteArray parsedHex;
    if (!tryParseCleanHex(text, &parsedHex)) {
        const QSignalBlocker b1(hexRadio_);
        const QSignalBlocker b2(asciiRadio_);
        hexRadio_->setChecked(true);
        inputEdit_->setStyleSheet(QStringLiteral("QTextEdit { border: 1px solid #EF4444; }"));
        inputEdit_->setToolTip(tr("Invalid HEX input: enter an even number of hex digits (0-9, A-F)."));
        return;
    }
    inputEdit_->setText(decodeBytesWithEncoding(parsedHex, encoding));
}

void GenericInputWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (sendHistoryCombo_) {
        sendHistoryCombo_->clear();
    }
    loadSettings();
}

void GenericInputWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    const QString key = settingsGroup_ + "/format";
    QString format = settingsService_->value(key).toString().toLower();
    const bool autoSend = settingsService_->value(settingsGroup_ + "/autoSend").toBool();
    const int intervalMs = settingsService_->contains(settingsGroup_ + "/intervalMs")
        ? settingsService_->value(settingsGroup_ + "/intervalMs").toInt()
        : config::GenericIo::kDefaultInputIntervalMs;
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
    lineEndingCombo_->setCurrentIndex(qBound(0, lineEnding, 3));
    int encIndex = encodingCombo_->findData(encoding);
    if (encIndex < 0) encIndex = 0; // fallback to UTF-8 for unknown/legacy encoding values
    encodingCombo_->setCurrentIndex(encIndex);

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
        data = encodeTextWithEncoding(text, enc);
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
    intervalSpin_->setEnabled(checked);
    sendBtn_->setEnabled(true);
    emit autoSendToggled(checked, intervalSpin_->value());
}

void GenericInputWidget::setAutoSendEnabled(bool enabled) {
    if (autoSendCheck_) {
        QSignalBlocker blocker(autoSendCheck_);
        autoSendCheck_->setChecked(enabled);
    }
    if (intervalSpin_) {
        intervalSpin_->setEnabled(enabled);
    }
}

bool GenericInputWidget::isAutoSendEnabled() const {
    return autoSendCheck_ && autoSendCheck_->isChecked();
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
    sendHistoryCombo_->clear();
    sendHistoryCombo_->setCurrentIndex(-1);
    if (settingsGroup_.isEmpty() || !settingsService_) return;

    const QString key = settingsGroup_ + "/sendHistory";
    if (!settingsService_->contains(key)) return;

    const QStringList items = settingsService_->value(key).toStringList();
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
        autoSendCheck_->setToolTip(tr("Send repeatedly at fixed interval"));
    }
    if (intervalSpin_) {
        intervalSpin_->setSuffix(tr(" ms"));
        intervalSpin_->setToolTip(tr("Auto-send interval (ms)"));
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
        lineEndingCombo_->setToolTip(tr("Line ending appended on send"));
    }
    if (encodingCombo_) {
        QSignalBlocker b(encodingCombo_);
        const int current = encodingCombo_->currentIndex();
        encodingCombo_->setItemText(0, tr("UTF-8"));
        encodingCombo_->setItemText(1, tr("Latin-1"));
        encodingCombo_->setItemText(2, tr("ASCII"));
        encodingCombo_->setCurrentIndex(current);
        encodingCombo_->setToolTip(tr("Text character encoding"));
    }
    if (sendHistoryCombo_) {
        sendHistoryCombo_->setToolTip(tr("Recent sent messages"));
    }
}

void GenericInputWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
