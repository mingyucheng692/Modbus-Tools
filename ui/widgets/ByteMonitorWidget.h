/**
 * @file ByteMonitorWidget.h
 * @brief Generic byte-level traffic monitor with HEX/ASCII display and statistics.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QByteArray>
#include <QList>
#include <QColor>
#include <QString>
#include <QElapsedTimer>
#include "LogListModel.h"
// TrafficStats inlined below — no longer a separate header.

class QListView;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;
class QButtonGroup;
class QTimer;
class QEvent;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

/// Lightweight TX/RX byte statistics tracker.
/// Previously a separate header (TrafficStats.h); inlined here as the sole
/// consumer.
struct TrafficStats {
    enum class Direction { Tx, Rx };

    qint64 txBytes = 0;
    qint64 rxBytes = 0;
    qint64 txFrames = 0;
    qint64 rxFrames = 0;

    void update(Direction dir, qint64 bytes) {
        if (dir == Direction::Tx) {
            txBytes += bytes;
            ++txFrames;
        } else {
            rxBytes += bytes;
            ++rxFrames;
        }
    }

    QString formatStats() const {
        return QStringLiteral("TX: %1 | RX: %2")
            .arg(formatBytes(txBytes), formatBytes(rxBytes));
    }

    void reset() {
        txBytes = 0;
        rxBytes = 0;
        txFrames = 0;
        rxFrames = 0;
    }

    [[nodiscard]] static QString formatBytes(qint64 bytes) {
        if (bytes < 1024) {
            return QStringLiteral("%1 B").arg(bytes);
        }
        if (bytes < 1024 * 1024) {
            return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
        }
        return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    }

    [[nodiscard]] static QString formatSize(qint64 bytes) {
        return formatBytes(bytes);
    }
};

struct PendingLine {
    enum class Kind { Tx, Rx, Info, Warn, Error };
    Kind kind = Kind::Info;
    QByteArray payload;
    QString message;
    qint64 elapsedMs = 0;
    QString wallTimeString;
};

class ByteMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ByteMonitorWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ByteMonitorWidget() override;

    void appendTx(const QByteArray& data);
    void appendRx(const QByteArray& data);
    void appendInfo(const QString& message);
    void appendError(const QString& message);
    void appendWarn(const QString& message);
    void clear();
    void appendMessage(bool isTx, const QByteArray& data);
    void appendMessageWithClient(bool isTx, const QByteArray& data, int clientId);
    void setSettingsGroup(const QString& group);

signals:
    void txDataReceived(QByteArray data);
    void rxDataReceived(QByteArray data);
    void infoMessageReceived(QString message);
    void errorMessageReceived(QString message);
    void warnMessageReceived(QString message);

private slots:
    void onTxData(QByteArray data);
    void onRxData(QByteArray data);
    void onInfoMessage(QString message);
    void onErrorMessage(QString message);
    void onWarnMessage(QString message);
    void onFlushPending();
    void onDisplayModeChanged(int id);
    void onTimestampFormatChanged(int index);
    void onPauseToggled(bool checked);
    void onClearClicked();
    void onSaveClicked();
    void onCopyClicked();

private:
    enum class DisplayMode {
        Hex = 0,
        Ascii = 1
    };

    enum class TimestampFormat {
        Absolute = 0,
        Relative = 1,
        None = 2
    };

    void setupUi();
    void appendLogLine(const QString& text, const QColor& color);
    void flushPending();
    QString formatData(const QByteArray& data) const;
    QString formatTimestamp();
    void updateStatsDisplay();
    void rebuildDisplay();
    bool isPaused() const;
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    LogListModel* logModel_ = nullptr;
    TrafficStats stats_;
    QElapsedTimer elapsedTimer_;
    qint64 lastAppendTimeMs_ = 0;

    QListView* logView_ = nullptr;
    QButtonGroup* displayGroup_ = nullptr;
    QComboBox* timestampCombo_ = nullptr;
    QCheckBox* showTxCheck_ = nullptr;
    QCheckBox* showRxCheck_ = nullptr;
    QCheckBox* autoScrollCheck_ = nullptr;
    QPushButton* pauseBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* saveBtn_ = nullptr;
    QLabel* txStatsLabel_ = nullptr;
    QLabel* rxStatsLabel_ = nullptr;
    QWidget* statsBar_ = nullptr;
    QTimer* flushTimer_ = nullptr;
    QList<PendingLine> pendingLines_;

    DisplayMode displayMode_ = DisplayMode::Hex;
    TimestampFormat timestampFormat_ = TimestampFormat::Absolute;
    bool paused_ = false;
    QString settingsGroup_;
    infra::config::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets