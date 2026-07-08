/**
 * @file ByteMonitorWidget.cpp
 * @brief Implementation of ByteMonitorWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ByteMonitorWidget.h"
#include "Config.h"
#include "../../core/common/ISettingsService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

namespace {

constexpr int kUiFlushIntervalMs = 120;
constexpr int kStatsUpdateIntervalMs = 500;
constexpr int kActionButtonMinWidth = 72;

QColor colorForTx() { return QColor(0, 0, 200); }
QColor colorForRx() { return QColor(0, 120, 0); }
QColor colorForInfo() { return Qt::gray; }
QColor colorForError() { return QColor(220, 20, 60); }
QColor colorForWarn() { return QColor(255, 140, 0); }

} // anonymous namespace

ByteMonitorWidget::ByteMonitorWidget(core::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    elapsedTimer_.start();
    setupUi();

    connect(this, &ByteMonitorWidget::txDataReceived,
            this, &ByteMonitorWidget::onTxData, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::rxDataReceived,
            this, &ByteMonitorWidget::onRxData, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::infoMessageReceived,
            this, &ByteMonitorWidget::onInfoMessage, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::errorMessageReceived,
            this, &ByteMonitorWidget::onErrorMessage, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::warnMessageReceived,
            this, &ByteMonitorWidget::onWarnMessage, Qt::QueuedConnection);

    flushTimer_ = new QTimer(this);
    flushTimer_->setInterval(kUiFlushIntervalMs);
    connect(flushTimer_, &QTimer::timeout, this, &ByteMonitorWidget::onFlushPending);

    statsTimer_ = new QTimer(this);
    statsTimer_->setInterval(kStatsUpdateIntervalMs);
    connect(statsTimer_, &QTimer::timeout, this, &ByteMonitorWidget::onStatsTimer);
    statsTimer_->start();
}

ByteMonitorWidget::~ByteMonitorWidget() {
    flushPending();
}

void ByteMonitorWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);

    auto toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(4);

    auto hexBtn = new QPushButton(this);
    hexBtn->setCheckable(true);
    hexBtn->setChecked(true);
    hexBtn->setMinimumWidth(kActionButtonMinWidth);

    auto asciiBtn = new QPushButton(this);
    asciiBtn->setCheckable(true);
    asciiBtn->setMinimumWidth(kActionButtonMinWidth);

    displayGroup_ = new QButtonGroup(this);
    displayGroup_->addButton(hexBtn, static_cast<int>(DisplayMode::Hex));
    displayGroup_->addButton(asciiBtn, static_cast<int>(DisplayMode::Ascii));
    connect(displayGroup_, &QButtonGroup::idClicked,
            this, &ByteMonitorWidget::onDisplayModeChanged);

    toolbarLayout->addWidget(hexBtn);
    toolbarLayout->addWidget(asciiBtn);

    toolbarLayout->addSpacing(8);

    timestampCombo_ = new QComboBox(this);
    timestampCombo_->setMinimumWidth(90);
    connect(timestampCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ByteMonitorWidget::onTimestampFormatChanged);
    toolbarLayout->addWidget(timestampCombo_);

    toolbarLayout->addSpacing(8);

    showTxCheck_ = new QCheckBox(this);
    showTxCheck_->setChecked(true);
    toolbarLayout->addWidget(showTxCheck_);

    showRxCheck_ = new QCheckBox(this);
    showRxCheck_->setChecked(true);
    toolbarLayout->addWidget(showRxCheck_);

    autoScrollCheck_ = new QCheckBox(this);
    autoScrollCheck_->setChecked(true);
    toolbarLayout->addWidget(autoScrollCheck_);

    toolbarLayout->addSpacing(8);

    pauseBtn_ = new QPushButton(this);
    pauseBtn_->setCheckable(true);
    pauseBtn_->setChecked(false);
    pauseBtn_->setMinimumWidth(kActionButtonMinWidth);
    connect(pauseBtn_, &QPushButton::toggled, this, &ByteMonitorWidget::onPauseToggled);
    toolbarLayout->addWidget(pauseBtn_);

    clearBtn_ = new QPushButton(this);
    clearBtn_->setMinimumWidth(kActionButtonMinWidth);
    connect(clearBtn_, &QPushButton::clicked, this, &ByteMonitorWidget::onClearClicked);
    toolbarLayout->addWidget(clearBtn_);

    saveBtn_ = new QPushButton(this);
    saveBtn_->setMinimumWidth(kActionButtonMinWidth);
    connect(saveBtn_, &QPushButton::clicked, this, &ByteMonitorWidget::onSaveClicked);
    toolbarLayout->addWidget(saveBtn_);

    recordBtn_ = new QPushButton(this);
    recordBtn_->setCheckable(true);
    recordBtn_->setChecked(false);
    recordBtn_->setMinimumWidth(kActionButtonMinWidth);
    connect(recordBtn_, &QPushButton::toggled, this, &ByteMonitorWidget::onRecordToggled);
    toolbarLayout->addWidget(recordBtn_);

    toolbarLayout->addStretch();
    mainLayout->addLayout(toolbarLayout);

    statsBar_ = new QWidget(this);
    auto statsLayout = new QHBoxLayout(statsBar_);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(12);

    txStatsLabel_ = new QLabel(statsBar_);
    txStatsLabel_->setStyleSheet(QStringLiteral("color: rgb(0,0,200); font-weight: bold;"));
    statsLayout->addWidget(txStatsLabel_);

    rxStatsLabel_ = new QLabel(statsBar_);
    rxStatsLabel_->setStyleSheet(QStringLiteral("color: rgb(0,120,0); font-weight: bold;"));
    statsLayout->addWidget(rxStatsLabel_);

    rateLabel_ = new QLabel(statsBar_);
    statsLayout->addWidget(rateLabel_);

    statsLayout->addStretch();
    mainLayout->addWidget(statsBar_);

    logView_ = new QListView(this);
    logModel_ = new LogListModel(config::Ui::kByteMonitorMaxBlockCount, this);
    logView_->setModel(logModel_);
    logView_->setAlternatingRowColors(true);
    logView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    logView_->setContextMenuPolicy(Qt::CustomContextMenu);
    logView_->setMinimumHeight(64);
    logView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(logView_, &QListView::customContextMenuRequested, [this](const QPoint& pos) {
        QMenu menu(this);
        menu.addAction(tr("Copy"), this, &ByteMonitorWidget::onCopyClicked);
        menu.exec(logView_->mapToGlobal(pos));
    });
    mainLayout->addWidget(logView_);

    connect(showTxCheck_, &QCheckBox::toggled, this, &ByteMonitorWidget::saveSettings);
    connect(showRxCheck_, &QCheckBox::toggled, this, &ByteMonitorWidget::saveSettings);
    connect(autoScrollCheck_, &QCheckBox::toggled, this, &ByteMonitorWidget::saveSettings);

    retranslateUi();
    updateStatsDisplay();
}

void ByteMonitorWidget::appendTx(const QByteArray& data) {
    emit txDataReceived(data);
}

void ByteMonitorWidget::appendRx(const QByteArray& data) {
    emit rxDataReceived(data);
}

void ByteMonitorWidget::appendMessage(bool isTx, const QByteArray& data) {
    if (isTx) {
        emit txDataReceived(data);
    } else {
        emit rxDataReceived(data);
    }
}

void ByteMonitorWidget::appendMessageWithClient(bool isTx, const QByteArray& data, int clientId) {
    QByteArray tagged = QByteArrayLiteral("[Client ") + QByteArray::number(clientId) + QByteArrayLiteral("] ");
    tagged.append(data);
    appendMessage(isTx, tagged);
}

void ByteMonitorWidget::appendInfo(const QString& message) {
    emit infoMessageReceived(message);
}

void ByteMonitorWidget::appendError(const QString& message) {
    emit errorMessageReceived(message);
}

void ByteMonitorWidget::appendWarn(const QString& message) {
    emit warnMessageReceived(message);
}

void ByteMonitorWidget::clear() {
    flushPending();
    pendingLines_.clear();
    stats_.reset();
    lastAppendTimeMs_ = 0;
    if (logModel_) {
        logModel_->clearAll();
    }
    updateStatsDisplay();
}

void ByteMonitorWidget::onTxData(QByteArray data) {
    if (!data.isEmpty() && !showTxCheck_->isChecked()) {
        return;
    }
    stats_.update(TrafficStats::Tx, data.size());
    const QString text = tr("[%1] [TX] %2").arg(formatTimestamp(), formatData(data));
    appendLogLine(text, colorForTx());
}

void ByteMonitorWidget::onRxData(QByteArray data) {
    if (!data.isEmpty() && !showRxCheck_->isChecked()) {
        return;
    }
    stats_.update(TrafficStats::Rx, data.size());
    const QString text = tr("[%1] [RX] %2").arg(formatTimestamp(), formatData(data));
    appendLogLine(text, colorForRx());
}

void ByteMonitorWidget::onInfoMessage(QString message) {
    const QString text = timestampFormat_ == TimestampFormat::None
        ? tr("[INFO] %1").arg(message)
        : tr("[%1] [INFO] %2").arg(formatTimestamp(), message);
    appendLogLine(text, colorForInfo());
}

void ByteMonitorWidget::onErrorMessage(QString message) {
    const QString text = timestampFormat_ == TimestampFormat::None
        ? tr("[ERROR] %1").arg(message)
        : tr("[%1] [ERROR] %2").arg(formatTimestamp(), message);
    appendLogLine(text, colorForError());
}

void ByteMonitorWidget::onWarnMessage(QString message) {
    const QString text = timestampFormat_ == TimestampFormat::None
        ? tr("[WARN] %1").arg(message)
        : tr("[%1] [WARN] %2").arg(formatTimestamp(), message);
    appendLogLine(text, colorForWarn());
}

void ByteMonitorWidget::onFlushPending() {
    flushPending();
}

void ByteMonitorWidget::onStatsTimer() {
    stats_.updateRate(kStatsUpdateIntervalMs);
    updateStatsDisplay();
}

void ByteMonitorWidget::onDisplayModeChanged(int id) {
    displayMode_ = static_cast<DisplayMode>(id);
    rebuildDisplay();
    saveSettings();
}

void ByteMonitorWidget::onTimestampFormatChanged(int index) {
    timestampFormat_ = static_cast<TimestampFormat>(index);
    saveSettings();
}

void ByteMonitorWidget::onRecordToggled(bool checked) {
    if (checked) {
        const QString path = QFileDialog::getSaveFileName(
            this, tr("Record Log"), QString(),
            tr("Text Files (*.txt);;All Files (*)"));
        if (path.isEmpty()) {
            QSignalBlocker b(recordBtn_);
            recordBtn_->setChecked(false);
            return;
        }
        if (!recorder_.start(path)) {
            appendError(tr("Failed to start recording: %1").arg(path));
            QSignalBlocker b(recordBtn_);
            recordBtn_->setChecked(false);
            return;
        }
        recordBtn_->setStyleSheet(QStringLiteral("color: red; font-weight: bold;"));
        recordBtn_->setText(QStringLiteral("\u25CF ") + tr("Stop"));
    } else {
        recorder_.stop();
        recordBtn_->setStyleSheet(QString());
        retranslateUi();
    }
}

void ByteMonitorWidget::onPauseToggled(bool checked) {
    paused_ = checked;
    if (!paused_) {
        if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_) {
            logView_->scrollToBottom();
        }
    }
}

void ByteMonitorWidget::onClearClicked() {
    clear();
}

void ByteMonitorWidget::onSaveClicked() {
    flushPending();
    if (!logModel_) {
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, tr("Save Log"), QString(),
        tr("Text Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        appendError(tr("Cannot write file: %1").arg(path));
        return;
    }

    QTextStream out(&file);
    const int count = logModel_->rowCount();
    for (int i = 0; i < count; ++i) {
        out << logModel_->lineAt(i) << "\n";
    }
}

void ByteMonitorWidget::onCopyClicked() {
    flushPending();
    if (!logView_ || !logModel_) {
        return;
    }
    QStringList selectedText;
    const auto indexes = logView_->selectionModel()
        ? logView_->selectionModel()->selectedRows()
        : QModelIndexList{};
    for (const auto& index : indexes) {
        selectedText << logModel_->lineAt(index.row());
    }
    if (!selectedText.isEmpty()) {
        QApplication::clipboard()->setText(selectedText.join("\n"));
    }
}

void ByteMonitorWidget::appendLogLine(const QString& text, const QColor& color) {
    if (recorder_.isRecording()) {
        recorder_.writeLine(text);
    }
    pendingLines_.append({text, color});
    if (flushTimer_ && !flushTimer_->isActive()) {
        flushTimer_->start();
    }
}

void ByteMonitorWidget::flushPending() {
    if (pendingLines_.isEmpty() || !logModel_) {
        if (flushTimer_) {
            flushTimer_->stop();
        }
        return;
    }

    QList<LogEntry> batch;
    batch.reserve(pendingLines_.size());
    for (const auto& line : pendingLines_) {
        batch.append({line.text, line.color});
    }
    pendingLines_.clear();
    if (flushTimer_) {
        flushTimer_->stop();
    }
    logModel_->appendEntries(batch);
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_ && !paused_) {
        logView_->scrollToBottom();
    }
}

QString ByteMonitorWidget::formatData(const QByteArray& data) const {
    if (displayMode_ == DisplayMode::Ascii) {
        QString result;
        result.reserve(data.size());
        for (int i = 0; i < data.size(); ++i) {
            const unsigned char ch = static_cast<unsigned char>(data.at(i));
            if (ch >= 32 && ch <= 126) {
                result += QChar(ch);
            } else {
                result += QChar('.');
            }
        }
        return result;
    }
    return QString(data.toHex(' ').toUpper());
}

QString ByteMonitorWidget::formatTimestamp() {
    switch (timestampFormat_) {
    case TimestampFormat::Relative: {
        const qint64 now = elapsedTimer_.elapsed();
        const qint64 delta = (lastAppendTimeMs_ == 0) ? 0 : (now - lastAppendTimeMs_);
        lastAppendTimeMs_ = now;
        return QStringLiteral("+%1ms").arg(delta);
    }
    case TimestampFormat::None:
        return {};
    case TimestampFormat::Absolute:
    default: {
        const QDateTime now = QDateTime::currentDateTimeUtc().toLocalTime();
        lastAppendTimeMs_ = elapsedTimer_.elapsed();
        return now.toString("HH:mm:ss.zzz");
    }
    }
}

void ByteMonitorWidget::updateStatsDisplay() {
    if (txStatsLabel_) {
        txStatsLabel_->setText(tr("TX: %1 (%2 pkts)")
            .arg(TrafficStats::formatSize(stats_.txBytes))
            .arg(stats_.txPackets));
    }
    if (rxStatsLabel_) {
        rxStatsLabel_->setText(tr("RX: %1 (%2 pkts)")
            .arg(TrafficStats::formatSize(stats_.rxBytes))
            .arg(stats_.rxPackets));
    }
    if (rateLabel_) {
        rateLabel_->setText(tr("Rate: %1 / %2")
            .arg(TrafficStats::formatRate(stats_.txRate))
            .arg(TrafficStats::formatRate(stats_.rxRate)));
    }
}

void ByteMonitorWidget::rebuildDisplay() {
    flushPending();
    if (!logModel_) {
        return;
    }

    const int count = logModel_->rowCount();
    QList<LogEntry> rebuilt;
    rebuilt.reserve(count);
    for (int i = 0; i < count; ++i) {
        const QString line = logModel_->lineAt(i);
        QColor color = Qt::gray;
        if (line.contains(QLatin1String("[TX]"))) {
            color = colorForTx();
        } else if (line.contains(QLatin1String("[RX]"))) {
            color = colorForRx();
        } else if (line.contains(QLatin1String("[ERROR]"))) {
            color = colorForError();
        } else if (line.contains(QLatin1String("[WARN]"))) {
            color = colorForWarn();
        }
        rebuilt.append({line, color});
    }
    logModel_->replaceEntries(rebuilt);
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_) {
        logView_->scrollToBottom();
    }
}

bool ByteMonitorWidget::isPaused() const {
    return paused_;
}

bool ByteMonitorWidget::isRecording() const {
    return recorder_.isRecording();
}

void ByteMonitorWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    loadSettings();
}

void ByteMonitorWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) {
        return;
    }

    const int displayMode = settingsService_->value(settingsGroup_ + "/displayMode").toInt();
    const int timestampFormat = settingsService_->value(settingsGroup_ + "/timestampFormat").toInt();
    const bool showTx = settingsService_->contains(settingsGroup_ + "/showTx")
        ? settingsService_->value(settingsGroup_ + "/showTx").toBool() : true;
    const bool showRx = settingsService_->contains(settingsGroup_ + "/showRx")
        ? settingsService_->value(settingsGroup_ + "/showRx").toBool() : true;
    const bool autoScroll = settingsService_->contains(settingsGroup_ + "/autoScroll")
        ? settingsService_->value(settingsGroup_ + "/autoScroll").toBool() : true;

    {
        QSignalBlocker b1(displayGroup_);
        QSignalBlocker b2(timestampCombo_);
        QSignalBlocker b3(showTxCheck_);
        QSignalBlocker b4(showRxCheck_);
        QSignalBlocker b5(autoScrollCheck_);

        const int mode = qBound(0, displayMode, 1);
        displayMode_ = static_cast<DisplayMode>(mode);
        if (displayGroup_->button(mode)) {
            displayGroup_->button(mode)->setChecked(true);
        }

        const int format = qBound(0, timestampFormat, 2);
        timestampFormat_ = static_cast<TimestampFormat>(format);
        timestampCombo_->setCurrentIndex(format);

        showTxCheck_->setChecked(showTx);
        showRxCheck_->setChecked(showRx);
        autoScrollCheck_->setChecked(autoScroll);
    }
}

void ByteMonitorWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) {
        return;
    }
    settingsService_->setValue(settingsGroup_ + "/displayMode", static_cast<int>(displayMode_));
    settingsService_->setValue(settingsGroup_ + "/timestampFormat", static_cast<int>(timestampFormat_));
    settingsService_->setValue(settingsGroup_ + "/showTx", showTxCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/showRx", showRxCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/autoScroll", autoScrollCheck_->isChecked());
}

void ByteMonitorWidget::retranslateUi() {
    if (displayGroup_) {
        if (auto btn = displayGroup_->button(static_cast<int>(DisplayMode::Hex))) {
            btn->setText(tr("HEX"));
        }
        if (auto btn = displayGroup_->button(static_cast<int>(DisplayMode::Ascii))) {
            btn->setText(tr("ASCII"));
        }
    }
    if (timestampCombo_) {
        QSignalBlocker b(timestampCombo_);
        timestampCombo_->clear();
        timestampCombo_->addItem(tr("Absolute"));
        timestampCombo_->addItem(tr("Relative"));
        timestampCombo_->addItem(tr("None"));
        timestampCombo_->setCurrentIndex(static_cast<int>(timestampFormat_));
    }
    if (showTxCheck_) showTxCheck_->setText(tr("TX"));
    if (showRxCheck_) showRxCheck_->setText(tr("RX"));
    if (autoScrollCheck_) autoScrollCheck_->setText(tr("Auto Scroll"));
    if (pauseBtn_) {
        pauseBtn_->setText(paused_ ? tr("Resume") : tr("Pause"));
    }
    if (clearBtn_) clearBtn_->setText(tr("Clear"));
    if (saveBtn_) saveBtn_->setText(tr("Save"));
    if (!recorder_.isRecording() && recordBtn_) {
        recordBtn_->setText(tr("Record"));
    }
    updateStatsDisplay();
}

void ByteMonitorWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets