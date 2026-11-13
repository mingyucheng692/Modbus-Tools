// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QByteArray>
#include <QList>
#include <QColor>
#include <QString>
#include <QElapsedTimer>
#include <deque>
#include "LogListModel.h"
#include "modbus/telemetry/TrafficMetrics.h"

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

using TrafficStats = ::modbus::telemetry::TrafficStats;

struct PendingLine {
    using Kind = LogEntry::Kind;
    Kind kind = Kind::Info;
    QByteArray payload;
    QString message;
    qint64 elapsedMs = 0;
    QString wallTimeString;
    QString clientLabel;
};

class ByteMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ByteMonitorWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ByteMonitorWidget() noexcept override;

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
    void clientTrafficReceived(bool isTx, QByteArray data, int clientId);
    void infoMessageReceived(QString message);
    void errorMessageReceived(QString message);
    void warnMessageReceived(QString message);

private slots:
    void onTxData(QByteArray data);
    void onRxData(QByteArray data);
    void onClientTrafficData(bool isTx, QByteArray data, int clientId);
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
    void enqueueTraffic(bool isTx, QByteArray data, const QString& clientLabel);
    void enqueueStatusMessage(PendingLine::Kind kind, QString message);
    void enqueuePendingLine(PendingLine&& line);
    QString currentWallTime() const;
    LogEntry buildLogEntry(const PendingLine& line);
    void reformatLogEntry(LogEntry& entry) const;
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
    std::deque<PendingLine> pendingLines_;
    int droppedPendingCount_ = 0;

    DisplayMode displayMode_ = DisplayMode::Hex;
    TimestampFormat timestampFormat_ = TimestampFormat::Absolute;
    bool paused_ = false;
    QString settingsGroup_;
    infra::config::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets
