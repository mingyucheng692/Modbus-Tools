// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ByteMonitorWidget.h"
#include "Config.h"
#include "infra/config/ISettingsService.h"
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
#include "common/FileExporter.h"
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

namespace {

constexpr int kUiFlushIntervalMs = 120;
constexpr int kActionButtonMinWidth = 72;

QColor colorForTx() { return QColor(0, 0, 200); }
QColor colorForRx() { return QColor(0, 120, 0); }
QColor colorForInfo() { return Qt::gray; }
QColor colorForError() { return QColor(220, 20, 60); }
QColor colorForWarn() { return QColor(255, 140, 0); }

} // anonymous namespace

ByteMonitorWidget::ByteMonitorWidget(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    elapsedTimer_.start();
    setupUi();

    connect(this, &ByteMonitorWidget::txDataReceived,
            this, &ByteMonitorWidget::onTxData, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::rxDataReceived,
            this, &ByteMonitorWidget::onRxData, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::clientTrafficReceived,
            this, &ByteMonitorWidget::onClientTrafficData, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::infoMessageReceived,
            this, &ByteMonitorWidget::onInfoMessage, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::errorMessageReceived,
            this, &ByteMonitorWidget::onErrorMessage, Qt::QueuedConnection);
    connect(this, &ByteMonitorWidget::warnMessageReceived,
            this, &ByteMonitorWidget::onWarnMessage, Qt::QueuedConnection);

    flushTimer_ = new QTimer(this);
    flushTimer_->setInterval(kUiFlushIntervalMs);
    connect(flushTimer_, &QTimer::timeout, this, &ByteMonitorWidget::onFlushPending);
}

