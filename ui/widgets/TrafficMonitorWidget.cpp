/**
 * @file TrafficMonitorWidget.cpp
 * @brief Implementation of TrafficMonitorWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TrafficMonitorWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QAbstractListModel>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QTimer>
#include <QModelIndex>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <memory>

namespace ui::widgets {

struct TrafficLogEntry {
    QString text;
    QColor color = Qt::gray;
};

class TrafficLogModel final : public QAbstractListModel {
public:
    explicit TrafficLogModel(QObject* parent = nullptr)
        : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : entries_.size();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
            return {};
        }
        const auto& entry = entries_.at(index.row());
        if (role == Qt::DisplayRole) {
            return entry.text;
        }
        if (role == Qt::ForegroundRole) {
            return entry.color;
        }
        return {};
    }

    void appendEntries(const QList<TrafficLogEntry>& newEntries) {
        if (newEntries.isEmpty()) {
            return;
        }
        beginResetModel();
        for (const auto& entry : newEntries) {
            entries_.append(entry);
        }
        const int maxRows = app::constants::Values::Ui::kTrafficMonitorMaxBlockCount;
        while (entries_.size() > maxRows) {
            entries_.removeFirst();
        }
        endResetModel();
    }

    void clearAll() {
        if (entries_.isEmpty()) {
            return;
        }
        beginResetModel();
        entries_.clear();
        endResetModel();
    }

    void replaceEntries(const QList<TrafficLogEntry>& newEntries) {
        beginResetModel();
        entries_ = newEntries;
        endResetModel();
    }

    QString lineAt(int row) const {
        if (row < 0 || row >= entries_.size()) {
            return {};
        }
        return entries_.at(row).text;
    }

private:
    QList<TrafficLogEntry> entries_;
};

namespace {

QList<TrafficLogEntry> toSingleEntryList(const QString& text, const QColor& color) {
    return {TrafficLogEntry{text, color}};
}

constexpr int kUiFlushIntervalMs = 120;
}

TrafficMonitorWidget::TrafficMonitorWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
    flushTimer_ = new QTimer(this);
    flushTimer_->setInterval(kUiFlushIntervalMs);
    connect(flushTimer_, &QTimer::timeout, this, &TrafficMonitorWidget::flushPendingEvents);
}

TrafficMonitorWidget::~TrafficMonitorWidget() {
    flushPendingEvents();
}

void TrafficMonitorWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    section_ = new CollapsibleSection(settingsService_, this);
    auto layout = new QVBoxLayout(section_->contentWidget());
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(6);

    // Toolbar
    auto toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(6);
    
    autoScrollCheck_ = new QCheckBox(this);
    autoScrollCheck_->setChecked(true);
    toolbarLayout->addWidget(autoScrollCheck_);

    rawFramesCheck_ = new QCheckBox(this);
    rawFramesCheck_->setChecked(false);
    toolbarLayout->addWidget(rawFramesCheck_);

    showTxCheck_ = new QCheckBox(this);
    showTxCheck_->setChecked(true);
    toolbarLayout->addWidget(showTxCheck_);

    showRxCheck_ = new QCheckBox(this);
    showRxCheck_->setChecked(true);
    toolbarLayout->addWidget(showRxCheck_);

    clearBtn_ = new QPushButton(this);
    toolbarLayout->addWidget(clearBtn_);
    
    saveBtn_ = new QPushButton(this);
    toolbarLayout->addWidget(saveBtn_);
    
    toolbarLayout->addStretch();
    layout->addLayout(toolbarLayout);

    rawHintLabel_ = new QLabel(this);
    rawHintLabel_->setVisible(false);
    rawHintLabel_->setWordWrap(true);
    layout->addWidget(rawHintLabel_);

    // Log View
    logView_ = new QListView(this);
    logModel_ = new TrafficLogModel(this);
    logView_->setModel(logModel_);
    logView_->setAlternatingRowColors(true);
    logView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    logView_->setContextMenuPolicy(Qt::CustomContextMenu);
    logView_->setMinimumHeight(64);
    logView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(logView_);

    mainLayout->addWidget(section_);
    setMinimumHeight(0);
    syncCollapsedGeometry(section_->isExpanded());

    // Connections
    connect(clearBtn_, &QPushButton::clicked, this, &TrafficMonitorWidget::clear);
    connect(saveBtn_, &QPushButton::clicked, this, &TrafficMonitorWidget::onSaveClicked);
    connect(autoScrollCheck_, &QCheckBox::toggled, this, &TrafficMonitorWidget::saveSettings);
    connect(rawFramesCheck_, &QCheckBox::toggled, this, [this]() {
        flushPendingEvents();
        syncDisplayModeUi();
        rebuildVisibleEntries();
        saveSettings();
    });
    connect(showTxCheck_, &QCheckBox::toggled, this, &TrafficMonitorWidget::saveSettings);
    connect(showRxCheck_, &QCheckBox::toggled, this, &TrafficMonitorWidget::saveSettings);
    connect(showTxCheck_, &QCheckBox::toggled, this, [this]() {
        flushPendingEvents();
        rebuildVisibleEntries();
    });
    connect(showRxCheck_, &QCheckBox::toggled, this, [this]() {
        flushPendingEvents();
        rebuildVisibleEntries();
    });
    connect(section_, &CollapsibleSection::expandedChanged, this, &TrafficMonitorWidget::syncCollapsedGeometry);
    connect(logView_, &QListView::customContextMenuRequested, [this](const QPoint& pos) {
        QMenu menu(this);
        menu.addAction(tr("Copy"), this, &TrafficMonitorWidget::onCopyClicked);
        menu.exec(logView_->mapToGlobal(pos));
    });
    
    retranslateUi();
    syncDisplayModeUi();
}

bool TrafficMonitorWidget::isRealtimeEvent(const ui::common::TrafficEvent& event) const {
    return event.level == ui::common::TrafficEventLevel::Warning
        || event.requestType == ui::common::TrafficRequestType::Connection;
}

TrafficMonitorWidget::DisplayMode TrafficMonitorWidget::currentDisplayMode() const {
    return (rawFramesCheck_ && rawFramesCheck_->isChecked())
        ? DisplayMode::RawFrames
        : DisplayMode::PollSummary;
}

bool TrafficMonitorWidget::isRawFramesModeEnabled() const {
    return currentDisplayMode() == DisplayMode::RawFrames;
}

void TrafficMonitorWidget::syncDisplayModeUi() {
    const bool rawFramesEnabled = currentDisplayMode() == DisplayMode::RawFrames;
    if (showTxCheck_) {
        showTxCheck_->setEnabled(rawFramesEnabled);
    }
    if (showRxCheck_) {
        showRxCheck_->setEnabled(rawFramesEnabled);
    }
    if (rawHintLabel_) {
        rawHintLabel_->setVisible(rawFramesEnabled);
    }
}

void TrafficMonitorWidget::appendEventToHistory(const ui::common::TrafficEvent& event) {
    eventHistory_.append(event);
    const int maxRows = app::constants::Values::Ui::kTrafficMonitorMaxBlockCount;
    while (eventHistory_.size() > maxRows) {
        eventHistory_.removeFirst();
    }
}

void TrafficMonitorWidget::rebuildVisibleEntries() {
    if (!logModel_) {
        return;
    }

    QList<TrafficLogEntry> rebuiltEntries;
    rebuiltEntries.reserve(eventHistory_.size());
    for (const auto& event : eventHistory_) {
        QString line;
        QColor color;
        if (renderEvent(event, line, color)) {
            rebuiltEntries.append({line, color});
        }
    }
    logModel_->replaceEntries(rebuiltEntries);
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_) {
        logView_->scrollToBottom();
    }
}

bool TrafficMonitorWidget::renderEvent(const ui::common::TrafficEvent& event, QString& outText, QColor& outColor) const {
    const QDateTime timestamp = event.timestamp.isValid() ? event.timestamp : QDateTime::currentDateTime();
    const QString timeStr = timestamp.toString("HH:mm:ss.zzz");
    outColor = Qt::gray;

    const bool rawFramesEnabled = currentDisplayMode() == DisplayMode::RawFrames;

    if (event.direction == ui::common::TrafficDirection::Tx) {
        if (!rawFramesEnabled) {
            return false;
        }
        if (!showTxCheck_->isChecked()) {
            return false;
        }
        outText = tr("[%1] [TX] %2").arg(timeStr, formatData(event.payload));
        outColor = Qt::blue;
        return true;
    }
    if (event.direction == ui::common::TrafficDirection::Rx) {
        if (!rawFramesEnabled) {
            return false;
        }
        if (!showRxCheck_->isChecked()) {
            return false;
        }
        outText = tr("[%1] [RX] %2").arg(timeStr, formatData(event.payload));
        outColor = Qt::darkGreen;
        return true;
    }
    if (event.level == ui::common::TrafficEventLevel::Warning) {
        outText = tr("[%1] [WARN] %2").arg(timeStr, event.summary);
        outColor = QColor(255, 140, 0);
        return true;
    }

    outText = tr("[%1] [INFO] %2").arg(timeStr, event.summary);
    return true;
}

void TrafficMonitorWidget::appendLogLine(const QString& text, const QColor& color) {
    if (!logModel_) {
        return;
    }
    logModel_->appendEntries(toSingleEntryList(text, color));
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_) {
        logView_->scrollToBottom();
    }
}

void TrafficMonitorWidget::appendTx(const QByteArray& data) {
    ui::common::TrafficEvent event;
    event.direction = ui::common::TrafficDirection::Tx;
    event.payload = data;
    appendEvent(event);
}

void TrafficMonitorWidget::appendRx(const QByteArray& data) {
    ui::common::TrafficEvent event;
    event.direction = ui::common::TrafficDirection::Rx;
    event.payload = data;
    appendEvent(event);
}

void TrafficMonitorWidget::appendInfo(const QString& message) {
    ui::common::TrafficEvent event;
    event.summary = message;
    appendEvent(event);
}

void TrafficMonitorWidget::appendWarning(const QString& message) {
    ui::common::TrafficEvent event;
    event.level = ui::common::TrafficEventLevel::Warning;
    event.summary = message;
    appendEvent(event);
}

void TrafficMonitorWidget::appendEvent(const ui::common::TrafficEvent& event) {
    appendEventToHistory(event);

    QString line;
    QColor color;
    if (!renderEvent(event, line, color)) {
        return;
    }

    if (isRealtimeEvent(event)) {
        flushPendingEvents();
        appendLogLine(line, color);
        return;
    }

    pendingEvents_.append(event);
    if (flushTimer_ && !flushTimer_->isActive()) {
        flushTimer_->start();
    }
}

void TrafficMonitorWidget::clear() {
    pendingEvents_.clear();
    eventHistory_.clear();
    if (flushTimer_) {
        flushTimer_->stop();
    }
    if (logModel_) {
        logModel_->clearAll();
    }
}

void TrafficMonitorWidget::flushPendingEvents() {
    if (pendingEvents_.isEmpty() || !logModel_) {
        if (flushTimer_) {
            flushTimer_->stop();
        }
        return;
    }

    QList<TrafficLogEntry> batch;
    batch.reserve(pendingEvents_.size());
    for (const auto& event : pendingEvents_) {
        QString line;
        QColor color;
        if (renderEvent(event, line, color)) {
            batch.append({line, color});
        }
    }
    pendingEvents_.clear();
    if (flushTimer_) {
        flushTimer_->stop();
    }
    if (batch.isEmpty()) {
        return;
    }
    logModel_->appendEntries(batch);
    if (autoScrollCheck_ && autoScrollCheck_->isChecked() && logView_) {
        logView_->scrollToBottom();
    }
}

void TrafficMonitorWidget::onSaveClicked() {
    flushPendingEvents();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QStringList exportLines;
    exportLines.reserve(eventHistory_.size());
    for (const auto& event : eventHistory_) {
        QString line;
        QColor color;
        if (renderEvent(event, line, color)) {
            exportLines << line;
        }
    }

    auto errorMessage = std::make_shared<QString>();
    struct SaveState {
        int nextIndex = 0;
        bool firstChunk = true;
    };
    auto state = std::make_shared<SaveState>();
    const int totalCount = exportLines.size();
    const int chunkSize = app::constants::Values::Ui::kTrafficLogExportChunkRows;
    auto scheduleNextChunk = std::make_shared<std::function<void()>>();

    *scheduleNextChunk = [this, fileName, totalCount, chunkSize, state, errorMessage, scheduleNextChunk, exportLines]() {
        if (!errorMessage->isEmpty()) {
            appendInfo(tr("Save failed: %1").arg(*errorMessage));
            return;
        }
        if (state->nextIndex >= totalCount) {
            return;
        }

        const int begin = state->nextIndex;
        const int end = qMin(begin + chunkSize, totalCount);
        QStringList lines;
        lines.reserve(end - begin);
        for (int i = begin; i < end; ++i) {
            lines << exportLines.at(i);
        }
        state->nextIndex = end;
        const bool firstChunk = state->firstChunk;
        state->firstChunk = false;

        auto* watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [this, errorMessage, scheduleNextChunk, watcher]() {
            watcher->deleteLater();
            if (!errorMessage->isEmpty()) {
                appendInfo(tr("Save failed: %1").arg(*errorMessage));
                return;
            }
            QMetaObject::invokeMethod(this, [scheduleNextChunk]() {
                (*scheduleNextChunk)();
            }, Qt::QueuedConnection);
        });

        watcher->setFuture(QtConcurrent::run([fileName, lines, firstChunk, errorMessage]() {
            QFile file(fileName);
            QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
            mode |= firstChunk ? QIODevice::Truncate : QIODevice::Append;
            if (!file.open(mode)) {
                *errorMessage = QObject::tr("Cannot write file: %1").arg(fileName);
                return;
            }

            QTextStream out(&file);
            for (const QString& line : lines) {
                out << line << "\n";
            }
        }));
    };

    (*scheduleNextChunk)();
}

void TrafficMonitorWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (section_) {
        section_->setSettingsKey(settingsGroup_ + "/ui/trafficMonitorCollapsed");
    }
    loadSettings();
}

void TrafficMonitorWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(autoScrollCheck_);
    QSignalBlocker b2(rawFramesCheck_);
    QSignalBlocker b3(showTxCheck_);
    QSignalBlocker b4(showRxCheck_);

    const QString autoKey = settingsGroup_ + "/autoScroll";
    const QString rawModeKey = settingsGroup_ + "/rawFramesMode";
    const QString txKey = settingsGroup_ + "/showTx";
    const QString rxKey = settingsGroup_ + "/showRx";

    const bool autoScroll = settingsService_->value(autoKey).toBool();
    const bool rawFramesMode = settingsService_->value(rawModeKey).toBool();
    const bool showTx = settingsService_->value(txKey).toBool();
    const bool showRx = settingsService_->value(rxKey).toBool();

    autoScrollCheck_->setChecked(autoScroll);
    rawFramesCheck_->setChecked(rawFramesMode);
    showTxCheck_->setChecked(showTx);
    showRxCheck_->setChecked(showRx);
    syncDisplayModeUi();
    rebuildVisibleEntries();
}

void TrafficMonitorWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/autoScroll", autoScrollCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/rawFramesMode", rawFramesCheck_ && rawFramesCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/showTx", showTxCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/showRx", showRxCheck_->isChecked());
}

void TrafficMonitorWidget::syncCollapsedGeometry(bool expanded) {
    const int expandedMinimumHeight = qMax(140, minimumSizeHint().height());
    setMinimumHeight(expanded ? expandedMinimumHeight : 0);
    QSizePolicy policy = sizePolicy();
    policy.setVerticalPolicy(expanded ? QSizePolicy::Preferred : QSizePolicy::Minimum);
    setSizePolicy(policy);
    updateGeometry();
}

QString TrafficMonitorWidget::formatData(const QByteArray& data) const {
    return QString(data.toHex(' ').toUpper());
}

void TrafficMonitorWidget::onCopyClicked() {
    flushPendingEvents();
    if (!logView_ || !logModel_) {
        return;
    }
    QStringList selectedText;
    const auto indexes = logView_->selectionModel() ? logView_->selectionModel()->selectedRows() : QModelIndexList{};
    for (const auto& index : indexes) {
        selectedText << logModel_->lineAt(index.row());
    }
    QApplication::clipboard()->setText(selectedText.join("\n"));
}

void TrafficMonitorWidget::retranslateUi() {
    if (section_) section_->setTitle(tr("Traffic Monitor"));
    if (autoScrollCheck_) autoScrollCheck_->setText(tr("Auto Scroll"));
    if (rawFramesCheck_) rawFramesCheck_->setText(tr("Raw Frames"));
    if (showTxCheck_) showTxCheck_->setText(QStringLiteral("TX"));
    if (showRxCheck_) showRxCheck_->setText(QStringLiteral("RX"));
    if (rawHintLabel_) rawHintLabel_->setText(tr("Raw Frames mode may reduce UI smoothness under high-frequency polling."));
    if (clearBtn_) clearBtn_->setText(tr("Clear"));
    if (saveBtn_) saveBtn_->setText(tr("Save"));
}

void TrafficMonitorWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