ByteMonitorWidget::~ByteMonitorWidget() noexcept {
    if (flushTimer_) {
        flushTimer_->stop();
    }
    pendingLines_.clear();
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
    connect(logView_, &QListView::customContextMenuRequested, this, [this](const QPoint& pos) {
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
    emit clientTrafficReceived(isTx, data, clientId);
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
    if (flushTimer_) {
        flushTimer_->stop();
    }
    pendingLines_.clear();
    droppedPendingCount_ = 0;
    stats_.reset();
    lastAppendTimeMs_ = 0;
    if (logModel_) {
        logModel_->clearAll();
    }
    updateStatsDisplay();
}

QString ByteMonitorWidget::currentWallTime() const {
    return (timestampFormat_ == TimestampFormat::Absolute)
        ? QDateTime::currentDateTimeUtc().toLocalTime().toString("HH:mm:ss.zzz")
        : QString();
}

void ByteMonitorWidget::enqueuePendingLine(PendingLine&& line) {
    constexpr size_t kMaxPendingLines = static_cast<size_t>(config::Ui::kTrafficMonitorMaxBlockCount);
    while (pendingLines_.size() >= kMaxPendingLines) {
        pendingLines_.pop_front();
        ++droppedPendingCount_;
    }
    pendingLines_.push_back(std::move(line));
    if (!paused_ && flushTimer_ && !flushTimer_->isActive()) {
        flushTimer_->start();
    }
}

void ByteMonitorWidget::enqueueTraffic(bool isTx, QByteArray data, const QString& clientLabel) {
    const bool show = isTx ? showTxCheck_->isChecked() : showRxCheck_->isChecked();
    if (!data.isEmpty() && !show) {
        return;
    }
    stats_.update(isTx ? TrafficStats::Direction::Tx : TrafficStats::Direction::Rx, data.size());
    PendingLine line;
    line.kind = isTx ? PendingLine::Kind::Tx : PendingLine::Kind::Rx;
    line.payload = std::move(data);
    line.elapsedMs = elapsedTimer_.elapsed();
    line.wallTimeString = currentWallTime();
    line.clientLabel = clientLabel;
    enqueuePendingLine(std::move(line));
}

void ByteMonitorWidget::enqueueStatusMessage(PendingLine::Kind kind, QString message) {
    PendingLine line;
    line.kind = kind;
    line.message = std::move(message);
    line.elapsedMs = elapsedTimer_.elapsed();
    line.wallTimeString = currentWallTime();
    enqueuePendingLine(std::move(line));
}

void ByteMonitorWidget::onTxData(QByteArray data) {
    enqueueTraffic(true, std::move(data), QString());
}

void ByteMonitorWidget::onRxData(QByteArray data) {
    enqueueTraffic(false, std::move(data), QString());
}

void ByteMonitorWidget::onClientTrafficData(bool isTx, QByteArray data, int clientId) {
    enqueueTraffic(isTx, std::move(data), QStringLiteral("[Client %1]").arg(clientId));
}

void ByteMonitorWidget::onInfoMessage(QString message) {
    enqueueStatusMessage(PendingLine::Kind::Info, std::move(message));
}

void ByteMonitorWidget::onErrorMessage(QString message) {
    enqueueStatusMessage(PendingLine::Kind::Error, std::move(message));
}

void ByteMonitorWidget::onWarnMessage(QString message) {
    enqueueStatusMessage(PendingLine::Kind::Warn, std::move(message));
}

void ByteMonitorWidget::onFlushPending() {
    flushPending();
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

void ByteMonitorWidget::onPauseToggled(bool checked) {
    paused_ = checked;
    if (paused_) {
        if (flushTimer_) {
            flushTimer_->stop();
        }
        return;
    }
    flushPending();
    updateStatsDisplay();
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

    QStringList lines;
    const int count = logModel_->rowCount();
    lines.reserve(count);
    for (int i = 0; i < count; ++i) {
        lines << logModel_->lineAt(i);
    }

    QString error;
    if (!ui::common::exportLinesToFile(path, lines, false, &error)) {
        appendError(tr("Cannot write file: %1 (%2)").arg(path, error));
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
    Q_UNUSED(color);
    onInfoMessage(text);
}

void ByteMonitorWidget::reformatLogEntry(LogEntry& entry) const {
    const QString& ts = entry.timestampText;
    const auto formatTraffic = [&entry, this]() {
        const QString formatted = formatData(entry.rawBytes);
        return entry.clientLabel.isEmpty()
            ? formatted
            : QStringLiteral("%1 %2").arg(entry.clientLabel, formatted);
    };
    switch (entry.kind) {
    case LogEntry::Kind::Tx: {
        const QString body = formatTraffic();
        entry.text = ts.isEmpty() ? tr("[TX] %1").arg(body)
                                  : tr("[%1] [TX] %2").arg(ts, body);
        entry.color = colorForTx();
        break;
    }
    case LogEntry::Kind::Rx: {
        const QString body = formatTraffic();
        entry.text = ts.isEmpty() ? tr("[RX] %1").arg(body)
                                  : tr("[%1] [RX] %2").arg(ts, body);
        entry.color = colorForRx();
        break;
    }
    case LogEntry::Kind::Info:
        entry.text = ts.isEmpty() ? tr("[INFO] %1").arg(entry.message)
                                  : tr("[%1] [INFO] %2").arg(ts, entry.message);
        entry.color = colorForInfo();
        break;
    case LogEntry::Kind::Warn:
        entry.text = ts.isEmpty() ? tr("[WARN] %1").arg(entry.message)
                                  : tr("[%1] [WARN] %2").arg(ts, entry.message);
        entry.color = colorForWarn();
        break;
    case LogEntry::Kind::Error:
        entry.text = ts.isEmpty() ? tr("[ERROR] %1").arg(entry.message)
                                  : tr("[%1] [ERROR] %2").arg(ts, entry.message);
        entry.color = colorForError();
        break;
    }
}

LogEntry ByteMonitorWidget::buildLogEntry(const PendingLine& line) {
    QString ts;
    if (timestampFormat_ == TimestampFormat::Relative) {
        const qint64 delta = (lastAppendTimeMs_ == 0) ? 0 : (line.elapsedMs - lastAppendTimeMs_);
        lastAppendTimeMs_ = line.elapsedMs;
        ts = QStringLiteral("+%1ms").arg(delta);
    } else if (timestampFormat_ == TimestampFormat::Absolute) {
        lastAppendTimeMs_ = line.elapsedMs;
        ts = line.wallTimeString;
    }

    LogEntry entry;
    entry.kind = line.kind;
    entry.rawBytes = line.payload;
    entry.message = line.message;
    entry.timestampText = ts;
    entry.clientLabel = line.clientLabel;
    reformatLogEntry(entry);
    return entry;
}

void ByteMonitorWidget::flushPending() {
    if (paused_ || (pendingLines_.empty() && droppedPendingCount_ == 0) || !logModel_) {
        if (flushTimer_) {
            flushTimer_->stop();
        }
        return;
    }

    QList<LogEntry> batch;
    batch.reserve(static_cast<qsizetype>(pendingLines_.size()) + (droppedPendingCount_ > 0 ? 1 : 0));
    if (droppedPendingCount_ > 0) {
        PendingLine notice;
        notice.kind = PendingLine::Kind::Info;
        notice.message = tr("Paused overflow: dropped %1 buffered log entries").arg(droppedPendingCount_);
        notice.elapsedMs = elapsedTimer_.elapsed();
        notice.wallTimeString = currentWallTime();
        batch.append(buildLogEntry(notice));
        droppedPendingCount_ = 0;
    }
    for (const auto& line : pendingLines_) {
        batch.append(buildLogEntry(line));
    }
    pendingLines_.clear();
    if (flushTimer_) {
        flushTimer_->stop();
    }
    logModel_->appendEntries(batch);
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_) {
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
        txStatsLabel_->setText(tr("TX: %1").arg(TrafficStats::formatBytes(stats_.txBytes)));
    }
    if (rxStatsLabel_) {
        rxStatsLabel_->setText(tr("RX: %1").arg(TrafficStats::formatBytes(stats_.rxBytes)));
    }
}

void ByteMonitorWidget::rebuildDisplay() {
    if (!paused_) {
        flushPending();
    }
    if (!logModel_) {
        return;
    }

    const int count = logModel_->rowCount();
    QList<LogEntry> rebuilt;
    rebuilt.reserve(count);
    for (int i = 0; i < count; ++i) {
        LogEntry entry = logModel_->entryAt(i);
        if (!entry.rawBytes.isEmpty() || !entry.message.isEmpty() ||
            entry.kind == LogEntry::Kind::Tx || entry.kind == LogEntry::Kind::Rx) {
            reformatLogEntry(entry);
        }
        rebuilt.append(std::move(entry));
    }
    logModel_->replaceEntries(rebuilt);
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_ && !paused_) {
        logView_->scrollToBottom();
    }
}

bool ByteMonitorWidget::isPaused() const {
    return paused_;
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
    updateStatsDisplay();
}

void ByteMonitorWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
